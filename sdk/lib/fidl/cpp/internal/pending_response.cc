// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "lib/fidl/cpp/internal/pending_response.h"

#include "lib/fidl/cpp/internal/logging.h"
#include "lib/fidl/cpp/internal/stub_controller.h"
#include "lib/fidl/cpp/internal/weak_message_sender.h"

namespace fidl {
namespace internal {

PendingResponse::PendingResponse() : txid_(0), weak_sender_(nullptr) {}

PendingResponse::PendingResponse(zx_txid_t txid, WeakMessageSender* weak_sender)
    : txid_(txid), weak_sender_(weak_sender) {
  if (weak_sender_)
    weak_sender_->AddRef();
}

PendingResponse::~PendingResponse() {
  if (weak_sender_)
    weak_sender_->Release();
}

PendingResponse::PendingResponse(const PendingResponse& other)
    : PendingResponse(other.txid_, other.weak_sender_) {}

PendingResponse& PendingResponse::operator=(const PendingResponse& other) {
  if (this == &other)
    return *this;
  txid_ = other.txid_;
  if (weak_sender_)
    weak_sender_->Release();
  weak_sender_ = other.weak_sender_;
  if (weak_sender_)
    weak_sender_->AddRef();
  return *this;
}

PendingResponse::PendingResponse(PendingResponse&& other)
    : txid_(other.txid_), weak_sender_(other.weak_sender_) {
  other.weak_sender_ = nullptr;
}

PendingResponse& PendingResponse::operator=(PendingResponse&& other) {
  if (this == &other)
    return *this;
  txid_ = other.txid_;
  if (weak_sender_)
    weak_sender_->Release();
  weak_sender_ = other.weak_sender_;
  other.weak_sender_ = nullptr;
  return *this;
}

zx_status_t PendingResponse::Send(const fidl_type_t* type, Message message) {
  if (!weak_sender_)
    return ZX_ERR_BAD_STATE;
  MessageSender* sender = weak_sender_->sender();
  if (!sender)
    return ZX_ERR_BAD_STATE;
  message.set_txid(txid_);
  return sender->Send(type, std::move(message));
}

}  // namespace internal
}  // namespace fidl
