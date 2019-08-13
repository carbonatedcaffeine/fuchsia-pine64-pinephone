// WARNING: This file is machine generated by fidlgen.

#include <fidl_llcpp_basictypes.h>
#include <memory>

namespace llcpp {

namespace fidl {
namespace test {
namespace llcpp {
namespace basictypes {

::llcpp::fidl::test::llcpp::basictypes::SimpleUnion::SimpleUnion() {
  tag_ = Tag::Invalid;
}

::llcpp::fidl::test::llcpp::basictypes::SimpleUnion::~SimpleUnion() {
  Destroy();
}

void ::llcpp::fidl::test::llcpp::basictypes::SimpleUnion::Destroy() {
  switch (which()) {
  default:
    break;
  }
  tag_ = Tag::Invalid;
}

void ::llcpp::fidl::test::llcpp::basictypes::SimpleUnion::MoveImpl_(SimpleUnion&& other) {
  switch (other.which()) {
  case Tag::kFieldA:
    mutable_field_a() = std::move(other.mutable_field_a());
    break;
  case Tag::kFieldB:
    mutable_field_b() = std::move(other.mutable_field_b());
    break;
  default:
    break;
  }
  other.Destroy();
}

void ::llcpp::fidl::test::llcpp::basictypes::SimpleUnion::SizeAndOffsetAssertionHelper() {
  static_assert(offsetof(::llcpp::fidl::test::llcpp::basictypes::SimpleUnion, field_a_) == 4);
  static_assert(offsetof(::llcpp::fidl::test::llcpp::basictypes::SimpleUnion, field_b_) == 4);
  static_assert(sizeof(::llcpp::fidl::test::llcpp::basictypes::SimpleUnion) == ::llcpp::fidl::test::llcpp::basictypes::SimpleUnion::PrimarySize);
}


int32_t& ::llcpp::fidl::test::llcpp::basictypes::SimpleUnion::mutable_field_a() {
  if (which() != Tag::kFieldA) {
    Destroy();
    new (&field_a_) int32_t;
  }
  tag_ = Tag::kFieldA;
  return field_a_;
}

int32_t& ::llcpp::fidl::test::llcpp::basictypes::SimpleUnion::mutable_field_b() {
  if (which() != Tag::kFieldB) {
    Destroy();
    new (&field_b_) int32_t;
  }
  tag_ = Tag::kFieldB;
  return field_b_;
}


namespace {

[[maybe_unused]]
constexpr uint64_t kTestInterface_ConsumeSimpleStruct_GenOrdinal = 0x2b65368b00000000lu;
extern "C" const fidl_type_t fidl_test_llcpp_basictypes_TestInterfaceConsumeSimpleStructRequestTable;
[[maybe_unused]]
constexpr uint64_t kTestInterface_ConsumeSimpleUnion_GenOrdinal = 0x2e46f97100000000lu;

}  // namespace
template <>
TestInterface::ResultOf::ConsumeSimpleStruct_Impl<TestInterface::ConsumeSimpleStructResponse>::ConsumeSimpleStruct_Impl(zx::unowned_channel _client_end, SimpleStruct arg) {
  constexpr uint32_t _kWriteAllocSize = ::fidl::internal::ClampedMessageSize<ConsumeSimpleStructRequest, ::fidl::MessageDirection::kSending>();
  ::fidl::internal::AlignedBuffer<_kWriteAllocSize> _write_bytes_inlined;
  auto& _write_bytes_array = _write_bytes_inlined;
  uint8_t* _write_bytes = _write_bytes_array.view().data();
  memset(_write_bytes, 0, ConsumeSimpleStructRequest::PrimarySize);
  auto& _request = *reinterpret_cast<ConsumeSimpleStructRequest*>(_write_bytes);
  _request.arg = std::move(arg);
  ::fidl::BytePart _request_bytes(_write_bytes, _kWriteAllocSize, sizeof(ConsumeSimpleStructRequest));
  ::fidl::DecodedMessage<ConsumeSimpleStructRequest> _decoded_request(std::move(_request_bytes));
  Super::SetResult(
      TestInterface::InPlace::ConsumeSimpleStruct(std::move(_client_end), std::move(_decoded_request), Super::response_buffer()));
}

TestInterface::ResultOf::ConsumeSimpleStruct TestInterface::SyncClient::ConsumeSimpleStruct(SimpleStruct arg) {
  return ResultOf::ConsumeSimpleStruct(zx::unowned_channel(this->channel_), std::move(arg));
}

TestInterface::ResultOf::ConsumeSimpleStruct TestInterface::Call::ConsumeSimpleStruct(zx::unowned_channel _client_end, SimpleStruct arg) {
  return ResultOf::ConsumeSimpleStruct(std::move(_client_end), std::move(arg));
}

template <>
TestInterface::UnownedResultOf::ConsumeSimpleStruct_Impl<TestInterface::ConsumeSimpleStructResponse>::ConsumeSimpleStruct_Impl(zx::unowned_channel _client_end, ::fidl::BytePart _request_buffer, SimpleStruct arg, ::fidl::BytePart _response_buffer) {
  if (_request_buffer.capacity() < ConsumeSimpleStructRequest::PrimarySize) {
    Super::SetFailure(::fidl::DecodeResult<ConsumeSimpleStructResponse>(ZX_ERR_BUFFER_TOO_SMALL, ::fidl::internal::kErrorRequestBufferTooSmall));
    return;
  }
  memset(_request_buffer.data(), 0, ConsumeSimpleStructRequest::PrimarySize);
  auto& _request = *reinterpret_cast<ConsumeSimpleStructRequest*>(_request_buffer.data());
  _request.arg = std::move(arg);
  _request_buffer.set_actual(sizeof(ConsumeSimpleStructRequest));
  ::fidl::DecodedMessage<ConsumeSimpleStructRequest> _decoded_request(std::move(_request_buffer));
  Super::SetResult(
      TestInterface::InPlace::ConsumeSimpleStruct(std::move(_client_end), std::move(_decoded_request), std::move(_response_buffer)));
}

TestInterface::UnownedResultOf::ConsumeSimpleStruct TestInterface::SyncClient::ConsumeSimpleStruct(::fidl::BytePart _request_buffer, SimpleStruct arg, ::fidl::BytePart _response_buffer) {
  return UnownedResultOf::ConsumeSimpleStruct(zx::unowned_channel(this->channel_), std::move(_request_buffer), std::move(arg), std::move(_response_buffer));
}

TestInterface::UnownedResultOf::ConsumeSimpleStruct TestInterface::Call::ConsumeSimpleStruct(zx::unowned_channel _client_end, ::fidl::BytePart _request_buffer, SimpleStruct arg, ::fidl::BytePart _response_buffer) {
  return UnownedResultOf::ConsumeSimpleStruct(std::move(_client_end), std::move(_request_buffer), std::move(arg), std::move(_response_buffer));
}

zx_status_t TestInterface::SyncClient::ConsumeSimpleStruct_Deprecated(SimpleStruct arg, int32_t* out_status, int32_t* out_field) {
  return TestInterface::Call::ConsumeSimpleStruct_Deprecated(zx::unowned_channel(this->channel_), std::move(arg), out_status, out_field);
}

zx_status_t TestInterface::Call::ConsumeSimpleStruct_Deprecated(zx::unowned_channel _client_end, SimpleStruct arg, int32_t* out_status, int32_t* out_field) {
  constexpr uint32_t _kWriteAllocSize = ::fidl::internal::ClampedMessageSize<ConsumeSimpleStructRequest, ::fidl::MessageDirection::kSending>();
  FIDL_ALIGNDECL uint8_t _write_bytes[_kWriteAllocSize] = {};
  auto& _request = *reinterpret_cast<ConsumeSimpleStructRequest*>(_write_bytes);
  _request._hdr.ordinal = kTestInterface_ConsumeSimpleStruct_GenOrdinal;
  _request.arg = std::move(arg);
  ::fidl::BytePart _request_bytes(_write_bytes, _kWriteAllocSize, sizeof(ConsumeSimpleStructRequest));
  ::fidl::DecodedMessage<ConsumeSimpleStructRequest> _decoded_request(std::move(_request_bytes));
  auto _encode_request_result = ::fidl::Encode(std::move(_decoded_request));
  if (_encode_request_result.status != ZX_OK) {
    return _encode_request_result.status;
  }
  constexpr uint32_t _kReadAllocSize = ::fidl::internal::ClampedMessageSize<ConsumeSimpleStructResponse, ::fidl::MessageDirection::kReceiving>();
  FIDL_ALIGNDECL uint8_t _read_bytes[_kReadAllocSize];
  ::fidl::BytePart _response_bytes(_read_bytes, _kReadAllocSize);
  auto _call_result = ::fidl::Call<ConsumeSimpleStructRequest, ConsumeSimpleStructResponse>(
    std::move(_client_end), std::move(_encode_request_result.message), std::move(_response_bytes));
  if (_call_result.status != ZX_OK) {
    return _call_result.status;
  }
  auto _decode_result = ::fidl::Decode(std::move(_call_result.message));
  if (_decode_result.status != ZX_OK) {
    return _decode_result.status;
  }
  auto& _response = *_decode_result.message.message();
  *out_status = std::move(_response.status);
  *out_field = std::move(_response.field);
  return ZX_OK;
}

::fidl::DecodeResult<TestInterface::ConsumeSimpleStructResponse> TestInterface::SyncClient::ConsumeSimpleStruct_Deprecated(::fidl::BytePart _request_buffer, SimpleStruct arg, ::fidl::BytePart _response_buffer, int32_t* out_status, int32_t* out_field) {
  return TestInterface::Call::ConsumeSimpleStruct_Deprecated(zx::unowned_channel(this->channel_), std::move(_request_buffer), std::move(arg), std::move(_response_buffer), out_status, out_field);
}

::fidl::DecodeResult<TestInterface::ConsumeSimpleStructResponse> TestInterface::Call::ConsumeSimpleStruct_Deprecated(zx::unowned_channel _client_end, ::fidl::BytePart _request_buffer, SimpleStruct arg, ::fidl::BytePart _response_buffer, int32_t* out_status, int32_t* out_field) {
  if (_request_buffer.capacity() < ConsumeSimpleStructRequest::PrimarySize) {
    return ::fidl::DecodeResult<ConsumeSimpleStructResponse>(ZX_ERR_BUFFER_TOO_SMALL, ::fidl::internal::kErrorRequestBufferTooSmall);
  }
  auto& _request = *reinterpret_cast<ConsumeSimpleStructRequest*>(_request_buffer.data());
  _request._hdr.ordinal = kTestInterface_ConsumeSimpleStruct_GenOrdinal;
  _request.arg = std::move(arg);
  _request_buffer.set_actual(sizeof(ConsumeSimpleStructRequest));
  ::fidl::DecodedMessage<ConsumeSimpleStructRequest> _decoded_request(std::move(_request_buffer));
  auto _encode_request_result = ::fidl::Encode(std::move(_decoded_request));
  if (_encode_request_result.status != ZX_OK) {
    return ::fidl::DecodeResult<ConsumeSimpleStructResponse>(_encode_request_result.status, _encode_request_result.error);
  }
  auto _call_result = ::fidl::Call<ConsumeSimpleStructRequest, ConsumeSimpleStructResponse>(
    std::move(_client_end), std::move(_encode_request_result.message), std::move(_response_buffer));
  if (_call_result.status != ZX_OK) {
    return ::fidl::DecodeResult<ConsumeSimpleStructResponse>(_call_result.status, _call_result.error);
  }
  auto _decode_result = ::fidl::Decode(std::move(_call_result.message));
  if (_decode_result.status != ZX_OK) {
    return _decode_result;
  }
  auto& _response = *_decode_result.message.message();
  *out_status = std::move(_response.status);
  *out_field = std::move(_response.field);
  return _decode_result;
}

::fidl::DecodeResult<TestInterface::ConsumeSimpleStructResponse> TestInterface::InPlace::ConsumeSimpleStruct(zx::unowned_channel _client_end, ::fidl::DecodedMessage<ConsumeSimpleStructRequest> params, ::fidl::BytePart response_buffer) {
  params.message()->_hdr = {};
  params.message()->_hdr.ordinal = kTestInterface_ConsumeSimpleStruct_GenOrdinal;
  auto _encode_request_result = ::fidl::Encode(std::move(params));
  if (_encode_request_result.status != ZX_OK) {
    return ::fidl::DecodeResult<TestInterface::ConsumeSimpleStructResponse>::FromFailure(
        std::move(_encode_request_result));
  }
  auto _call_result = ::fidl::Call<ConsumeSimpleStructRequest, ConsumeSimpleStructResponse>(
    std::move(_client_end), std::move(_encode_request_result.message), std::move(response_buffer));
  if (_call_result.status != ZX_OK) {
    return ::fidl::DecodeResult<TestInterface::ConsumeSimpleStructResponse>::FromFailure(
        std::move(_call_result));
  }
  return ::fidl::Decode(std::move(_call_result.message));
}

template <>
TestInterface::ResultOf::ConsumeSimpleUnion_Impl<TestInterface::ConsumeSimpleUnionResponse>::ConsumeSimpleUnion_Impl(zx::unowned_channel _client_end, SimpleUnion arg) {
  constexpr uint32_t _kWriteAllocSize = ::fidl::internal::ClampedMessageSize<ConsumeSimpleUnionRequest, ::fidl::MessageDirection::kSending>();
  ::fidl::internal::AlignedBuffer<_kWriteAllocSize> _write_bytes_inlined;
  auto& _write_bytes_array = _write_bytes_inlined;
  uint8_t* _write_bytes = _write_bytes_array.view().data();
  memset(_write_bytes, 0, ConsumeSimpleUnionRequest::PrimarySize);
  auto& _request = *reinterpret_cast<ConsumeSimpleUnionRequest*>(_write_bytes);
  _request.arg = std::move(arg);
  ::fidl::BytePart _request_bytes(_write_bytes, _kWriteAllocSize, sizeof(ConsumeSimpleUnionRequest));
  ::fidl::DecodedMessage<ConsumeSimpleUnionRequest> _decoded_request(std::move(_request_bytes));
  Super::SetResult(
      TestInterface::InPlace::ConsumeSimpleUnion(std::move(_client_end), std::move(_decoded_request), Super::response_buffer()));
}

TestInterface::ResultOf::ConsumeSimpleUnion TestInterface::SyncClient::ConsumeSimpleUnion(SimpleUnion arg) {
  return ResultOf::ConsumeSimpleUnion(zx::unowned_channel(this->channel_), std::move(arg));
}

TestInterface::ResultOf::ConsumeSimpleUnion TestInterface::Call::ConsumeSimpleUnion(zx::unowned_channel _client_end, SimpleUnion arg) {
  return ResultOf::ConsumeSimpleUnion(std::move(_client_end), std::move(arg));
}

template <>
TestInterface::UnownedResultOf::ConsumeSimpleUnion_Impl<TestInterface::ConsumeSimpleUnionResponse>::ConsumeSimpleUnion_Impl(zx::unowned_channel _client_end, ::fidl::BytePart _request_buffer, SimpleUnion arg, ::fidl::BytePart _response_buffer) {
  if (_request_buffer.capacity() < ConsumeSimpleUnionRequest::PrimarySize) {
    Super::SetFailure(::fidl::DecodeResult<ConsumeSimpleUnionResponse>(ZX_ERR_BUFFER_TOO_SMALL, ::fidl::internal::kErrorRequestBufferTooSmall));
    return;
  }
  memset(_request_buffer.data(), 0, ConsumeSimpleUnionRequest::PrimarySize);
  auto& _request = *reinterpret_cast<ConsumeSimpleUnionRequest*>(_request_buffer.data());
  _request.arg = std::move(arg);
  _request_buffer.set_actual(sizeof(ConsumeSimpleUnionRequest));
  ::fidl::DecodedMessage<ConsumeSimpleUnionRequest> _decoded_request(std::move(_request_buffer));
  Super::SetResult(
      TestInterface::InPlace::ConsumeSimpleUnion(std::move(_client_end), std::move(_decoded_request), std::move(_response_buffer)));
}

TestInterface::UnownedResultOf::ConsumeSimpleUnion TestInterface::SyncClient::ConsumeSimpleUnion(::fidl::BytePart _request_buffer, SimpleUnion arg, ::fidl::BytePart _response_buffer) {
  return UnownedResultOf::ConsumeSimpleUnion(zx::unowned_channel(this->channel_), std::move(_request_buffer), std::move(arg), std::move(_response_buffer));
}

TestInterface::UnownedResultOf::ConsumeSimpleUnion TestInterface::Call::ConsumeSimpleUnion(zx::unowned_channel _client_end, ::fidl::BytePart _request_buffer, SimpleUnion arg, ::fidl::BytePart _response_buffer) {
  return UnownedResultOf::ConsumeSimpleUnion(std::move(_client_end), std::move(_request_buffer), std::move(arg), std::move(_response_buffer));
}

zx_status_t TestInterface::SyncClient::ConsumeSimpleUnion_Deprecated(SimpleUnion arg, uint32_t* out_index, int32_t* out_field) {
  return TestInterface::Call::ConsumeSimpleUnion_Deprecated(zx::unowned_channel(this->channel_), std::move(arg), out_index, out_field);
}

zx_status_t TestInterface::Call::ConsumeSimpleUnion_Deprecated(zx::unowned_channel _client_end, SimpleUnion arg, uint32_t* out_index, int32_t* out_field) {
  constexpr uint32_t _kWriteAllocSize = ::fidl::internal::ClampedMessageSize<ConsumeSimpleUnionRequest, ::fidl::MessageDirection::kSending>();
  FIDL_ALIGNDECL uint8_t _write_bytes[_kWriteAllocSize] = {};
  auto& _request = *reinterpret_cast<ConsumeSimpleUnionRequest*>(_write_bytes);
  _request._hdr.ordinal = kTestInterface_ConsumeSimpleUnion_GenOrdinal;
  _request.arg = std::move(arg);
  ::fidl::BytePart _request_bytes(_write_bytes, _kWriteAllocSize, sizeof(ConsumeSimpleUnionRequest));
  ::fidl::DecodedMessage<ConsumeSimpleUnionRequest> _decoded_request(std::move(_request_bytes));
  auto _encode_request_result = ::fidl::Encode(std::move(_decoded_request));
  if (_encode_request_result.status != ZX_OK) {
    return _encode_request_result.status;
  }
  constexpr uint32_t _kReadAllocSize = ::fidl::internal::ClampedMessageSize<ConsumeSimpleUnionResponse, ::fidl::MessageDirection::kReceiving>();
  FIDL_ALIGNDECL uint8_t _read_bytes[_kReadAllocSize];
  ::fidl::BytePart _response_bytes(_read_bytes, _kReadAllocSize);
  auto _call_result = ::fidl::Call<ConsumeSimpleUnionRequest, ConsumeSimpleUnionResponse>(
    std::move(_client_end), std::move(_encode_request_result.message), std::move(_response_bytes));
  if (_call_result.status != ZX_OK) {
    return _call_result.status;
  }
  auto _decode_result = ::fidl::Decode(std::move(_call_result.message));
  if (_decode_result.status != ZX_OK) {
    return _decode_result.status;
  }
  auto& _response = *_decode_result.message.message();
  *out_index = std::move(_response.index);
  *out_field = std::move(_response.field);
  return ZX_OK;
}

::fidl::DecodeResult<TestInterface::ConsumeSimpleUnionResponse> TestInterface::SyncClient::ConsumeSimpleUnion_Deprecated(::fidl::BytePart _request_buffer, SimpleUnion arg, ::fidl::BytePart _response_buffer, uint32_t* out_index, int32_t* out_field) {
  return TestInterface::Call::ConsumeSimpleUnion_Deprecated(zx::unowned_channel(this->channel_), std::move(_request_buffer), std::move(arg), std::move(_response_buffer), out_index, out_field);
}

::fidl::DecodeResult<TestInterface::ConsumeSimpleUnionResponse> TestInterface::Call::ConsumeSimpleUnion_Deprecated(zx::unowned_channel _client_end, ::fidl::BytePart _request_buffer, SimpleUnion arg, ::fidl::BytePart _response_buffer, uint32_t* out_index, int32_t* out_field) {
  if (_request_buffer.capacity() < ConsumeSimpleUnionRequest::PrimarySize) {
    return ::fidl::DecodeResult<ConsumeSimpleUnionResponse>(ZX_ERR_BUFFER_TOO_SMALL, ::fidl::internal::kErrorRequestBufferTooSmall);
  }
  auto& _request = *reinterpret_cast<ConsumeSimpleUnionRequest*>(_request_buffer.data());
  _request._hdr.ordinal = kTestInterface_ConsumeSimpleUnion_GenOrdinal;
  _request.arg = std::move(arg);
  _request_buffer.set_actual(sizeof(ConsumeSimpleUnionRequest));
  ::fidl::DecodedMessage<ConsumeSimpleUnionRequest> _decoded_request(std::move(_request_buffer));
  auto _encode_request_result = ::fidl::Encode(std::move(_decoded_request));
  if (_encode_request_result.status != ZX_OK) {
    return ::fidl::DecodeResult<ConsumeSimpleUnionResponse>(_encode_request_result.status, _encode_request_result.error);
  }
  auto _call_result = ::fidl::Call<ConsumeSimpleUnionRequest, ConsumeSimpleUnionResponse>(
    std::move(_client_end), std::move(_encode_request_result.message), std::move(_response_buffer));
  if (_call_result.status != ZX_OK) {
    return ::fidl::DecodeResult<ConsumeSimpleUnionResponse>(_call_result.status, _call_result.error);
  }
  auto _decode_result = ::fidl::Decode(std::move(_call_result.message));
  if (_decode_result.status != ZX_OK) {
    return _decode_result;
  }
  auto& _response = *_decode_result.message.message();
  *out_index = std::move(_response.index);
  *out_field = std::move(_response.field);
  return _decode_result;
}

::fidl::DecodeResult<TestInterface::ConsumeSimpleUnionResponse> TestInterface::InPlace::ConsumeSimpleUnion(zx::unowned_channel _client_end, ::fidl::DecodedMessage<ConsumeSimpleUnionRequest> params, ::fidl::BytePart response_buffer) {
  params.message()->_hdr = {};
  params.message()->_hdr.ordinal = kTestInterface_ConsumeSimpleUnion_GenOrdinal;
  auto _encode_request_result = ::fidl::Encode(std::move(params));
  if (_encode_request_result.status != ZX_OK) {
    return ::fidl::DecodeResult<TestInterface::ConsumeSimpleUnionResponse>::FromFailure(
        std::move(_encode_request_result));
  }
  auto _call_result = ::fidl::Call<ConsumeSimpleUnionRequest, ConsumeSimpleUnionResponse>(
    std::move(_client_end), std::move(_encode_request_result.message), std::move(response_buffer));
  if (_call_result.status != ZX_OK) {
    return ::fidl::DecodeResult<TestInterface::ConsumeSimpleUnionResponse>::FromFailure(
        std::move(_call_result));
  }
  return ::fidl::Decode(std::move(_call_result.message));
}


bool TestInterface::TryDispatch(Interface* impl, fidl_msg_t* msg, ::fidl::Transaction* txn) {
  if (msg->num_bytes < sizeof(fidl_message_header_t)) {
    zx_handle_close_many(msg->handles, msg->num_handles);
    txn->Close(ZX_ERR_INVALID_ARGS);
    return true;
  }
  fidl_message_header_t* hdr = reinterpret_cast<fidl_message_header_t*>(msg->bytes);
  switch (hdr->ordinal) {
    case kTestInterface_ConsumeSimpleStruct_GenOrdinal:
    {
      auto result = ::fidl::DecodeAs<ConsumeSimpleStructRequest>(msg);
      if (result.status != ZX_OK) {
        txn->Close(ZX_ERR_INVALID_ARGS);
        return true;
      }
      auto message = result.message.message();
      impl->ConsumeSimpleStruct(std::move(message->arg),
        Interface::ConsumeSimpleStructCompleter::Sync(txn));
      return true;
    }
    case kTestInterface_ConsumeSimpleUnion_GenOrdinal:
    {
      auto result = ::fidl::DecodeAs<ConsumeSimpleUnionRequest>(msg);
      if (result.status != ZX_OK) {
        txn->Close(ZX_ERR_INVALID_ARGS);
        return true;
      }
      auto message = result.message.message();
      impl->ConsumeSimpleUnion(std::move(message->arg),
        Interface::ConsumeSimpleUnionCompleter::Sync(txn));
      return true;
    }
    default: {
      return false;
    }
  }
}

bool TestInterface::Dispatch(Interface* impl, fidl_msg_t* msg, ::fidl::Transaction* txn) {
  bool found = TryDispatch(impl, msg, txn);
  if (!found) {
    zx_handle_close_many(msg->handles, msg->num_handles);
    txn->Close(ZX_ERR_NOT_SUPPORTED);
  }
  return found;
}


void TestInterface::Interface::ConsumeSimpleStructCompleterBase::Reply(int32_t status, int32_t field) {
  constexpr uint32_t _kWriteAllocSize = ::fidl::internal::ClampedMessageSize<ConsumeSimpleStructResponse, ::fidl::MessageDirection::kSending>();
  FIDL_ALIGNDECL uint8_t _write_bytes[_kWriteAllocSize] = {};
  auto& _response = *reinterpret_cast<ConsumeSimpleStructResponse*>(_write_bytes);
  _response._hdr.ordinal = kTestInterface_ConsumeSimpleStruct_GenOrdinal;
  _response.status = std::move(status);
  _response.field = std::move(field);
  ::fidl::BytePart _response_bytes(_write_bytes, _kWriteAllocSize, sizeof(ConsumeSimpleStructResponse));
  CompleterBase::SendReply(::fidl::DecodedMessage<ConsumeSimpleStructResponse>(std::move(_response_bytes)));
}

void TestInterface::Interface::ConsumeSimpleStructCompleterBase::Reply(::fidl::BytePart _buffer, int32_t status, int32_t field) {
  if (_buffer.capacity() < ConsumeSimpleStructResponse::PrimarySize) {
    CompleterBase::Close(ZX_ERR_INTERNAL);
    return;
  }
  auto& _response = *reinterpret_cast<ConsumeSimpleStructResponse*>(_buffer.data());
  _response._hdr.ordinal = kTestInterface_ConsumeSimpleStruct_GenOrdinal;
  _response.status = std::move(status);
  _response.field = std::move(field);
  _buffer.set_actual(sizeof(ConsumeSimpleStructResponse));
  CompleterBase::SendReply(::fidl::DecodedMessage<ConsumeSimpleStructResponse>(std::move(_buffer)));
}

void TestInterface::Interface::ConsumeSimpleStructCompleterBase::Reply(::fidl::DecodedMessage<ConsumeSimpleStructResponse> params) {
  params.message()->_hdr = {};
  params.message()->_hdr.ordinal = kTestInterface_ConsumeSimpleStruct_GenOrdinal;
  CompleterBase::SendReply(std::move(params));
}


void TestInterface::Interface::ConsumeSimpleUnionCompleterBase::Reply(uint32_t index, int32_t field) {
  constexpr uint32_t _kWriteAllocSize = ::fidl::internal::ClampedMessageSize<ConsumeSimpleUnionResponse, ::fidl::MessageDirection::kSending>();
  FIDL_ALIGNDECL uint8_t _write_bytes[_kWriteAllocSize] = {};
  auto& _response = *reinterpret_cast<ConsumeSimpleUnionResponse*>(_write_bytes);
  _response._hdr.ordinal = kTestInterface_ConsumeSimpleUnion_GenOrdinal;
  _response.index = std::move(index);
  _response.field = std::move(field);
  ::fidl::BytePart _response_bytes(_write_bytes, _kWriteAllocSize, sizeof(ConsumeSimpleUnionResponse));
  CompleterBase::SendReply(::fidl::DecodedMessage<ConsumeSimpleUnionResponse>(std::move(_response_bytes)));
}

void TestInterface::Interface::ConsumeSimpleUnionCompleterBase::Reply(::fidl::BytePart _buffer, uint32_t index, int32_t field) {
  if (_buffer.capacity() < ConsumeSimpleUnionResponse::PrimarySize) {
    CompleterBase::Close(ZX_ERR_INTERNAL);
    return;
  }
  auto& _response = *reinterpret_cast<ConsumeSimpleUnionResponse*>(_buffer.data());
  _response._hdr.ordinal = kTestInterface_ConsumeSimpleUnion_GenOrdinal;
  _response.index = std::move(index);
  _response.field = std::move(field);
  _buffer.set_actual(sizeof(ConsumeSimpleUnionResponse));
  CompleterBase::SendReply(::fidl::DecodedMessage<ConsumeSimpleUnionResponse>(std::move(_buffer)));
}

void TestInterface::Interface::ConsumeSimpleUnionCompleterBase::Reply(::fidl::DecodedMessage<ConsumeSimpleUnionResponse> params) {
  params.message()->_hdr = {};
  params.message()->_hdr.ordinal = kTestInterface_ConsumeSimpleUnion_GenOrdinal;
  CompleterBase::SendReply(std::move(params));
}


}  // namespace basictypes
}  // namespace llcpp
}  // namespace test
}  // namespace fidl
}  // namespace llcpp
