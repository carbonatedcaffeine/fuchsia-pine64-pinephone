// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_DEVICES_BLOCK_DRIVERS_BLOCK_VERITY_HASH_BLOCK_ACCUMULATOR_H_
#define SRC_DEVICES_BLOCK_DRIVERS_BLOCK_VERITY_HASH_BLOCK_ACCUMULATOR_H_

#include "constants.h"
#include "device-info.h"

namespace block_verity {

// Future work could genericize this over block size and hash algorithm,
// but for now it's expedient to assume 4k and SHA256.
class HashBlockAccumulator {
 public:
  HashBlockAccumulator();
  ~HashBlockAccumulator() = default;

  void Reset();
  bool IsEmpty() const;
  bool IsFull() const;
  void Feed(const uint8_t* buf, size_t count);
  void PadBlockWithZeroesToFill();
  const uint8_t* BlockData() const;
  bool HasWriteRequested() const;
  void MarkWriteRequested();

 private:
  uint8_t block[kBlockSize];
  size_t block_bytes_filled;
  bool write_requested_;
};

}  // namespace block_verity

#endif  // SRC_DEVICES_BLOCK_DRIVERS_BLOCK_VERITY_HASH_BLOCK_ACCUMULATOR_H_
