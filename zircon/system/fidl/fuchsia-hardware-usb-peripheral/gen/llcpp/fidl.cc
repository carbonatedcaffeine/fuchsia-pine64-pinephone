// WARNING: This file is machine generated by fidlgen.

#include <fuchsia/hardware/usb/peripheral/llcpp/fidl.h>
#include <memory>

namespace llcpp {

namespace fuchsia {
namespace hardware {
namespace usb {
namespace peripheral {

namespace {

[[maybe_unused]]
constexpr uint64_t kEvents_FunctionRegistered_Ordinal = 0x48ca785200000000lu;
[[maybe_unused]]
constexpr uint64_t kEvents_FunctionRegistered_GenOrdinal = 0x191278425c4a96e8lu;
[[maybe_unused]]
constexpr uint64_t kEvents_FunctionsCleared_Ordinal = 0x24319fec00000000lu;
[[maybe_unused]]
constexpr uint64_t kEvents_FunctionsCleared_GenOrdinal = 0x6feab079055dacf1lu;

}  // namespace
template <>
Events::ResultOf::FunctionRegistered_Impl<Events::FunctionRegisteredResponse>::FunctionRegistered_Impl(zx::unowned_channel _client_end) {
  constexpr uint32_t _kWriteAllocSize = ::fidl::internal::ClampedMessageSize<FunctionRegisteredRequest, ::fidl::MessageDirection::kSending>();
  ::fidl::internal::AlignedBuffer<_kWriteAllocSize> _write_bytes_inlined;
  auto& _write_bytes_array = _write_bytes_inlined;
  uint8_t* _write_bytes = _write_bytes_array.view().data();
  memset(_write_bytes, 0, FunctionRegisteredRequest::PrimarySize);
  ::fidl::BytePart _request_bytes(_write_bytes, _kWriteAllocSize, sizeof(FunctionRegisteredRequest));
  ::fidl::DecodedMessage<FunctionRegisteredRequest> _decoded_request(std::move(_request_bytes));
  Super::SetResult(
      Events::InPlace::FunctionRegistered(std::move(_client_end), Super::response_buffer()));
}

Events::ResultOf::FunctionRegistered Events::SyncClient::FunctionRegistered() {
  return ResultOf::FunctionRegistered(zx::unowned_channel(this->channel_));
}

Events::ResultOf::FunctionRegistered Events::Call::FunctionRegistered(zx::unowned_channel _client_end) {
  return ResultOf::FunctionRegistered(std::move(_client_end));
}

::fidl::DecodeResult<Events::FunctionRegisteredResponse> Events::InPlace::FunctionRegistered(zx::unowned_channel _client_end, ::fidl::BytePart response_buffer) {
  constexpr uint32_t _write_num_bytes = sizeof(FunctionRegisteredRequest);
  ::fidl::internal::AlignedBuffer<_write_num_bytes> _write_bytes;
  ::fidl::BytePart _request_buffer = _write_bytes.view();
  _request_buffer.set_actual(_write_num_bytes);
  ::fidl::DecodedMessage<FunctionRegisteredRequest> params(std::move(_request_buffer));
  params.message()->_hdr = {};
  params.message()->_hdr.ordinal = kEvents_FunctionRegistered_Ordinal;
  auto _encode_request_result = ::fidl::Encode(std::move(params));
  if (_encode_request_result.status != ZX_OK) {
    return ::fidl::DecodeResult<Events::FunctionRegisteredResponse>::FromFailure(
        std::move(_encode_request_result));
  }
  auto _call_result = ::fidl::Call<FunctionRegisteredRequest, FunctionRegisteredResponse>(
    std::move(_client_end), std::move(_encode_request_result.message), std::move(response_buffer));
  if (_call_result.status != ZX_OK) {
    return ::fidl::DecodeResult<Events::FunctionRegisteredResponse>::FromFailure(
        std::move(_call_result));
  }
  return ::fidl::Decode(std::move(_call_result.message));
}


Events::ResultOf::FunctionsCleared_Impl::FunctionsCleared_Impl(zx::unowned_channel _client_end) {
  constexpr uint32_t _kWriteAllocSize = ::fidl::internal::ClampedMessageSize<FunctionsClearedRequest, ::fidl::MessageDirection::kSending>();
  ::fidl::internal::AlignedBuffer<_kWriteAllocSize> _write_bytes_inlined;
  auto& _write_bytes_array = _write_bytes_inlined;
  uint8_t* _write_bytes = _write_bytes_array.view().data();
  memset(_write_bytes, 0, FunctionsClearedRequest::PrimarySize);
  ::fidl::BytePart _request_bytes(_write_bytes, _kWriteAllocSize, sizeof(FunctionsClearedRequest));
  ::fidl::DecodedMessage<FunctionsClearedRequest> _decoded_request(std::move(_request_bytes));
  Super::operator=(
      Events::InPlace::FunctionsCleared(std::move(_client_end)));
}

Events::ResultOf::FunctionsCleared Events::SyncClient::FunctionsCleared() {
  return ResultOf::FunctionsCleared(zx::unowned_channel(this->channel_));
}

Events::ResultOf::FunctionsCleared Events::Call::FunctionsCleared(zx::unowned_channel _client_end) {
  return ResultOf::FunctionsCleared(std::move(_client_end));
}

::fidl::internal::StatusAndError Events::InPlace::FunctionsCleared(zx::unowned_channel _client_end) {
  constexpr uint32_t _write_num_bytes = sizeof(FunctionsClearedRequest);
  ::fidl::internal::AlignedBuffer<_write_num_bytes> _write_bytes;
  ::fidl::BytePart _request_buffer = _write_bytes.view();
  _request_buffer.set_actual(_write_num_bytes);
  ::fidl::DecodedMessage<FunctionsClearedRequest> params(std::move(_request_buffer));
  params.message()->_hdr = {};
  params.message()->_hdr.ordinal = kEvents_FunctionsCleared_Ordinal;
  auto _encode_request_result = ::fidl::Encode(std::move(params));
  if (_encode_request_result.status != ZX_OK) {
    return ::fidl::internal::StatusAndError::FromFailure(
        std::move(_encode_request_result));
  }
  zx_status_t _write_status =
      ::fidl::Write(std::move(_client_end), std::move(_encode_request_result.message));
  if (_write_status != ZX_OK) {
    return ::fidl::internal::StatusAndError(_write_status, ::fidl::internal::kErrorWriteFailed);
  } else {
    return ::fidl::internal::StatusAndError(ZX_OK, nullptr);
  }
}


bool Events::TryDispatch(Interface* impl, fidl_msg_t* msg, ::fidl::Transaction* txn) {
  if (msg->num_bytes < sizeof(fidl_message_header_t)) {
    zx_handle_close_many(msg->handles, msg->num_handles);
    txn->Close(ZX_ERR_INVALID_ARGS);
    return true;
  }
  fidl_message_header_t* hdr = reinterpret_cast<fidl_message_header_t*>(msg->bytes);
  switch (hdr->ordinal) {
    case kEvents_FunctionRegistered_Ordinal:
    case kEvents_FunctionRegistered_GenOrdinal:
    {
      auto result = ::fidl::DecodeAs<FunctionRegisteredRequest>(msg);
      if (result.status != ZX_OK) {
        txn->Close(ZX_ERR_INVALID_ARGS);
        return true;
      }
      impl->FunctionRegistered(
        Interface::FunctionRegisteredCompleter::Sync(txn));
      return true;
    }
    case kEvents_FunctionsCleared_Ordinal:
    case kEvents_FunctionsCleared_GenOrdinal:
    {
      auto result = ::fidl::DecodeAs<FunctionsClearedRequest>(msg);
      if (result.status != ZX_OK) {
        txn->Close(ZX_ERR_INVALID_ARGS);
        return true;
      }
      impl->FunctionsCleared(
        Interface::FunctionsClearedCompleter::Sync(txn));
      return true;
    }
    default: {
      return false;
    }
  }
}

bool Events::Dispatch(Interface* impl, fidl_msg_t* msg, ::fidl::Transaction* txn) {
  bool found = TryDispatch(impl, msg, txn);
  if (!found) {
    zx_handle_close_many(msg->handles, msg->num_handles);
    txn->Close(ZX_ERR_NOT_SUPPORTED);
  }
  return found;
}


void Events::Interface::FunctionRegisteredCompleterBase::Reply() {
  constexpr uint32_t _kWriteAllocSize = ::fidl::internal::ClampedMessageSize<FunctionRegisteredResponse, ::fidl::MessageDirection::kSending>();
  FIDL_ALIGNDECL uint8_t _write_bytes[_kWriteAllocSize] = {};
  auto& _response = *reinterpret_cast<FunctionRegisteredResponse*>(_write_bytes);
  _response._hdr.ordinal = kEvents_FunctionRegistered_Ordinal;
  ::fidl::BytePart _response_bytes(_write_bytes, _kWriteAllocSize, sizeof(FunctionRegisteredResponse));
  CompleterBase::SendReply(::fidl::DecodedMessage<FunctionRegisteredResponse>(std::move(_response_bytes)));
}


::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Result::Device_SetConfiguration_Result() {
  tag_ = Tag::Invalid;
}

::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Result::~Device_SetConfiguration_Result() {
  Destroy();
}

void ::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Result::Destroy() {
  switch (which()) {
  case Tag::kResponse:
    response_.~Device_SetConfiguration_Response();
    break;
  default:
    break;
  }
  tag_ = Tag::Invalid;
}

void ::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Result::MoveImpl_(Device_SetConfiguration_Result&& other) {
  switch (other.which()) {
  case Tag::kResponse:
    mutable_response() = std::move(other.mutable_response());
    break;
  case Tag::kErr:
    mutable_err() = std::move(other.mutable_err());
    break;
  default:
    break;
  }
  other.Destroy();
}

void ::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Result::SizeAndOffsetAssertionHelper() {
  static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Result, response_) == 4);
  static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Result, err_) == 4);
  static_assert(sizeof(::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Result) == ::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Result::PrimarySize);
}


Device_SetConfiguration_Response& ::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Result::mutable_response() {
  if (which() != Tag::kResponse) {
    Destroy();
    new (&response_) Device_SetConfiguration_Response;
  }
  tag_ = Tag::kResponse;
  return response_;
}

int32_t& ::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Result::mutable_err() {
  if (which() != Tag::kErr) {
    Destroy();
    new (&err_) int32_t;
  }
  tag_ = Tag::kErr;
  return err_;
}


namespace {

[[maybe_unused]]
constexpr uint64_t kDevice_SetConfiguration_Ordinal = 0x7969547500000000lu;
[[maybe_unused]]
constexpr uint64_t kDevice_SetConfiguration_GenOrdinal = 0x464bafee91a3d6delu;
extern "C" const fidl_type_t fuchsia_hardware_usb_peripheral_DeviceSetConfigurationRequestTable;
extern "C" const fidl_type_t fuchsia_hardware_usb_peripheral_DeviceSetConfigurationResponseTable;
[[maybe_unused]]
constexpr uint64_t kDevice_ClearFunctions_Ordinal = 0x4e0ef30000000000lu;
[[maybe_unused]]
constexpr uint64_t kDevice_ClearFunctions_GenOrdinal = 0x67d9d8086dfab0cblu;
[[maybe_unused]]
constexpr uint64_t kDevice_SetStateChangeListener_Ordinal = 0x4409dd700000000lu;
[[maybe_unused]]
constexpr uint64_t kDevice_SetStateChangeListener_GenOrdinal = 0x5575723c4674d1d9lu;
extern "C" const fidl_type_t fuchsia_hardware_usb_peripheral_DeviceSetStateChangeListenerRequestTable;

}  // namespace
template <>
Device::ResultOf::SetConfiguration_Impl<Device::SetConfigurationResponse>::SetConfiguration_Impl(zx::unowned_channel _client_end, DeviceDescriptor device_desc, ::fidl::VectorView<FunctionDescriptor> function_descriptors) {
  constexpr uint32_t _kWriteAllocSize = ::fidl::internal::ClampedMessageSize<SetConfigurationRequest, ::fidl::MessageDirection::kSending>();
  std::unique_ptr _write_bytes_boxed = std::make_unique<::fidl::internal::AlignedBuffer<_kWriteAllocSize>>();
  auto& _write_bytes_array = *_write_bytes_boxed;
  SetConfigurationRequest _request = {};
  _request.device_desc = std::move(device_desc);
  _request.function_descriptors = std::move(function_descriptors);
  auto _linearize_result = ::fidl::Linearize(&_request, _write_bytes_array.view());
  if (_linearize_result.status != ZX_OK) {
    Super::SetFailure(std::move(_linearize_result));
    return;
  }
  ::fidl::DecodedMessage<SetConfigurationRequest> _decoded_request = std::move(_linearize_result.message);
  Super::SetResult(
      Device::InPlace::SetConfiguration(std::move(_client_end), std::move(_decoded_request), Super::response_buffer()));
}

Device::ResultOf::SetConfiguration Device::SyncClient::SetConfiguration(DeviceDescriptor device_desc, ::fidl::VectorView<FunctionDescriptor> function_descriptors) {
  return ResultOf::SetConfiguration(zx::unowned_channel(this->channel_), std::move(device_desc), std::move(function_descriptors));
}

Device::ResultOf::SetConfiguration Device::Call::SetConfiguration(zx::unowned_channel _client_end, DeviceDescriptor device_desc, ::fidl::VectorView<FunctionDescriptor> function_descriptors) {
  return ResultOf::SetConfiguration(std::move(_client_end), std::move(device_desc), std::move(function_descriptors));
}

template <>
Device::UnownedResultOf::SetConfiguration_Impl<Device::SetConfigurationResponse>::SetConfiguration_Impl(zx::unowned_channel _client_end, ::fidl::BytePart _request_buffer, DeviceDescriptor device_desc, ::fidl::VectorView<FunctionDescriptor> function_descriptors, ::fidl::BytePart _response_buffer) {
  if (_request_buffer.capacity() < SetConfigurationRequest::PrimarySize) {
    Super::SetFailure(::fidl::DecodeResult<SetConfigurationResponse>(ZX_ERR_BUFFER_TOO_SMALL, ::fidl::internal::kErrorRequestBufferTooSmall));
    return;
  }
  SetConfigurationRequest _request = {};
  _request.device_desc = std::move(device_desc);
  _request.function_descriptors = std::move(function_descriptors);
  auto _linearize_result = ::fidl::Linearize(&_request, std::move(_request_buffer));
  if (_linearize_result.status != ZX_OK) {
    Super::SetFailure(std::move(_linearize_result));
    return;
  }
  ::fidl::DecodedMessage<SetConfigurationRequest> _decoded_request = std::move(_linearize_result.message);
  Super::SetResult(
      Device::InPlace::SetConfiguration(std::move(_client_end), std::move(_decoded_request), std::move(_response_buffer)));
}

Device::UnownedResultOf::SetConfiguration Device::SyncClient::SetConfiguration(::fidl::BytePart _request_buffer, DeviceDescriptor device_desc, ::fidl::VectorView<FunctionDescriptor> function_descriptors, ::fidl::BytePart _response_buffer) {
  return UnownedResultOf::SetConfiguration(zx::unowned_channel(this->channel_), std::move(_request_buffer), std::move(device_desc), std::move(function_descriptors), std::move(_response_buffer));
}

Device::UnownedResultOf::SetConfiguration Device::Call::SetConfiguration(zx::unowned_channel _client_end, ::fidl::BytePart _request_buffer, DeviceDescriptor device_desc, ::fidl::VectorView<FunctionDescriptor> function_descriptors, ::fidl::BytePart _response_buffer) {
  return UnownedResultOf::SetConfiguration(std::move(_client_end), std::move(_request_buffer), std::move(device_desc), std::move(function_descriptors), std::move(_response_buffer));
}

::fidl::DecodeResult<Device::SetConfigurationResponse> Device::InPlace::SetConfiguration(zx::unowned_channel _client_end, ::fidl::DecodedMessage<SetConfigurationRequest> params, ::fidl::BytePart response_buffer) {
  params.message()->_hdr = {};
  params.message()->_hdr.ordinal = kDevice_SetConfiguration_Ordinal;
  auto _encode_request_result = ::fidl::Encode(std::move(params));
  if (_encode_request_result.status != ZX_OK) {
    return ::fidl::DecodeResult<Device::SetConfigurationResponse>::FromFailure(
        std::move(_encode_request_result));
  }
  auto _call_result = ::fidl::Call<SetConfigurationRequest, SetConfigurationResponse>(
    std::move(_client_end), std::move(_encode_request_result.message), std::move(response_buffer));
  if (_call_result.status != ZX_OK) {
    return ::fidl::DecodeResult<Device::SetConfigurationResponse>::FromFailure(
        std::move(_call_result));
  }
  return ::fidl::Decode(std::move(_call_result.message));
}

template <>
Device::ResultOf::ClearFunctions_Impl<Device::ClearFunctionsResponse>::ClearFunctions_Impl(zx::unowned_channel _client_end) {
  constexpr uint32_t _kWriteAllocSize = ::fidl::internal::ClampedMessageSize<ClearFunctionsRequest, ::fidl::MessageDirection::kSending>();
  ::fidl::internal::AlignedBuffer<_kWriteAllocSize> _write_bytes_inlined;
  auto& _write_bytes_array = _write_bytes_inlined;
  uint8_t* _write_bytes = _write_bytes_array.view().data();
  memset(_write_bytes, 0, ClearFunctionsRequest::PrimarySize);
  ::fidl::BytePart _request_bytes(_write_bytes, _kWriteAllocSize, sizeof(ClearFunctionsRequest));
  ::fidl::DecodedMessage<ClearFunctionsRequest> _decoded_request(std::move(_request_bytes));
  Super::SetResult(
      Device::InPlace::ClearFunctions(std::move(_client_end), Super::response_buffer()));
}

Device::ResultOf::ClearFunctions Device::SyncClient::ClearFunctions() {
  return ResultOf::ClearFunctions(zx::unowned_channel(this->channel_));
}

Device::ResultOf::ClearFunctions Device::Call::ClearFunctions(zx::unowned_channel _client_end) {
  return ResultOf::ClearFunctions(std::move(_client_end));
}

::fidl::DecodeResult<Device::ClearFunctionsResponse> Device::InPlace::ClearFunctions(zx::unowned_channel _client_end, ::fidl::BytePart response_buffer) {
  constexpr uint32_t _write_num_bytes = sizeof(ClearFunctionsRequest);
  ::fidl::internal::AlignedBuffer<_write_num_bytes> _write_bytes;
  ::fidl::BytePart _request_buffer = _write_bytes.view();
  _request_buffer.set_actual(_write_num_bytes);
  ::fidl::DecodedMessage<ClearFunctionsRequest> params(std::move(_request_buffer));
  params.message()->_hdr = {};
  params.message()->_hdr.ordinal = kDevice_ClearFunctions_Ordinal;
  auto _encode_request_result = ::fidl::Encode(std::move(params));
  if (_encode_request_result.status != ZX_OK) {
    return ::fidl::DecodeResult<Device::ClearFunctionsResponse>::FromFailure(
        std::move(_encode_request_result));
  }
  auto _call_result = ::fidl::Call<ClearFunctionsRequest, ClearFunctionsResponse>(
    std::move(_client_end), std::move(_encode_request_result.message), std::move(response_buffer));
  if (_call_result.status != ZX_OK) {
    return ::fidl::DecodeResult<Device::ClearFunctionsResponse>::FromFailure(
        std::move(_call_result));
  }
  return ::fidl::Decode(std::move(_call_result.message));
}


Device::ResultOf::SetStateChangeListener_Impl::SetStateChangeListener_Impl(zx::unowned_channel _client_end, ::zx::channel listener) {
  constexpr uint32_t _kWriteAllocSize = ::fidl::internal::ClampedMessageSize<SetStateChangeListenerRequest, ::fidl::MessageDirection::kSending>();
  ::fidl::internal::AlignedBuffer<_kWriteAllocSize> _write_bytes_inlined;
  auto& _write_bytes_array = _write_bytes_inlined;
  uint8_t* _write_bytes = _write_bytes_array.view().data();
  memset(_write_bytes, 0, SetStateChangeListenerRequest::PrimarySize);
  auto& _request = *reinterpret_cast<SetStateChangeListenerRequest*>(_write_bytes);
  _request.listener = std::move(listener);
  ::fidl::BytePart _request_bytes(_write_bytes, _kWriteAllocSize, sizeof(SetStateChangeListenerRequest));
  ::fidl::DecodedMessage<SetStateChangeListenerRequest> _decoded_request(std::move(_request_bytes));
  Super::operator=(
      Device::InPlace::SetStateChangeListener(std::move(_client_end), std::move(_decoded_request)));
}

Device::ResultOf::SetStateChangeListener Device::SyncClient::SetStateChangeListener(::zx::channel listener) {
  return ResultOf::SetStateChangeListener(zx::unowned_channel(this->channel_), std::move(listener));
}

Device::ResultOf::SetStateChangeListener Device::Call::SetStateChangeListener(zx::unowned_channel _client_end, ::zx::channel listener) {
  return ResultOf::SetStateChangeListener(std::move(_client_end), std::move(listener));
}


Device::UnownedResultOf::SetStateChangeListener_Impl::SetStateChangeListener_Impl(zx::unowned_channel _client_end, ::fidl::BytePart _request_buffer, ::zx::channel listener) {
  if (_request_buffer.capacity() < SetStateChangeListenerRequest::PrimarySize) {
    Super::status_ = ZX_ERR_BUFFER_TOO_SMALL;
    Super::error_ = ::fidl::internal::kErrorRequestBufferTooSmall;
    return;
  }
  memset(_request_buffer.data(), 0, SetStateChangeListenerRequest::PrimarySize);
  auto& _request = *reinterpret_cast<SetStateChangeListenerRequest*>(_request_buffer.data());
  _request.listener = std::move(listener);
  _request_buffer.set_actual(sizeof(SetStateChangeListenerRequest));
  ::fidl::DecodedMessage<SetStateChangeListenerRequest> _decoded_request(std::move(_request_buffer));
  Super::operator=(
      Device::InPlace::SetStateChangeListener(std::move(_client_end), std::move(_decoded_request)));
}

Device::UnownedResultOf::SetStateChangeListener Device::SyncClient::SetStateChangeListener(::fidl::BytePart _request_buffer, ::zx::channel listener) {
  return UnownedResultOf::SetStateChangeListener(zx::unowned_channel(this->channel_), std::move(_request_buffer), std::move(listener));
}

Device::UnownedResultOf::SetStateChangeListener Device::Call::SetStateChangeListener(zx::unowned_channel _client_end, ::fidl::BytePart _request_buffer, ::zx::channel listener) {
  return UnownedResultOf::SetStateChangeListener(std::move(_client_end), std::move(_request_buffer), std::move(listener));
}

::fidl::internal::StatusAndError Device::InPlace::SetStateChangeListener(zx::unowned_channel _client_end, ::fidl::DecodedMessage<SetStateChangeListenerRequest> params) {
  params.message()->_hdr = {};
  params.message()->_hdr.ordinal = kDevice_SetStateChangeListener_Ordinal;
  auto _encode_request_result = ::fidl::Encode(std::move(params));
  if (_encode_request_result.status != ZX_OK) {
    return ::fidl::internal::StatusAndError::FromFailure(
        std::move(_encode_request_result));
  }
  zx_status_t _write_status =
      ::fidl::Write(std::move(_client_end), std::move(_encode_request_result.message));
  if (_write_status != ZX_OK) {
    return ::fidl::internal::StatusAndError(_write_status, ::fidl::internal::kErrorWriteFailed);
  } else {
    return ::fidl::internal::StatusAndError(ZX_OK, nullptr);
  }
}


bool Device::TryDispatch(Interface* impl, fidl_msg_t* msg, ::fidl::Transaction* txn) {
  if (msg->num_bytes < sizeof(fidl_message_header_t)) {
    zx_handle_close_many(msg->handles, msg->num_handles);
    txn->Close(ZX_ERR_INVALID_ARGS);
    return true;
  }
  fidl_message_header_t* hdr = reinterpret_cast<fidl_message_header_t*>(msg->bytes);
  switch (hdr->ordinal) {
    case kDevice_SetConfiguration_Ordinal:
    case kDevice_SetConfiguration_GenOrdinal:
    {
      auto result = ::fidl::DecodeAs<SetConfigurationRequest>(msg);
      if (result.status != ZX_OK) {
        txn->Close(ZX_ERR_INVALID_ARGS);
        return true;
      }
      auto message = result.message.message();
      impl->SetConfiguration(std::move(message->device_desc), std::move(message->function_descriptors),
        Interface::SetConfigurationCompleter::Sync(txn));
      return true;
    }
    case kDevice_ClearFunctions_Ordinal:
    case kDevice_ClearFunctions_GenOrdinal:
    {
      auto result = ::fidl::DecodeAs<ClearFunctionsRequest>(msg);
      if (result.status != ZX_OK) {
        txn->Close(ZX_ERR_INVALID_ARGS);
        return true;
      }
      impl->ClearFunctions(
        Interface::ClearFunctionsCompleter::Sync(txn));
      return true;
    }
    case kDevice_SetStateChangeListener_Ordinal:
    case kDevice_SetStateChangeListener_GenOrdinal:
    {
      auto result = ::fidl::DecodeAs<SetStateChangeListenerRequest>(msg);
      if (result.status != ZX_OK) {
        txn->Close(ZX_ERR_INVALID_ARGS);
        return true;
      }
      auto message = result.message.message();
      impl->SetStateChangeListener(std::move(message->listener),
        Interface::SetStateChangeListenerCompleter::Sync(txn));
      return true;
    }
    default: {
      return false;
    }
  }
}

bool Device::Dispatch(Interface* impl, fidl_msg_t* msg, ::fidl::Transaction* txn) {
  bool found = TryDispatch(impl, msg, txn);
  if (!found) {
    zx_handle_close_many(msg->handles, msg->num_handles);
    txn->Close(ZX_ERR_NOT_SUPPORTED);
  }
  return found;
}


void Device::Interface::SetConfigurationCompleterBase::Reply(Device_SetConfiguration_Result result) {
  constexpr uint32_t _kWriteAllocSize = ::fidl::internal::ClampedMessageSize<SetConfigurationResponse, ::fidl::MessageDirection::kSending>();
  FIDL_ALIGNDECL uint8_t _write_bytes[_kWriteAllocSize] = {};
  auto& _response = *reinterpret_cast<SetConfigurationResponse*>(_write_bytes);
  _response._hdr.ordinal = kDevice_SetConfiguration_Ordinal;
  _response.result = std::move(result);
  ::fidl::BytePart _response_bytes(_write_bytes, _kWriteAllocSize, sizeof(SetConfigurationResponse));
  CompleterBase::SendReply(::fidl::DecodedMessage<SetConfigurationResponse>(std::move(_response_bytes)));
}
void Device::Interface::SetConfigurationCompleterBase::ReplySuccess() {
  Device_SetConfiguration_Response response;

  Reply(Device_SetConfiguration_Result::WithResponse(std::move(response)));
}
void Device::Interface::SetConfigurationCompleterBase::ReplyError(int32_t error) {
  Reply(Device_SetConfiguration_Result::WithErr(std::move(error)));
}

void Device::Interface::SetConfigurationCompleterBase::Reply(::fidl::BytePart _buffer, Device_SetConfiguration_Result result) {
  if (_buffer.capacity() < SetConfigurationResponse::PrimarySize) {
    CompleterBase::Close(ZX_ERR_INTERNAL);
    return;
  }
  auto& _response = *reinterpret_cast<SetConfigurationResponse*>(_buffer.data());
  _response._hdr.ordinal = kDevice_SetConfiguration_Ordinal;
  _response.result = std::move(result);
  _buffer.set_actual(sizeof(SetConfigurationResponse));
  CompleterBase::SendReply(::fidl::DecodedMessage<SetConfigurationResponse>(std::move(_buffer)));
}
void Device::Interface::SetConfigurationCompleterBase::ReplySuccess(::fidl::BytePart _buffer) {
  Device_SetConfiguration_Response response;

  Reply(std::move(_buffer), Device_SetConfiguration_Result::WithResponse(std::move(response)));
}

void Device::Interface::SetConfigurationCompleterBase::Reply(::fidl::DecodedMessage<SetConfigurationResponse> params) {
  params.message()->_hdr = {};
  params.message()->_hdr.ordinal = kDevice_SetConfiguration_Ordinal;
  CompleterBase::SendReply(std::move(params));
}


void Device::Interface::ClearFunctionsCompleterBase::Reply() {
  constexpr uint32_t _kWriteAllocSize = ::fidl::internal::ClampedMessageSize<ClearFunctionsResponse, ::fidl::MessageDirection::kSending>();
  FIDL_ALIGNDECL uint8_t _write_bytes[_kWriteAllocSize] = {};
  auto& _response = *reinterpret_cast<ClearFunctionsResponse*>(_write_bytes);
  _response._hdr.ordinal = kDevice_ClearFunctions_Ordinal;
  ::fidl::BytePart _response_bytes(_write_bytes, _kWriteAllocSize, sizeof(ClearFunctionsResponse));
  CompleterBase::SendReply(::fidl::DecodedMessage<ClearFunctionsResponse>(std::move(_response_bytes)));
}


}  // namespace peripheral
}  // namespace usb
}  // namespace hardware
}  // namespace fuchsia
}  // namespace llcpp
