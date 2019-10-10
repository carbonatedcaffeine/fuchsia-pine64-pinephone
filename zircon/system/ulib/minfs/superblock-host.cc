// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <minfs/superblock.h>

#include <lib/cksum.h>
#include <stdlib.h>
#include <string.h>

#include <utility>

#include <bitmap/raw-bitmap.h>

namespace minfs {

SuperblockManager::SuperblockManager(const Superblock* info) {
  memcpy(&info_blk_[0], info, sizeof(Superblock));
}

SuperblockManager::~SuperblockManager() = default;

// Static.
zx_status_t SuperblockManager::Create(const Superblock* info, uint32_t max_blocks,
                                      IntegrityCheck checks,
                                      fbl::unique_ptr<SuperblockManager>* out) {
  zx_status_t status = ZX_OK;
  if (checks == IntegrityCheck::kAll) {
    status = CheckSuperblock(info, max_blocks);
    if (status != ZX_OK) {
      FS_TRACE_ERROR("SuperblockManager::Create failed to check info: %d\n", status);
      return status;
    }
  }

  auto sb = fbl::unique_ptr<SuperblockManager>(new SuperblockManager(info));
  *out = std::move(sb);
  return ZX_OK;
}

void SuperblockManager::Write(PendingWork* transaction, UpdateBackupSuperblock write_backup) {
  UpdateChecksum(MutableInfo());
  auto data = &info_blk_[0];

  storage::Operation op = {
      .type = storage::OperationType::kWrite,
      .vmo_offset = 0,
      .dev_offset = kSuperblockStart,
      .length = 1,
  };
  transaction->EnqueueMetadata(data, std::move(op));

  if (write_backup == UpdateBackupSuperblock::kUpdate) {
    blk_t superblock_dev_offset = kNonFvmSuperblockBackup;

    if (MutableInfo()->flags & kMinfsFlagFVM) {
      superblock_dev_offset = kFvmSuperblockBackup;
    }

    storage::Operation op = {
        .type = storage::OperationType::kWrite,
        .vmo_offset = 0,
        .dev_offset = superblock_dev_offset,
        .length = 1,
    };
    transaction->EnqueueMetadata(data, std::move(op));
  }
}

}  // namespace minfs
