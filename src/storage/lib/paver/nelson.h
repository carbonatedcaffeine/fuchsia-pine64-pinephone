// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef SRC_STORAGE_LIB_PAVER_NELSON_H_
#define SRC_STORAGE_LIB_PAVER_NELSON_H_

#include "src/storage/lib/paver/abr-client.h"
#include "src/storage/lib/paver/device-partitioner.h"
#include "src/storage/lib/paver/gpt.h"
#include "src/storage/lib/paver/partition-client.h"

namespace paver {

class NelsonPartitioner : public DevicePartitioner {
 public:
  static zx::status<std::unique_ptr<DevicePartitioner>> Initialize(
      fbl::unique_fd devfs_root, const zx::channel& svc_root, const fbl::unique_fd& block_device);

  bool IsFvmWithinFtl() const override { return false; }

  bool SupportsPartition(const PartitionSpec& spec) const override;

  zx::status<std::unique_ptr<PartitionClient>> AddPartition(
      const PartitionSpec& spec) const override;

  zx::status<std::unique_ptr<PartitionClient>> FindPartition(
      const PartitionSpec& spec) const override;

  zx::status<> FinalizePartition(const PartitionSpec& spec) const override { return zx::ok(); }

  zx::status<> WipeFvm() const override;

  zx::status<> InitPartitionTables() const override;

  zx::status<> WipePartitionTables() const override;

  zx::status<> ValidatePayload(const PartitionSpec& spec,
                               fbl::Span<const uint8_t> data) const override;

  zx::status<> Flush() const override { return zx::ok(); }

 private:
  explicit NelsonPartitioner(std::unique_ptr<GptDevicePartitioner> gpt) : gpt_(std::move(gpt)) {}

  zx::status<std::unique_ptr<PartitionClient>> GetBL2PartitionClient() const;

  std::unique_ptr<GptDevicePartitioner> gpt_;
};

class NelsonPartitionerFactory : public DevicePartitionerFactory {
 public:
  zx::status<std::unique_ptr<DevicePartitioner>> New(fbl::unique_fd devfs_root,
                                                     const zx::channel& svc_root, Arch arch,
                                                     std::shared_ptr<Context> context,
                                                     const fbl::unique_fd& block_device) final;
};

class NelsonAbrClientFactory : public abr::ClientFactory {
 public:
  zx::status<std::unique_ptr<abr::Client>> New(fbl::unique_fd devfs_root,
                                               const zx::channel& svc_root,
                                               std::shared_ptr<paver::Context> context) final;
};

}  // namespace paver

#endif  // SRC_STORAGE_LIB_PAVER_NELSON_H_
