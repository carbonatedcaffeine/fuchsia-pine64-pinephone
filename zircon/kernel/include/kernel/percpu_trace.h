// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ZIRCON_KERNEL_INCLUDE_KERNEL_PERCPU_TRACE_H_
#define ZIRCON_KERNEL_INCLUDE_KERNEL_PERCPU_TRACE_H_

#include <stdint.h>
#include <zircon/compiler.h>

__BEGIN_CDECLS

void percpu_trace_log(const char* fname, const char* func, int line, const char* tag,
                      uint32_t a = 0, uint32_t b = 0, uint32_t c = 0, uint32_t d = 0,
                      void* ptr1 = nullptr, void* ptr2 = nullptr);

void percpu_trace_log_low_level(const char* fname, const char* func, int line, const char* tag,
                                uint32_t a = 0, uint32_t b = 0, uint32_t c = 0, uint32_t d = 0,
                                void* ptr1 = nullptr, void* ptr2 = nullptr);

#define PTRACE(tag, ...) percpu_trace_log(__FILE__, __func__, __LINE__, tag, ##__VA_ARGS__)
#define PTRACE_LOW_LEVEL(tag, ...) percpu_trace_log_low_level(__FILE__, __func__, __LINE__, tag, ##__VA_ARGS__)

__END_CDECLS

#endif  // ZIRCON_KERNEL_INCLUDE_KERNEL_PERCPU_TRACE_H_
