// Copyright 2020 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#ifndef ZIRCON_KERNEL_ARCH_RISCV64_INCLUDE_ARCH_ARCH_INTERRUPT_H_
#define ZIRCON_KERNEL_ARCH_RISCV64_INCLUDE_ARCH_ARCH_INTERRUPT_H_

#include <zircon/compiler.h>

#include <arch/riscv64/interrupt.h>
#include <arch/riscv64.h>

// Note: still pulled in from some C code, remove when the last C code is gone.
__BEGIN_CDECLS

typedef bool interrupt_saved_state_t;

__WARN_UNUSED_RESULT
static inline interrupt_saved_state_t arch_interrupt_save(void) {
  /* disable interrupts by clearing the MIE bit while atomically saving the old state */
  return riscv_csr_read_clear(RISCV_CSR_XSTATUS, RISCV_CSR_XSTATUS_IE) & RISCV_CSR_XSTATUS_IE;
}

static inline void arch_interrupt_restore(interrupt_saved_state_t old_state) {
  riscv_csr_set(RISCV_CSR_XSTATUS, old_state ? RISCV_CSR_XSTATUS_IE : 0);
}

__END_CDECLS

#endif  // ZIRCON_KERNEL_ARCH_RISCV64_INCLUDE_ARCH_ARCH_INTERRUPT_H_
