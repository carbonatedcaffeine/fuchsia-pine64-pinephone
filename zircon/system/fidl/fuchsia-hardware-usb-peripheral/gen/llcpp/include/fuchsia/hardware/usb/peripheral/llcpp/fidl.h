// WARNING: This file is machine generated by fidlgen.

#pragma once

#include <lib/fidl/internal.h>
#include <lib/fidl/llcpp/array.h>
#include <lib/fidl/llcpp/coding.h>
#include <lib/fidl/llcpp/string_view.h>
#include <lib/fidl/llcpp/sync_call.h>
#include <lib/fidl/llcpp/traits.h>
#include <lib/fidl/llcpp/transaction.h>
#include <lib/fidl/llcpp/vector_view.h>
#include <lib/fit/function.h>
#include <lib/zx/channel.h>
#include <zircon/fidl.h>

namespace llcpp {

namespace fuchsia {
namespace hardware {
namespace usb {
namespace peripheral {

class Events;
struct FunctionDescriptor;
struct Device_SetConfiguration_Response;
struct Device_SetConfiguration_Result;
struct DeviceDescriptor;
class Device;

// Events protocol that is used as a callback to inform the client
// of the completion of various server-side events.
// This callback interface can be registered using the SetStateChangeListener
// method on the Device protocol.
class Events final {
  Events() = delete;
 public:

  using FunctionRegisteredResponse = ::fidl::AnyZeroArgMessage;
  using FunctionRegisteredRequest = ::fidl::AnyZeroArgMessage;

  using FunctionsClearedRequest = ::fidl::AnyZeroArgMessage;


  // Collection of return types of FIDL calls in this interface.
  class ResultOf final {
    ResultOf() = delete;
   private:
    template <typename ResponseType>
    class FunctionRegistered_Impl final : private ::fidl::internal::OwnedSyncCallBase<ResponseType> {
      using Super = ::fidl::internal::OwnedSyncCallBase<ResponseType>;
     public:
      FunctionRegistered_Impl(zx::unowned_channel _client_end);
      ~FunctionRegistered_Impl() = default;
      FunctionRegistered_Impl(FunctionRegistered_Impl&& other) = default;
      FunctionRegistered_Impl& operator=(FunctionRegistered_Impl&& other) = default;
      using Super::status;
      using Super::error;
      using Super::ok;
      using Super::Unwrap;
      using Super::value;
      using Super::operator->;
      using Super::operator*;
    };
    class FunctionsCleared_Impl final : private ::fidl::internal::StatusAndError {
      using Super = ::fidl::internal::StatusAndError;
     public:
      FunctionsCleared_Impl(zx::unowned_channel _client_end);
      ~FunctionsCleared_Impl() = default;
      FunctionsCleared_Impl(FunctionsCleared_Impl&& other) = default;
      FunctionsCleared_Impl& operator=(FunctionsCleared_Impl&& other) = default;
      using Super::status;
      using Super::error;
      using Super::ok;
    };

   public:
    using FunctionRegistered = FunctionRegistered_Impl<FunctionRegisteredResponse>;
    using FunctionsCleared = FunctionsCleared_Impl;
  };

  // Collection of return types of FIDL calls in this interface,
  // when the caller-allocate flavor or in-place call is used.
  class UnownedResultOf final {
    UnownedResultOf() = delete;
   private:
    template <typename ResponseType>
    class FunctionRegistered_Impl final : private ::fidl::internal::UnownedSyncCallBase<ResponseType> {
      using Super = ::fidl::internal::UnownedSyncCallBase<ResponseType>;
     public:
      FunctionRegistered_Impl(zx::unowned_channel _client_end, ::fidl::BytePart _response_buffer);
      ~FunctionRegistered_Impl() = default;
      FunctionRegistered_Impl(FunctionRegistered_Impl&& other) = default;
      FunctionRegistered_Impl& operator=(FunctionRegistered_Impl&& other) = default;
      using Super::status;
      using Super::error;
      using Super::ok;
      using Super::Unwrap;
      using Super::value;
      using Super::operator->;
      using Super::operator*;
    };
    class FunctionsCleared_Impl final : private ::fidl::internal::StatusAndError {
      using Super = ::fidl::internal::StatusAndError;
     public:
      FunctionsCleared_Impl(zx::unowned_channel _client_end);
      ~FunctionsCleared_Impl() = default;
      FunctionsCleared_Impl(FunctionsCleared_Impl&& other) = default;
      FunctionsCleared_Impl& operator=(FunctionsCleared_Impl&& other) = default;
      using Super::status;
      using Super::error;
      using Super::ok;
    };

   public:
    using FunctionRegistered = FunctionRegistered_Impl<FunctionRegisteredResponse>;
    using FunctionsCleared = FunctionsCleared_Impl;
  };

  class SyncClient final {
   public:
    explicit SyncClient(::zx::channel channel) : channel_(std::move(channel)) {}
    ~SyncClient() = default;
    SyncClient(SyncClient&&) = default;
    SyncClient& operator=(SyncClient&&) = default;

    const ::zx::channel& channel() const { return channel_; }

    ::zx::channel* mutable_channel() { return &channel_; }

    // Invoked when a function registers
    // Allocates 32 bytes of message buffer on the stack. No heap allocation necessary.
    ResultOf::FunctionRegistered FunctionRegistered();


    // Invoked when all functions have been cleared.
    // Allocates 16 bytes of message buffer on the stack. No heap allocation necessary.
    ResultOf::FunctionsCleared FunctionsCleared();


   private:
    ::zx::channel channel_;
  };

  // Methods to make a sync FIDL call directly on an unowned channel, avoiding setting up a client.
  class Call final {
    Call() = delete;
   public:

    // Invoked when a function registers
    // Allocates 32 bytes of message buffer on the stack. No heap allocation necessary.
    static ResultOf::FunctionRegistered FunctionRegistered(zx::unowned_channel _client_end);


    // Invoked when all functions have been cleared.
    // Allocates 16 bytes of message buffer on the stack. No heap allocation necessary.
    static ResultOf::FunctionsCleared FunctionsCleared(zx::unowned_channel _client_end);


  };

  // Messages are encoded and decoded in-place when these methods are used.
  // Additionally, requests must be already laid-out according to the FIDL wire-format.
  class InPlace final {
    InPlace() = delete;
   public:

    // Invoked when a function registers
    static ::fidl::DecodeResult<FunctionRegisteredResponse> FunctionRegistered(zx::unowned_channel _client_end, ::fidl::BytePart response_buffer);

    // Invoked when all functions have been cleared.
    static ::fidl::internal::StatusAndError FunctionsCleared(zx::unowned_channel _client_end);

  };

  // Pure-virtual interface to be implemented by a server.
  class Interface {
   public:
    Interface() = default;
    virtual ~Interface() = default;
    using _Outer = Events;
    using _Base = ::fidl::CompleterBase;

    class FunctionRegisteredCompleterBase : public _Base {
     public:
      void Reply();

     protected:
      using ::fidl::CompleterBase::CompleterBase;
    };

    using FunctionRegisteredCompleter = ::fidl::Completer<FunctionRegisteredCompleterBase>;

    virtual void FunctionRegistered(FunctionRegisteredCompleter::Sync _completer) = 0;

    using FunctionsClearedCompleter = ::fidl::Completer<>;

    virtual void FunctionsCleared(FunctionsClearedCompleter::Sync _completer) = 0;

  };

  // Attempts to dispatch the incoming message to a handler function in the server implementation.
  // If there is no matching handler, it returns false, leaving the message and transaction intact.
  // In all other cases, it consumes the message and returns true.
  // It is possible to chain multiple TryDispatch functions in this manner.
  static bool TryDispatch(Interface* impl, fidl_msg_t* msg, ::fidl::Transaction* txn);

  // Dispatches the incoming message to one of the handlers functions in the interface.
  // If there is no matching handler, it closes all the handles in |msg| and closes the channel with
  // a |ZX_ERR_NOT_SUPPORTED| epitaph, before returning false. The message should then be discarded.
  static bool Dispatch(Interface* impl, fidl_msg_t* msg, ::fidl::Transaction* txn);

  // Same as |Dispatch|, but takes a |void*| instead of |Interface*|. Only used with |fidl::Bind|
  // to reduce template expansion.
  // Do not call this method manually. Use |Dispatch| instead.
  static bool TypeErasedDispatch(void* impl, fidl_msg_t* msg, ::fidl::Transaction* txn) {
    return Dispatch(static_cast<Interface*>(impl), msg, txn);
  }

};

constexpr uint32_t MAX_STRING_LENGTH = 127u;

constexpr uint32_t MAX_STRING_DESCRIPTORS = 255u;

constexpr uint32_t MAX_FUNCTION_DESCRIPTORS = 32u;



struct FunctionDescriptor {
  static constexpr const fidl_type_t* Type = nullptr;
  static constexpr uint32_t MaxNumHandles = 0;
  static constexpr uint32_t PrimarySize = 3;
  [[maybe_unused]]
  static constexpr uint32_t MaxOutOfLine = 0;

  uint8_t interface_class = {};

  uint8_t interface_subclass = {};

  uint8_t interface_protocol = {};
};



struct Device_SetConfiguration_Response {
  static constexpr const fidl_type_t* Type = nullptr;
  static constexpr uint32_t MaxNumHandles = 0;
  static constexpr uint32_t PrimarySize = 1;
  [[maybe_unused]]
  static constexpr uint32_t MaxOutOfLine = 0;

  uint8_t __reserved = {};
};

extern "C" const fidl_type_t fuchsia_hardware_usb_peripheral_Device_SetConfiguration_ResultTable;

struct Device_SetConfiguration_Result {
  enum class Tag : fidl_union_tag_t {
    kResponse = 0,
    kErr = 1,
    Invalid = ::std::numeric_limits<::fidl_union_tag_t>::max(),
  };

  Device_SetConfiguration_Result();
  ~Device_SetConfiguration_Result();

  Device_SetConfiguration_Result(Device_SetConfiguration_Result&& other) {
    tag_ = Tag::Invalid;
    if (this != &other) {
      MoveImpl_(std::move(other));
    }
  }

  Device_SetConfiguration_Result& operator=(Device_SetConfiguration_Result&& other) {
    if (this != &other) {
      MoveImpl_(std::move(other));
    }
    return *this;
  }

  bool has_invalid_tag() const { return tag_ == Tag::Invalid; }

  bool is_response() const { return tag_ == Tag::kResponse; }

  static Device_SetConfiguration_Result WithResponse(Device_SetConfiguration_Response&& val) {
    Device_SetConfiguration_Result result;
    result.set_response(std::move(val));
    return result;
  }

  Device_SetConfiguration_Response& mutable_response();

  template <typename T>
  std::enable_if_t<std::is_convertible<T, Device_SetConfiguration_Response>::value && std::is_copy_assignable<T>::value>
  set_response(const T& v) {
    mutable_response() = v;
  }

  template <typename T>
  std::enable_if_t<std::is_convertible<T, Device_SetConfiguration_Response>::value && std::is_move_assignable<T>::value>
  set_response(T&& v) {
    mutable_response() = std::move(v);
  }

  Device_SetConfiguration_Response const & response() const { return response_; }

  bool is_err() const { return tag_ == Tag::kErr; }

  static Device_SetConfiguration_Result WithErr(int32_t&& val) {
    Device_SetConfiguration_Result result;
    result.set_err(std::move(val));
    return result;
  }

  int32_t& mutable_err();

  template <typename T>
  std::enable_if_t<std::is_convertible<T, int32_t>::value && std::is_copy_assignable<T>::value>
  set_err(const T& v) {
    mutable_err() = v;
  }

  template <typename T>
  std::enable_if_t<std::is_convertible<T, int32_t>::value && std::is_move_assignable<T>::value>
  set_err(T&& v) {
    mutable_err() = std::move(v);
  }

  int32_t const & err() const { return err_; }

  Tag which() const { return tag_; }

  static constexpr const fidl_type_t* Type = &fuchsia_hardware_usb_peripheral_Device_SetConfiguration_ResultTable;
  static constexpr uint32_t MaxNumHandles = 0;
  static constexpr uint32_t PrimarySize = 8;
  [[maybe_unused]]
  static constexpr uint32_t MaxOutOfLine = 0;

 private:
  void Destroy();
  void MoveImpl_(Device_SetConfiguration_Result&& other);
  static void SizeAndOffsetAssertionHelper();
  Tag tag_;
  union {
    Device_SetConfiguration_Response response_;
    int32_t err_;
  };
};

extern "C" const fidl_type_t fuchsia_hardware_usb_peripheral_DeviceDescriptorTable;

// The fields in DeviceDescriptor match those in usb_descriptor_t in the USB specification,
// except for the string fields.
struct DeviceDescriptor {
  static constexpr const fidl_type_t* Type = &fuchsia_hardware_usb_peripheral_DeviceDescriptorTable;
  static constexpr uint32_t MaxNumHandles = 0;
  static constexpr uint32_t PrimarySize = 72;
  [[maybe_unused]]
  static constexpr uint32_t MaxOutOfLine = 384;

  uint16_t bcdUSB = {};

  uint8_t bDeviceClass = {};

  uint8_t bDeviceSubClass = {};

  uint8_t bDeviceProtocol = {};

  uint8_t bMaxPacketSize0 = {};

  uint16_t idVendor = {};

  uint16_t idProduct = {};

  uint16_t bcdDevice = {};

  ::fidl::StringView manufacturer = {};

  ::fidl::StringView product = {};

  ::fidl::StringView serial = {};

  uint8_t bNumConfigurations = {};
};

extern "C" const fidl_type_t fuchsia_hardware_usb_peripheral_DeviceSetConfigurationRequestTable;
extern "C" const fidl_type_t fuchsia_hardware_usb_peripheral_DeviceSetConfigurationResponseTable;
extern "C" const fidl_type_t fuchsia_hardware_usb_peripheral_DeviceSetStateChangeListenerRequestTable;

class Device final {
  Device() = delete;
 public:

  struct SetConfigurationResponse final {
    FIDL_ALIGNDECL
    fidl_message_header_t _hdr;
    Device_SetConfiguration_Result result;

    static constexpr const fidl_type_t* Type = &fuchsia_hardware_usb_peripheral_DeviceSetConfigurationResponseTable;
    static constexpr uint32_t MaxNumHandles = 0;
    static constexpr uint32_t PrimarySize = 24;
    static constexpr uint32_t MaxOutOfLine = 0;
    static constexpr bool HasFlexibleEnvelope = false;
    static constexpr ::fidl::internal::TransactionalMessageKind MessageKind =
        ::fidl::internal::TransactionalMessageKind::kResponse;
  };
  struct SetConfigurationRequest final {
    FIDL_ALIGNDECL
    fidl_message_header_t _hdr;
    DeviceDescriptor device_desc;
    ::fidl::VectorView<FunctionDescriptor> function_descriptors;

    static constexpr const fidl_type_t* Type = &fuchsia_hardware_usb_peripheral_DeviceSetConfigurationRequestTable;
    static constexpr uint32_t MaxNumHandles = 0;
    static constexpr uint32_t PrimarySize = 104;
    static constexpr uint32_t MaxOutOfLine = 480;
    static constexpr bool HasFlexibleEnvelope = false;
    static constexpr ::fidl::internal::TransactionalMessageKind MessageKind =
        ::fidl::internal::TransactionalMessageKind::kRequest;
    using ResponseType = SetConfigurationResponse;
  };

  using ClearFunctionsResponse = ::fidl::AnyZeroArgMessage;
  using ClearFunctionsRequest = ::fidl::AnyZeroArgMessage;

  struct SetStateChangeListenerRequest final {
    FIDL_ALIGNDECL
    fidl_message_header_t _hdr;
    ::zx::channel listener;

    static constexpr const fidl_type_t* Type = &fuchsia_hardware_usb_peripheral_DeviceSetStateChangeListenerRequestTable;
    static constexpr uint32_t MaxNumHandles = 1;
    static constexpr uint32_t PrimarySize = 24;
    static constexpr uint32_t MaxOutOfLine = 0;
    static constexpr bool HasFlexibleEnvelope = false;
    static constexpr ::fidl::internal::TransactionalMessageKind MessageKind =
        ::fidl::internal::TransactionalMessageKind::kRequest;
  };


  // Collection of return types of FIDL calls in this interface.
  class ResultOf final {
    ResultOf() = delete;
   private:
    template <typename ResponseType>
    class SetConfiguration_Impl final : private ::fidl::internal::OwnedSyncCallBase<ResponseType> {
      using Super = ::fidl::internal::OwnedSyncCallBase<ResponseType>;
     public:
      SetConfiguration_Impl(zx::unowned_channel _client_end, DeviceDescriptor device_desc, ::fidl::VectorView<FunctionDescriptor> function_descriptors);
      ~SetConfiguration_Impl() = default;
      SetConfiguration_Impl(SetConfiguration_Impl&& other) = default;
      SetConfiguration_Impl& operator=(SetConfiguration_Impl&& other) = default;
      using Super::status;
      using Super::error;
      using Super::ok;
      using Super::Unwrap;
      using Super::value;
      using Super::operator->;
      using Super::operator*;
    };
    template <typename ResponseType>
    class ClearFunctions_Impl final : private ::fidl::internal::OwnedSyncCallBase<ResponseType> {
      using Super = ::fidl::internal::OwnedSyncCallBase<ResponseType>;
     public:
      ClearFunctions_Impl(zx::unowned_channel _client_end);
      ~ClearFunctions_Impl() = default;
      ClearFunctions_Impl(ClearFunctions_Impl&& other) = default;
      ClearFunctions_Impl& operator=(ClearFunctions_Impl&& other) = default;
      using Super::status;
      using Super::error;
      using Super::ok;
      using Super::Unwrap;
      using Super::value;
      using Super::operator->;
      using Super::operator*;
    };
    class SetStateChangeListener_Impl final : private ::fidl::internal::StatusAndError {
      using Super = ::fidl::internal::StatusAndError;
     public:
      SetStateChangeListener_Impl(zx::unowned_channel _client_end, ::zx::channel listener);
      ~SetStateChangeListener_Impl() = default;
      SetStateChangeListener_Impl(SetStateChangeListener_Impl&& other) = default;
      SetStateChangeListener_Impl& operator=(SetStateChangeListener_Impl&& other) = default;
      using Super::status;
      using Super::error;
      using Super::ok;
    };

   public:
    using SetConfiguration = SetConfiguration_Impl<SetConfigurationResponse>;
    using ClearFunctions = ClearFunctions_Impl<ClearFunctionsResponse>;
    using SetStateChangeListener = SetStateChangeListener_Impl;
  };

  // Collection of return types of FIDL calls in this interface,
  // when the caller-allocate flavor or in-place call is used.
  class UnownedResultOf final {
    UnownedResultOf() = delete;
   private:
    template <typename ResponseType>
    class SetConfiguration_Impl final : private ::fidl::internal::UnownedSyncCallBase<ResponseType> {
      using Super = ::fidl::internal::UnownedSyncCallBase<ResponseType>;
     public:
      SetConfiguration_Impl(zx::unowned_channel _client_end, ::fidl::BytePart _request_buffer, DeviceDescriptor device_desc, ::fidl::VectorView<FunctionDescriptor> function_descriptors, ::fidl::BytePart _response_buffer);
      ~SetConfiguration_Impl() = default;
      SetConfiguration_Impl(SetConfiguration_Impl&& other) = default;
      SetConfiguration_Impl& operator=(SetConfiguration_Impl&& other) = default;
      using Super::status;
      using Super::error;
      using Super::ok;
      using Super::Unwrap;
      using Super::value;
      using Super::operator->;
      using Super::operator*;
    };
    template <typename ResponseType>
    class ClearFunctions_Impl final : private ::fidl::internal::UnownedSyncCallBase<ResponseType> {
      using Super = ::fidl::internal::UnownedSyncCallBase<ResponseType>;
     public:
      ClearFunctions_Impl(zx::unowned_channel _client_end, ::fidl::BytePart _response_buffer);
      ~ClearFunctions_Impl() = default;
      ClearFunctions_Impl(ClearFunctions_Impl&& other) = default;
      ClearFunctions_Impl& operator=(ClearFunctions_Impl&& other) = default;
      using Super::status;
      using Super::error;
      using Super::ok;
      using Super::Unwrap;
      using Super::value;
      using Super::operator->;
      using Super::operator*;
    };
    class SetStateChangeListener_Impl final : private ::fidl::internal::StatusAndError {
      using Super = ::fidl::internal::StatusAndError;
     public:
      SetStateChangeListener_Impl(zx::unowned_channel _client_end, ::fidl::BytePart _request_buffer, ::zx::channel listener);
      ~SetStateChangeListener_Impl() = default;
      SetStateChangeListener_Impl(SetStateChangeListener_Impl&& other) = default;
      SetStateChangeListener_Impl& operator=(SetStateChangeListener_Impl&& other) = default;
      using Super::status;
      using Super::error;
      using Super::ok;
    };

   public:
    using SetConfiguration = SetConfiguration_Impl<SetConfigurationResponse>;
    using ClearFunctions = ClearFunctions_Impl<ClearFunctionsResponse>;
    using SetStateChangeListener = SetStateChangeListener_Impl;
  };

  class SyncClient final {
   public:
    explicit SyncClient(::zx::channel channel) : channel_(std::move(channel)) {}
    ~SyncClient() = default;
    SyncClient(SyncClient&&) = default;
    SyncClient& operator=(SyncClient&&) = default;

    const ::zx::channel& channel() const { return channel_; }

    ::zx::channel* mutable_channel() { return &channel_; }

    // Sets the device's descriptors, adds the functions and creates the child devices for the
    // configuration's interfaces.
    // At least one function descriptor must be provided.
    // Allocates 24 bytes of response buffer on the stack. Request is heap-allocated.
    ResultOf::SetConfiguration SetConfiguration(DeviceDescriptor device_desc, ::fidl::VectorView<FunctionDescriptor> function_descriptors);

    // Sets the device's descriptors, adds the functions and creates the child devices for the
    // configuration's interfaces.
    // At least one function descriptor must be provided.
    // Caller provides the backing storage for FIDL message via request and response buffers.
    UnownedResultOf::SetConfiguration SetConfiguration(::fidl::BytePart _request_buffer, DeviceDescriptor device_desc, ::fidl::VectorView<FunctionDescriptor> function_descriptors, ::fidl::BytePart _response_buffer);

    // Tells the device to remove the child devices for the configuration's interfaces
    // and reset the list of functions to empty.
    // The caller should wait for the |FunctionsCleared| event.
    // Allocates 32 bytes of message buffer on the stack. No heap allocation necessary.
    ResultOf::ClearFunctions ClearFunctions();


    // Adds a state change listener that is invoked when a state change completes.
    // Allocates 24 bytes of message buffer on the stack. No heap allocation necessary.
    ResultOf::SetStateChangeListener SetStateChangeListener(::zx::channel listener);

    // Adds a state change listener that is invoked when a state change completes.
    // Caller provides the backing storage for FIDL message via request and response buffers.
    UnownedResultOf::SetStateChangeListener SetStateChangeListener(::fidl::BytePart _request_buffer, ::zx::channel listener);

   private:
    ::zx::channel channel_;
  };

  // Methods to make a sync FIDL call directly on an unowned channel, avoiding setting up a client.
  class Call final {
    Call() = delete;
   public:

    // Sets the device's descriptors, adds the functions and creates the child devices for the
    // configuration's interfaces.
    // At least one function descriptor must be provided.
    // Allocates 24 bytes of response buffer on the stack. Request is heap-allocated.
    static ResultOf::SetConfiguration SetConfiguration(zx::unowned_channel _client_end, DeviceDescriptor device_desc, ::fidl::VectorView<FunctionDescriptor> function_descriptors);

    // Sets the device's descriptors, adds the functions and creates the child devices for the
    // configuration's interfaces.
    // At least one function descriptor must be provided.
    // Caller provides the backing storage for FIDL message via request and response buffers.
    static UnownedResultOf::SetConfiguration SetConfiguration(zx::unowned_channel _client_end, ::fidl::BytePart _request_buffer, DeviceDescriptor device_desc, ::fidl::VectorView<FunctionDescriptor> function_descriptors, ::fidl::BytePart _response_buffer);

    // Tells the device to remove the child devices for the configuration's interfaces
    // and reset the list of functions to empty.
    // The caller should wait for the |FunctionsCleared| event.
    // Allocates 32 bytes of message buffer on the stack. No heap allocation necessary.
    static ResultOf::ClearFunctions ClearFunctions(zx::unowned_channel _client_end);


    // Adds a state change listener that is invoked when a state change completes.
    // Allocates 24 bytes of message buffer on the stack. No heap allocation necessary.
    static ResultOf::SetStateChangeListener SetStateChangeListener(zx::unowned_channel _client_end, ::zx::channel listener);

    // Adds a state change listener that is invoked when a state change completes.
    // Caller provides the backing storage for FIDL message via request and response buffers.
    static UnownedResultOf::SetStateChangeListener SetStateChangeListener(zx::unowned_channel _client_end, ::fidl::BytePart _request_buffer, ::zx::channel listener);

  };

  // Messages are encoded and decoded in-place when these methods are used.
  // Additionally, requests must be already laid-out according to the FIDL wire-format.
  class InPlace final {
    InPlace() = delete;
   public:

    // Sets the device's descriptors, adds the functions and creates the child devices for the
    // configuration's interfaces.
    // At least one function descriptor must be provided.
    static ::fidl::DecodeResult<SetConfigurationResponse> SetConfiguration(zx::unowned_channel _client_end, ::fidl::DecodedMessage<SetConfigurationRequest> params, ::fidl::BytePart response_buffer);

    // Tells the device to remove the child devices for the configuration's interfaces
    // and reset the list of functions to empty.
    // The caller should wait for the |FunctionsCleared| event.
    static ::fidl::DecodeResult<ClearFunctionsResponse> ClearFunctions(zx::unowned_channel _client_end, ::fidl::BytePart response_buffer);

    // Adds a state change listener that is invoked when a state change completes.
    static ::fidl::internal::StatusAndError SetStateChangeListener(zx::unowned_channel _client_end, ::fidl::DecodedMessage<SetStateChangeListenerRequest> params);

  };

  // Pure-virtual interface to be implemented by a server.
  class Interface {
   public:
    Interface() = default;
    virtual ~Interface() = default;
    using _Outer = Device;
    using _Base = ::fidl::CompleterBase;

    class SetConfigurationCompleterBase : public _Base {
     public:
      void Reply(Device_SetConfiguration_Result result);
      void ReplySuccess();
      void ReplyError(int32_t error);
      void Reply(::fidl::BytePart _buffer, Device_SetConfiguration_Result result);
      void ReplySuccess(::fidl::BytePart _buffer);
      void Reply(::fidl::DecodedMessage<SetConfigurationResponse> params);

     protected:
      using ::fidl::CompleterBase::CompleterBase;
    };

    using SetConfigurationCompleter = ::fidl::Completer<SetConfigurationCompleterBase>;

    virtual void SetConfiguration(DeviceDescriptor device_desc, ::fidl::VectorView<FunctionDescriptor> function_descriptors, SetConfigurationCompleter::Sync _completer) = 0;

    class ClearFunctionsCompleterBase : public _Base {
     public:
      void Reply();

     protected:
      using ::fidl::CompleterBase::CompleterBase;
    };

    using ClearFunctionsCompleter = ::fidl::Completer<ClearFunctionsCompleterBase>;

    virtual void ClearFunctions(ClearFunctionsCompleter::Sync _completer) = 0;

    using SetStateChangeListenerCompleter = ::fidl::Completer<>;

    virtual void SetStateChangeListener(::zx::channel listener, SetStateChangeListenerCompleter::Sync _completer) = 0;

  };

  // Attempts to dispatch the incoming message to a handler function in the server implementation.
  // If there is no matching handler, it returns false, leaving the message and transaction intact.
  // In all other cases, it consumes the message and returns true.
  // It is possible to chain multiple TryDispatch functions in this manner.
  static bool TryDispatch(Interface* impl, fidl_msg_t* msg, ::fidl::Transaction* txn);

  // Dispatches the incoming message to one of the handlers functions in the interface.
  // If there is no matching handler, it closes all the handles in |msg| and closes the channel with
  // a |ZX_ERR_NOT_SUPPORTED| epitaph, before returning false. The message should then be discarded.
  static bool Dispatch(Interface* impl, fidl_msg_t* msg, ::fidl::Transaction* txn);

  // Same as |Dispatch|, but takes a |void*| instead of |Interface*|. Only used with |fidl::Bind|
  // to reduce template expansion.
  // Do not call this method manually. Use |Dispatch| instead.
  static bool TypeErasedDispatch(void* impl, fidl_msg_t* msg, ::fidl::Transaction* txn) {
    return Dispatch(static_cast<Interface*>(impl), msg, txn);
  }

};

}  // namespace peripheral
}  // namespace usb
}  // namespace hardware
}  // namespace fuchsia
}  // namespace llcpp

namespace fidl {

template <>
struct IsFidlType<::llcpp::fuchsia::hardware::usb::peripheral::FunctionDescriptor> : public std::true_type {};
static_assert(std::is_standard_layout_v<::llcpp::fuchsia::hardware::usb::peripheral::FunctionDescriptor>);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::FunctionDescriptor, interface_class) == 0);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::FunctionDescriptor, interface_subclass) == 1);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::FunctionDescriptor, interface_protocol) == 2);
static_assert(sizeof(::llcpp::fuchsia::hardware::usb::peripheral::FunctionDescriptor) == ::llcpp::fuchsia::hardware::usb::peripheral::FunctionDescriptor::PrimarySize);

template <>
struct IsFidlType<::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Response> : public std::true_type {};
static_assert(std::is_standard_layout_v<::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Response>);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Response, __reserved) == 0);
static_assert(sizeof(::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Response) == ::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Response::PrimarySize);

template <>
struct IsFidlType<::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Result> : public std::true_type {};
static_assert(std::is_standard_layout_v<::llcpp::fuchsia::hardware::usb::peripheral::Device_SetConfiguration_Result>);

template <>
struct IsFidlType<::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor> : public std::true_type {};
static_assert(std::is_standard_layout_v<::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor>);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor, bcdUSB) == 0);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor, bDeviceClass) == 2);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor, bDeviceSubClass) == 3);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor, bDeviceProtocol) == 4);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor, bMaxPacketSize0) == 5);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor, idVendor) == 6);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor, idProduct) == 8);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor, bcdDevice) == 10);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor, manufacturer) == 16);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor, product) == 32);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor, serial) == 48);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor, bNumConfigurations) == 64);
static_assert(sizeof(::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor) == ::llcpp::fuchsia::hardware::usb::peripheral::DeviceDescriptor::PrimarySize);

template <>
struct IsFidlType<::llcpp::fuchsia::hardware::usb::peripheral::Device::SetConfigurationRequest> : public std::true_type {};
template <>
struct IsFidlMessage<::llcpp::fuchsia::hardware::usb::peripheral::Device::SetConfigurationRequest> : public std::true_type {};
static_assert(sizeof(::llcpp::fuchsia::hardware::usb::peripheral::Device::SetConfigurationRequest)
    == ::llcpp::fuchsia::hardware::usb::peripheral::Device::SetConfigurationRequest::PrimarySize);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::Device::SetConfigurationRequest, device_desc) == 16);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::Device::SetConfigurationRequest, function_descriptors) == 88);

template <>
struct IsFidlType<::llcpp::fuchsia::hardware::usb::peripheral::Device::SetConfigurationResponse> : public std::true_type {};
template <>
struct IsFidlMessage<::llcpp::fuchsia::hardware::usb::peripheral::Device::SetConfigurationResponse> : public std::true_type {};
static_assert(sizeof(::llcpp::fuchsia::hardware::usb::peripheral::Device::SetConfigurationResponse)
    == ::llcpp::fuchsia::hardware::usb::peripheral::Device::SetConfigurationResponse::PrimarySize);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::Device::SetConfigurationResponse, result) == 16);

template <>
struct IsFidlType<::llcpp::fuchsia::hardware::usb::peripheral::Device::SetStateChangeListenerRequest> : public std::true_type {};
template <>
struct IsFidlMessage<::llcpp::fuchsia::hardware::usb::peripheral::Device::SetStateChangeListenerRequest> : public std::true_type {};
static_assert(sizeof(::llcpp::fuchsia::hardware::usb::peripheral::Device::SetStateChangeListenerRequest)
    == ::llcpp::fuchsia::hardware::usb::peripheral::Device::SetStateChangeListenerRequest::PrimarySize);
static_assert(offsetof(::llcpp::fuchsia::hardware::usb::peripheral::Device::SetStateChangeListenerRequest, listener) == 16);

}  // namespace fidl
