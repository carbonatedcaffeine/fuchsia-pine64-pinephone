// Copyright 2020 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT
#include <arch.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <inttypes.h>
#include <lib/arch/intrin.h>
#include <lib/cmdline.h>
#include <platform.h>
#include <stdlib.h>
#include <string.h>
#include <trace.h>
#include <zircon/errors.h>
#include <zircon/types.h>

#include <arch/mp.h>
#include <arch/ops.h>
#include <kernel/atomic.h>
#include <kernel/thread.h>
#include <lk/init.h>
#include <lk/main.h>

// per cpu structure, pointed to by xscratch
struct riscv64_percpu percpu[SMP_MAX_CPUS];

// called extremely early from start.S prior to getting into any other C code on
// both the boot cpu and the secondaries
extern "C" void riscv64_configure_percpu_early(uint hart_id) {
  // point xscratch at the current cpu
  // on the first cpu cpu_num should be set to 0 so we'll leave it alone
  // on secondary cpus the secondary boot code will fill in the cpu number
  riscv_csr_write(RISCV_CSR_XSCRATCH, &percpu[hart_id]);
  percpu[hart_id].hart_id = hart_id;
}

// first C level code to initialize each cpu
void riscv64_early_init_percpu(void) {
  // set the top level exception handler
  riscv_csr_write(RISCV_CSR_XTVEC, (uintptr_t)&riscv64_exception_entry);

  // mask all exceptions, just in case
  riscv_csr_clear(RISCV_CSR_XSTATUS, RISCV_CSR_XSTATUS_IE);
  riscv_csr_clear(RISCV_CSR_XIE, RISCV_CSR_XIE_SIE | RISCV_CSR_XIE_TIE | RISCV_CSR_XIE_EIE);
}

void arch_early_init() {
  riscv64_early_init_percpu();
}

void arch_prevm_init() {
}

// later init per cpu
void riscv64_init_percpu(void) {
  // enable software interrupts, used for inter-processor-interrupts
  riscv_csr_set(RISCV_CSR_XIE, RISCV_CSR_XIE_SIE);

  // enable external interrupts
  riscv_csr_set(RISCV_CSR_XIE, RISCV_CSR_XIE_EIE);
}

void arch_init() TA_NO_THREAD_SAFETY_ANALYSIS {
  riscv64_init_percpu();

  // print some arch info
  dprintf(INFO, "RISCV: Supervisor mode\n");
  dprintf(INFO, "RISCV: MMU enabled sv49\n");
  dprintf(INFO, "RISCV: SBI impl id %#lx version %#lx\n", sbi_call(SBI_GET_SBI_IMPL_ID).value, sbi_call(SBI_GET_SBI_IMPL_VERSION).value);
}

void arch_late_init_percpu(void) {
}

__NO_RETURN int arch_idle_thread_routine(void*) {
  for (;;) {
    __asm__ volatile("wfi");
  }
}

void arch_setup_uspace_iframe(iframe_t* iframe, uintptr_t pc, uintptr_t sp, uintptr_t arg1,
                              uintptr_t arg2) {
}

// Switch to user mode, set the user stack pointer to user_stack_top, put the svc stack pointer to
// the top of the kernel stack.
void arch_enter_uspace(iframe_t* iframe) {
    while(1) ;
}

/* unimplemented cache operations */
void arch_disable_cache(uint flags) { }
void arch_enable_cache(uint flags) { }

void arch_clean_cache_range(vaddr_t start, size_t len) { }
void arch_clean_invalidate_cache_range(vaddr_t start, size_t len) { }
void arch_invalidate_cache_range(vaddr_t start, size_t len) { }
void arch_sync_cache_range(vaddr_t start, size_t len) { }

