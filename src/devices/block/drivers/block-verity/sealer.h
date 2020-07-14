// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_DEVICES_BLOCK_DRIVERS_BLOCK_VERITY_SEALER_H_
#define SRC_DEVICES_BLOCK_DRIVERS_BLOCK_VERITY_SEALER_H_

#include <zircon/types.h>

#include <memory>
#include <vector>

#include <digest/digest.h>

#include "constants.h"
#include "device-info.h"
#include "hash-block-accumulator.h"

namespace block_verity {

typedef void (*sealer_callback)(void* ctx, zx_status_t status, const uint8_t* buf, size_t len);

class Sealer {
 public:
  Sealer(DeviceInfo info);
  ~Sealer();

  // Disallow copy & assign.  Allow move.
  Sealer(const Sealer&) = delete;
  Sealer& operator=(const Sealer&) = delete;

  zx_status_t StartSealing(void* cookie, sealer_callback callback);

  // Based on current state: either take an action (request an I/O) or advance
  // the state machine.
  void ScheduleNextWorkUnit();

  // Request the next data block(s) from disk so we can hash them.
  void RequestNextRead();
  void OnReadCompleted(zx_status_t status, block_op_t* block);
  static void ReadCompletedCallback(void* cookie, zx_status_t status, block_op_t* block);

  // Check if any integrity accumulators are full.  If so, write them out and
  // prepare new empty ones.
  void WriteIntegrityIfReady();
  void OnIntegrityWriteCompleted(zx_status_t status, block_op_t* block);
  static void IntegrityWriteCompletedCallback(void* cookie, zx_status_t status, block_op_t* block);

  void WriteSuperblock();
  void OnSuperblockWriteCompleted(zx_status_t status, block_op_t* block);
  static void SuperblockWriteCompletedCallback(void* cookie, zx_status_t status, block_op_t* block);

  void Flush();
  void OnFlushCompleted(zx_status_t status, block_op_t* block);
  static void FlushCompletedCallback(void* cookie, zx_status_t status, block_op_t* block);

  // Mark the computation as failed and trigger the sealer's callback.
  void Fail(zx_status_t error);

 private:
  enum State {
    // Initial state
    Initial,

    // Still reading through data blocks, writing integrity blocks as they
    // complete
    ReadLoop,

    // Done reading through data blocks; padding out hash blocks with zeroes
    PadHashBlocks,

    // Writing out the superblock
    CommitSuperblock,

    // Requesting flush of all writes
    FinalFlush,

    // Finished.
    Done,

    // If any block operation fails along the way, mark the whole thing as a
    // failure.
    Failed,
  };

  // Drive geometry/block client handle.
  const DeviceInfo info_;

  // The current state of the sealing computation.
  State state_;

  // The index into the integrity section of the first integrity block that we
  // have *not* written out yet.
  uint64_t integrity_block_index_;

  // The first block in the data section that we have *not* requested a block read for yet.
  uint64_t data_block_index_;

  // The number of outstanding block requests.  We can only safely terminate
  // once these are all settled.
  // For this first pass implementation, we never have more than 1 request
  // outstanding, so this should always be either 0 or 1.
  uint64_t outstanding_block_requests_;

  // A single block op request buffer, allocated to be the size of the parent
  // block op size request.
  std::unique_ptr<uint8_t[]> block_op_buf_;

  // A vmo used in block device operations
  zx::vmo block_op_vmo_;

  // The start address where that vmo is mapped
  uint8_t* vmo_base_;

  // Accumulate hashes into blocks.  One for the current block-in-progress at
  // each tier of the hash tree.
  std::vector<HashBlockAccumulator> hash_block_accumulators_;

  // Hash of the root block of the merkle tree.
  uint8_t root_hash_[32];

  // Holds the callback function and context pointer across async boundaries.
  // Saved when StartSealing is called and called exactly once.
  sealer_callback callback_;
  void* cookie_;
};

}  // namespace block_verity

#endif  // SRC_DEVICES_BLOCK_DRIVERS_BLOCK_VERITY_SEALER_H_
