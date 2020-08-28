// Copyright 2020 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#ifndef ZIRCON_KERNEL_ARCH_RISCV64_INCLUDE_ARCH_CURRENT_THREAD_H_
#define ZIRCON_KERNEL_ARCH_RISCV64_INCLUDE_ARCH_CURRENT_THREAD_H_

#include <lib/arch/intrin.h>

extern "C" Thread* riscv64_get_current_thread(void);
extern "C" void riscv64_set_current_thread(Thread* t);

/* use the cpu local thread context pointer to store current_thread */
static inline Thread* arch_get_current_thread(void) {
  return riscv64_get_current_thread();
}

static inline void arch_set_current_thread(Thread* t) {
  riscv64_set_current_thread(t);
}

#endif  // ZIRCON_KERNEL_ARCH_RISCV64_INCLUDE_ARCH_CURRENT_THREAD_H_
