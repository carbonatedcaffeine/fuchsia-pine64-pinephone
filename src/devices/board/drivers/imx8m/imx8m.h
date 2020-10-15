// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_DEVICES_BOARD_DRIVERS_IMX8M_IMX8M_H_
#define SRC_DEVICES_BOARD_DRIVERS_IMX8M_IMX8M_H_

#include <ddk/device.h>
#include <ddktl/device.h>
#include <ddktl/protocol/gpioimpl.h>
#include <ddktl/protocol/iommu.h>
#include <ddktl/protocol/platform/bus.h>

namespace imx8m {

class Imx8m : public ddk::Device<Imx8m> {
 public:
  Imx8m(zx_device_t* parent, pbus_protocol_t* pbus, iommu_protocol_t* iommu)
      : ddk::Device<Imx8m>(parent), pbus_(pbus), iommu_(iommu) {}

  static zx_status_t Create(void* ctx, zx_device_t* parent);

  void DdkRelease() { delete this; }

 private:
  ddk::PBusProtocolClient pbus_;
  ddk::IommuProtocolClient iommu_;
  ddk::GpioImplProtocolClient gpio_impl_;
};

}  // namespace imx8m

#endif  // SRC_DEVICES_BOARD_DRIVERS_IMX8M_IMX8M_H_
