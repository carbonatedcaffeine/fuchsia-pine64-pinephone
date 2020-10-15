// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "imx8m.h"

#include <ddk/debug.h>

#include "src/devices/board/drivers/imx8m/imx8m-bind.h"

namespace imx8m {

zx_status_t Imx8m::Create(void* ctx, zx_device_t* parent) {
  zxlogf(INFO, "Create!");
  return ZX_ERR_NOT_SUPPORTED;
}

static zx_driver_ops_t imx8m_driver_ops = []() {
  zx_driver_ops_t ops = {};
  ops.version = DRIVER_OPS_VERSION;
  ops.bind = Imx8m::Create;
  return ops;
}();

}  // namespace imx8m

ZIRCON_DRIVER(imx8m, imx8m::imx8m_driver_ops, "zircon", "0.1")
