// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sealer.h"

#include <endian.h>
#include <lib/zx/vmar.h>
#include <lib/zx/vmo.h>
#include <zircon/assert.h>
#include <zircon/status.h>

#include <ddk/debug.h>
#include <fbl/auto_call.h>

#include "hash-block-accumulator.h"

namespace block_verity {

// Size of the VMO buffer to allocate.  Chosen so we can read a whole integrity
// block worth of data blocks at a time.  (In practice this is 512KiB.)
constexpr size_t kVmoSize = kBlockSize * (kBlockSize / kHashOutputSize);

Sealer::Sealer(DeviceInfo info)
    : info_(std::move(info)),
      state_(Initial),
      integrity_block_index_(0),
      data_block_index_(0),
      outstanding_block_requests_(0) {
  block_op_buf_ = std::make_unique<uint8_t[]>(info_.upstream_op_size);
  for (uint32_t i = 0; i < info_.block_allocation.integrity_shape.tree_depth; i++) {
    hash_block_accumulators_.emplace_back(HashBlockAccumulator());
  }
}

Sealer::~Sealer() {
  // We should not be destructed until all our outstanding block requests have
  // completed, lest they trigger callbacks that refer to the destructed object.
  ZX_ASSERT(outstanding_block_requests_ == 0);

  // Unmap the vmo frmo the vmar.
  if (vmo_base_ == nullptr) {
    return;
  }
  uintptr_t address = reinterpret_cast<uintptr_t>(vmo_base_);
  vmo_base_ = nullptr;
  zx_status_t rc = zx::vmar::root_self()->unmap(address, kVmoSize);
  if (rc != ZX_OK) {
    zxlogf(WARNING, "failed to unmap %lu bytes at %lu: %s", kVmoSize, address,
           zx_status_get_string(rc));
  }
}

zx_status_t Sealer::StartSealing(void* cookie, sealer_callback callback) {
  if (state_ != Initial) {
    return ZX_ERR_BAD_STATE;
  }

  // Allocate and map a VMO for block operations
  zx_status_t rc;
  if ((rc = zx::vmo::create(kVmoSize, 0, &block_op_vmo_)) != ZX_OK) {
    zxlogf(ERROR, "zx::vmo::create failed: %s", zx_status_get_string(rc));
    return rc;
  }
  auto cleanup = fbl::MakeAutoCall([this]() { block_op_vmo_.reset(); });
  constexpr uint32_t flags = ZX_VM_PERM_READ | ZX_VM_PERM_WRITE;
  uintptr_t address;
  if ((rc = zx::vmar::root_self()->map(0, block_op_vmo_, 0, kVmoSize, flags, &address)) != ZX_OK) {
    zxlogf(ERROR, "zx::vmar::map failed: %s", zx_status_get_string(rc));
    return rc;
  }
  vmo_base_ = reinterpret_cast<uint8_t*>(address);
  cleanup.cancel();

  // Save the callback & userdata.
  cookie_ = cookie;
  callback_ = callback;

  // The overall algorithm here is:
  // * while data_blocks is not at the end of the data segment:
  //   * READ the next data block into memory
  //   * hash the contents of that block
  //   * feed that hash result into the 0-level integrity block accumulator
  //   * while any block accumulator has a full block (from lowest tier to highest):
  //     * if block is complete, WRITE out the block
  //     * then hash the block and feed it into the next integrity block accumulator
  //     * then reset this level's block accumulator
  // * then pad out the remaining blocks and WRITE them all out
  // * then take the hash of the root block and put it in the superblock and
  //   WRITE the superblock out
  // * then FLUSH everything
  // * then hash the superblock itself and mark sealing as complete

  // But to start: all we need to do is set state to ReadLoop, and request the
  // first read.  Every continuation will either schedule the next additional I/O,
  // or call ScheduleNextWorkUnit() (which is the main state-machine-advancing
  // loop).
  state_ = ReadLoop;
  RequestNextRead();
  return ZX_OK;
}

void Sealer::ScheduleNextWorkUnit() {
  switch (state_) {
    case Initial:
      zxlogf(ERROR, "ScheduleNextWorkUnit called while state was Initial\n");
      return;
    case ReadLoop:
      // See if we have read everything.  If not, dispatch a read.
      if (data_block_index_ < info_.block_allocation.data_block_count) {
        RequestNextRead();
        return;
      } else {
        // Otherwise, update state, then fall through to PadHashBlocks.
        state_ = PadHashBlocks;
      }
    case PadHashBlocks: {
      // For each hash tier that is not already empty (since we eagerly flush
      // full blocks), pad it with zeroes until it is full, and flush it to disk.
      for (size_t tier = 0; tier < hash_block_accumulators_.size(); tier++) {
        auto& hba = hash_block_accumulators_[tier];
        if (!hba.IsEmpty()) {
          hba.PadBlockWithZeroesToFill();
          WriteIntegrityIfReady();
          return;
        }
      }
      // If all hash tiers have been fully written out, proceed to writing out
      // the superblock.
      state_ = CommitSuperblock;
    }
    case CommitSuperblock:
      WriteSuperblock();
      return;
    case FinalFlush: {
      Flush();
      return;
    }
    case Done:
      zxlogf(ERROR, "ScheduleNextWorkUnit called while state was Done\n");
      return;
    case Failed:
      zxlogf(ERROR, "ScheduleNextWorkUnit called while state was Failed\n");
      return;
  }
}

void Sealer::RequestNextRead() {
  // TODO(perf optimization): read up to as many blocks as will fill an
  // integrity block at a time.  It's a convenient batch size.
  uint64_t mapped_data_block = info_.data_start_offset + data_block_index_;

  // For now, we'll just read one block, though we could move to larger batches
  // ~trivially with a larger block_op_vmo_ buffer.
  block_op_t* block_op = reinterpret_cast<block_op_t*>(block_op_buf_.get());
  block_op->rw.command = BLOCK_OP_READ;
  block_op->rw.length = 1;
  block_op->rw.offset_dev = mapped_data_block;
  data_block_index_++;
  block_op->rw.offset_vmo = 0;
  block_op->rw.vmo = block_op_vmo_.get();

  // Send read request.
  outstanding_block_requests_ += 1;
  ZX_ASSERT(outstanding_block_requests_ == 1);
  info_.block_protocol.Queue(block_op, ReadCompletedCallback, this);
}

void Sealer::ReadCompletedCallback(void* cookie, zx_status_t status, block_op_t* block) {
  // Static trampoline to OnReadCompleted.
  Sealer* device = static_cast<Sealer*>(cookie);
  device->OnReadCompleted(status, block);
}

void Sealer::OnReadCompleted(zx_status_t status, block_op_t* block) {
  // Decrement outstanding requests.
  outstanding_block_requests_ -= 1;
  ZX_ASSERT(outstanding_block_requests_ == 0);

  // Check for failures.
  if (status != ZX_OK) {
    zxlogf(ERROR, "Failed to read %d blocks starting at offset %lu: %s", block->rw.length,
           block->rw.offset_dev, zx_status_get_string(status));
    Fail(status);
    return;
  }

  // Hash the contents of the block we just read.
  // TODO: read batching
  digest::Digest hasher;
  hasher.Hash(reinterpret_cast<void*>(vmo_base_), kBlockSize);

  // Feed that hash result into the 0-level integrity block accumulator.
  hash_block_accumulators_[0].Feed(hasher.get(), hasher.len());

  // then check if we need to flush any out to disk
  WriteIntegrityIfReady();
}

void Sealer::Fail(zx_status_t error) {
  state_ = Failed;
  zxlogf(ERROR, "Seal entered failed state\n");
  // Notify computation completion (failed)
  if (callback_) {
    sealer_callback callback = callback_;
    callback_ = nullptr;
    void* cookie = cookie_;
    cookie_ = nullptr;
    callback(cookie, error, nullptr, 0);
  }
  return;
}

void Sealer::WriteIntegrityIfReady() {
  // for each block accumulator:
  //   if full:
  //     if not write_requested:
  //       mark write requested
  //       send write request
  //       return
  //     else:
  //       if (not root hash block):
  //         feed hash output up a level
  //       else:
  //         save root hash for superblock
  //       reset this tier's hash block accumulator
  // if done, schedule next work unit

  for (size_t tier = 0; tier < hash_block_accumulators_.size(); tier++) {
    HashBlockAccumulator& hba = hash_block_accumulators_[tier];
    if (hba.IsFull()) {
      if (!hba.HasWriteRequested()) {
        uint64_t mapped_integrity_block = info_.integrity_start_offset + integrity_block_index_;
        hba.MarkWriteRequested();

        // Copy integrity block contents into VMO for write
        memcpy(vmo_base_, hba.BlockData(), kBlockSize);

        // prepare write block op
        block_op_t* block_op = reinterpret_cast<block_op_t*>(block_op_buf_.get());
        block_op->rw.command = BLOCK_OP_WRITE;
        block_op->rw.length = 1;
        block_op->rw.offset_dev = mapped_integrity_block;
        integrity_block_index_++;
        block_op->rw.offset_vmo = 0;
        block_op->rw.vmo = block_op_vmo_.get();

        // send write request
        outstanding_block_requests_ += 1;
        ZX_ASSERT(outstanding_block_requests_ == 1);
        info_.block_protocol.Queue(block_op, IntegrityWriteCompletedCallback, this);
        return;
      } else {
        // We previously marked this write as requested and have now completed it.
        // We should now hash this block and feed it into the next hash block
        // accumulator up.  That block might now be full, so we continue the
        // top-level for loop.

        digest::Digest hasher;
        const uint8_t* block_hash = hasher.Hash(hba.BlockData(), kBlockSize);

        if (tier + 1 < hash_block_accumulators_.size()) {
          // Some tier other than the last.  Feed integrity block into parent.
          HashBlockAccumulator& next_tier_hba = hash_block_accumulators_[tier + 1];
          next_tier_hba.Feed(block_hash, hasher.len());
        } else {
          // This is the final tier.  Save the root hash so we can put it in the
          // superblock.
          memcpy(root_hash_, block_hash, hasher.len());
        }

        hba.Reset();
      }
    }
  }

  // If we made it here, we've finished flushing all hash blocks that we've fed
  // in enough input to complete.
  ScheduleNextWorkUnit();
}

void Sealer::OnIntegrityWriteCompleted(zx_status_t status, block_op_t* block) {
  // Decrement outstanding requests.
  outstanding_block_requests_ -= 1;
  ZX_ASSERT(outstanding_block_requests_ == 0);

  // Check for failures.
  if (status != ZX_OK) {
    zxlogf(ERROR, "Failed to write %d blocks starting at offset %lu: %s", block->rw.length,
           block->rw.offset_dev, zx_status_get_string(status));
    Fail(status);
    return;
  }

  // Continue updating integrity blocks until flushed.
  WriteIntegrityIfReady();
}

void Sealer::IntegrityWriteCompletedCallback(void* cookie, zx_status_t status, block_op_t* block) {
  // Static trampoline to OnIntegrityWriteCompleted.
  Sealer* device = static_cast<Sealer*>(cookie);
  device->OnIntegrityWriteCompleted(status, block);
}

void Sealer::WriteSuperblock() {
  // Construct a valid superblock in the first block of block_op_vmo_.
  //
  // A v1 superblock looks like:
  //
  // 16 bytes magic
  // 8 bytes block count (little-endian)
  // 4 bytes block size (little-endian)
  // 4 bytes hash function tag (little-endian)
  // 32 bytes integrity root hash
  // 4032 zero bytes padding the rest of the block
  uint8_t* ptr = vmo_base_;

  // Set backdrop of zeroes.
  memset(vmo_base_, 0, kBlockSize);

  // magic (16 bytes)
  memcpy(ptr, kBlockVerityMagic, sizeof(kBlockVerityMagic));
  ptr += sizeof(kBlockVerityMagic);

  // block count
  uint64_t block_count = htole64(info_.block_count);
  memcpy(ptr, &block_count, sizeof(block_count));
  ptr += sizeof(block_count);

  // block size in bytes
  uint32_t block_size = htole32(info_.block_size);
  memcpy(ptr, &block_size, sizeof(block_size));
  ptr += sizeof(block_size);

  // hash function tag
  memcpy(ptr, kSHA256HashTag, sizeof(kSHA256HashTag));
  ptr += sizeof(kSHA256HashTag);

  // hash of integrity root
  memcpy(ptr, root_hash_, sizeof(root_hash_));
  ptr += sizeof(root_hash_);

  // prepare write block op
  block_op_t* block_op = reinterpret_cast<block_op_t*>(block_op_buf_.get());
  block_op->rw.command = BLOCK_OP_WRITE;
  block_op->rw.length = 1;
  block_op->rw.offset_dev = 0;  // Superblock is block 0
  block_op->rw.offset_vmo = 0;
  block_op->rw.vmo = block_op_vmo_.get();

  // send write request
  outstanding_block_requests_ += 1;
  ZX_ASSERT(outstanding_block_requests_ == 1);
  info_.block_protocol.Queue(block_op, SuperblockWriteCompletedCallback, this);
  return;
}

void Sealer::OnSuperblockWriteCompleted(zx_status_t status, block_op_t* block) {
  // Decrement outstanding requests.
  outstanding_block_requests_ -= 1;
  ZX_ASSERT(outstanding_block_requests_ == 0);

  // Check for failures.
  if (status != ZX_OK) {
    zxlogf(ERROR, "Failed to write %d blocks starting at offset %lu: %s", block->rw.length,
           block->rw.offset_dev, zx_status_get_string(status));
    Fail(status);
    return;
  }

  state_ = FinalFlush;
  ScheduleNextWorkUnit();
}

void Sealer::SuperblockWriteCompletedCallback(void* cookie, zx_status_t status, block_op_t* block) {
  // Static trampoline to OnSuperblockWriteCompleted.
  Sealer* device = static_cast<Sealer*>(cookie);
  device->OnSuperblockWriteCompleted(status, block);
}

void Sealer::Flush() {
  // prepare flush block op
  block_op_t* block_op = reinterpret_cast<block_op_t*>(block_op_buf_.get());
  block_op->command = BLOCK_OP_FLUSH;

  // send write request
  outstanding_block_requests_ += 1;
  ZX_ASSERT(outstanding_block_requests_ == 1);
  info_.block_protocol.Queue(block_op, FlushCompletedCallback, this);
  return;
}

void Sealer::OnFlushCompleted(zx_status_t status, block_op_t* block) {
  // Decrement outstanding requests.
  outstanding_block_requests_ -= 1;
  ZX_ASSERT(outstanding_block_requests_ == 0);

  // Check for failures.
  if (status != ZX_OK) {
    zxlogf(ERROR, "Failed to flush: %s", zx_status_get_string(status));
    Fail(status);
    return;
  }

  state_ = Done;

  // Hash the superblock that was the last thing we wrote to vmo_base_.
  digest::Digest hasher;
  const uint8_t* hashed = hasher.Hash(reinterpret_cast<void*>(vmo_base_), kBlockSize);

  ZX_ASSERT(callback_);
  sealer_callback callback = callback_;
  callback_ = nullptr;
  void* cookie = cookie_;
  cookie_ = nullptr;
  // Calling the callback must be the very last thing we do.  We expect
  // `this` to be deallocated in the course of the callback here.
  callback(cookie, ZX_OK, hashed, hasher.len());
}

void Sealer::FlushCompletedCallback(void* cookie, zx_status_t status, block_op_t* block) {
  // Static trampoline to OnSuperblockWriteCompleted.
  Sealer* device = static_cast<Sealer*>(cookie);
  device->OnFlushCompleted(status, block);
}

}  // namespace block_verity
