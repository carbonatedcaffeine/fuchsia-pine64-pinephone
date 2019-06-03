// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "lib/fidl/cpp/internal/weak_message_sender.h"

namespace fidl {
namespace internal {

WeakMessageSender::WeakMessageSender(MessageSender* sender)
    : ref_count_(1u), sender_(sender) {}

WeakMessageSender::~WeakMessageSender() = default;

void WeakMessageSender::AddRef() { ++ref_count_; }

void WeakMessageSender::Release() {
  if (--ref_count_ == 0)
    delete this;
}

void WeakMessageSender::Invalidate() { sender_ = nullptr; }

}  // namespace internal
}  // namespace fidl
