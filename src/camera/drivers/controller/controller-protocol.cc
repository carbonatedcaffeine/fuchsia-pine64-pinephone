// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "controller-protocol.h"

#include <src/lib/fxl/logging.h>

#include "fuchsia/camera2/cpp/fidl.h"

namespace camera {

ControllerImpl::ControllerImpl(fidl::InterfaceRequest<fuchsia::camera2::hal::Controller> control,
                               async_dispatcher_t* dispatcher, ddk::IspProtocolClient& isp,
                               fit::closure on_connection_closed)
    : dispatcher_(dispatcher), binding_(this), isp_(isp) {
  binding_.set_error_handler([occ = std::move(on_connection_closed)](zx_status_t status) {
    FXL_PLOG(ERROR, status) << "Client disconnected";
    occ();
  });
  binding_.Bind(std::move(control), dispatcher);
}

zx_status_t ControllerImpl::GetInternalConfiguration(uint32_t config_index,
                                                     InternalConfigInfo** internal_config) {
  if (config_index >= internal_configs_.configs_info.size() || internal_config == nullptr) {
    return ZX_ERR_INVALID_ARGS;
  }

  *internal_config = &internal_configs_.configs_info[config_index];
  return ZX_OK;
}

void ControllerImpl::GetConfigs(GetConfigsCallback callback) {
  PopulateConfigurations();
  callback(fidl::Clone(configs_), ZX_OK);
}

void ControllerImpl::CreateStream(uint32_t config_index, uint32_t stream_index,
                                  uint32_t image_format_index,
                                  fuchsia::sysmem::BufferCollectionInfo_2 buffer_collection,
                                  fidl::InterfaceRequest<fuchsia::camera2::Stream> stream) {
  if (stream_) {
    FXL_PLOG(ERROR, ZX_ERR_ALREADY_BOUND) << "Stream already bound";
    stream.Close(ZX_ERR_ALREADY_BOUND);
    return;
  }

  if (config_index >= configs_.size()) {
    FXL_LOG(ERROR) << "Invalid config index " << config_index;
    stream.Close(ZX_ERR_INVALID_ARGS);
    return;
  }
  const auto& config = configs_[config_index];

  if (stream_index >= config.stream_configs.size()) {
    FXL_LOG(ERROR) << "Invalid stream index " << stream_index;
    stream.Close(ZX_ERR_INVALID_ARGS);
    return;
  }
  const auto& stream_config = config.stream_configs[stream_index];

  if (image_format_index >= stream_config.image_formats.size()) {
    FXL_LOG(ERROR) << "Invalid image format index " << image_format_index;
    stream.Close(ZX_ERR_INVALID_ARGS);
    return;
  }
  const auto& image_format = stream_config.image_formats[image_format_index];

  if (buffer_collection.buffer_count == 0) {
    FXL_LOG(ERROR) << "Invalid buffer count " << buffer_collection.buffer_count;
    stream.Close(ZX_ERR_INVALID_ARGS);
    return;
  }

  auto stream_impl = std::make_unique<camera::StreamImpl>(dispatcher_);
  if (!stream_impl) {
    FXL_LOG(ERROR) << "Failed to create StreamImpl";
    stream.Close(ZX_ERR_INTERNAL);
    return;
  }

  fuchsia_sysmem_BufferCollectionInfo buffers{};
  buffers.buffer_count = buffer_collection.buffer_count;
  buffers.format.image.width = image_format.coded_width;
  buffers.format.image.height = image_format.coded_height;
  buffers.format.image.layers = image_format.layers;
  buffers.format.image.pixel_format =
      *reinterpret_cast<const fuchsia_sysmem_PixelFormat*>(&image_format.pixel_format);
  buffers.format.image.color_space =
      *reinterpret_cast<const fuchsia_sysmem_ColorSpace*>(&image_format.color_space);
  buffers.format.image.planes[0].bytes_per_row = image_format.bytes_per_row;
  for (uint32_t i = 0; i < buffer_collection.buffer_count; ++i) {
    buffers.vmos[i] = buffer_collection.buffers[i].vmo.release();
  }
  buffers.vmo_size = buffer_collection.settings.buffer_settings.size_bytes;

  // TODO: negotiate stream type
  zx_status_t status = isp_.CreateOutputStream(
      &buffers, reinterpret_cast<const frame_rate_t*>(&stream_config.frame_rate),
      STREAM_TYPE_FULL_RESOLUTION, stream_impl->Callbacks(), stream_impl->Protocol());
  if (status != ZX_OK) {
    FXL_PLOG(ERROR, status) << "Failed to create output stream on ISP";
    stream.Close(ZX_ERR_INTERNAL);
    return;
  }

  // The ISP does not actually take ownership of the buffers upon creating the stream (they are
  // duplicated internally), so they must be manually released here.
  status = zx_handle_close_many(buffers.vmos, buffers.buffer_count);
  if (status != ZX_OK) {
    FXL_PLOG(ERROR, status) << "Failed to close buffer handles";
    stream.Close(ZX_ERR_INTERNAL);
    return;
  }

  status = stream_impl->Attach(stream.TakeChannel(), []() {
    FXL_LOG(INFO) << "Stream client disconnected";
    // TODO(fxb/37296): allow shutting down the stream
    // stream_ = nullptr;
  });
  if (status != ZX_OK) {
    FXL_PLOG(ERROR, status) << "Failed to bind output stream";
    return;
  }

  stream_ = std::move(stream_impl);
}

void ControllerImpl::EnableStreaming() {}

void ControllerImpl::DisableStreaming() {}

void ControllerImpl::GetDeviceInfo(GetDeviceInfoCallback callback) {
  fuchsia::camera2::DeviceInfo camera_device_info;
  camera_device_info.set_vendor_name(kCameraVendorName);
  camera_device_info.set_product_name(kCameraProductName);
  camera_device_info.set_type(fuchsia::camera2::DeviceType::BUILTIN);
  callback(std::move(camera_device_info));
}

}  // namespace camera
