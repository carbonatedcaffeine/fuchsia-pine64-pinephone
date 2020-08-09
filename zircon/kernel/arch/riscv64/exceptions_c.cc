// Copyright 2016 The Fuchsia Authors
// Copyright (c) 2014 Travis Geiselbrecht
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT
#include <bits.h>
#include <debug.h>
#include <inttypes.h>
#include <lib/counters.h>
#include <lib/crashlog.h>
#include <platform.h>
#include <stdio.h>
#include <trace.h>
#include <zircon/syscalls/exception.h>
#include <zircon/types.h>

#include <arch/arch_ops.h>
#include <arch/exception.h>
#include <arch/thread.h>
#include <arch/user_copy.h>
#include <kernel/interrupt.h>
#include <kernel/thread.h>
#include <pretty/hexdump.h>
#include <vm/fault.h>
#include <vm/vm.h>

#define LOCAL_TRACE 0

extern void riscv64_timer_exception();

/* called from assembly */
extern "C" void arch_iframe_process_pending_signals(iframe_t* iframe) {
}

void arch_dump_exception_context(const arch_exception_context_t* context) {
}

void arch_fill_in_exception_context(const arch_exception_context_t* arch_context,
                                    zx_exception_report_t* report) {
}

zx_status_t arch_dispatch_user_policy_exception(void) {
    return ZX_OK;
}

bool arch_install_exception_context(Thread* thread, const arch_exception_context_t* context) {
    return true;
}

void arch_remove_exception_context(Thread* thread) { }

static const char *cause_to_string(long cause) {
    switch (cause) {
        case RISCV_EXCEPTION_IADDR_MISALIGN:
            return "Instruction address misaligned";
        case RISCV_EXCEPTION_IACCESS_FAULT:
            return "Instruction access fault";
        case RISCV_EXCEPTION_ILLEGAL_INS:
            return "Illegal instruction";
        case RISCV_EXCEPTION_BREAKPOINT:
            return "Breakpoint";
        case RISCV_EXCEPTION_LOAD_ADDR_MISALIGN:
            return "Load address misaligned";
        case RISCV_EXCEPTION_LOAD_ACCESS_FAULT:
            return "Load access fault";
        case RISCV_EXCEPTION_STORE_ADDR_MISALIGN:
            return "Store/AMO address misaligned";
        case RISCV_EXCEPTION_STORE_ACCESS_FAULT:
            return "Store/AMO access fault";
        case RISCV_EXCEPTION_ENV_CALL_U_MODE:
            return "Environment call from U-mode";
        case RISCV_EXCEPTION_ENV_CALL_S_MODE:
            return "Environment call from S-mode";
        case RISCV_EXCEPTION_ENV_CALL_M_MODE:
            return "Environment call from M-mode";
        case RISCV_EXCEPTION_INS_PAGE_FAULT:
            return "Instruction page fault";
        case RISCV_EXCEPTION_LOAD_PAGE_FAULT:
            return "Load page fault";
        case RISCV_EXCEPTION_STORE_PAGE_FAULT:
            return "Store/AMO page fault";
    }
    return "Unknown";
}

__NO_RETURN __NO_INLINE
static void fatal_exception(long cause, ulong epc, struct iframe_t *frame) {
    if (cause < 0) {
        panic("unhandled interrupt cause %#lx, epc %#lx, tval %#lx\n", cause, epc,
              riscv_csr_read(RISCV_CSR_XTVAL));
    } else {
        panic("unhandled exception cause %#lx (%s), epc %#lx, tval %#lx\n", cause,
              cause_to_string(cause), epc, riscv_csr_read(RISCV_CSR_XTVAL));
    }
}

static void riscv64_page_fault_handler(long cause, ulong epc, struct iframe_t *frame) {
  vaddr_t tval = riscv_csr_read(RISCV_CSR_XTVAL);
  uint pf_flags = VMM_PF_FLAG_NOT_PRESENT;
  pf_flags |= cause == RISCV_EXCEPTION_STORE_PAGE_FAULT ? VMM_PF_FLAG_WRITE : 0;
  pf_flags |= cause == RISCV_EXCEPTION_INS_PAGE_FAULT ? VMM_PF_FLAG_INSTRUCTION : 0;
  pf_flags |= is_user_address(tval) ? VMM_PF_FLAG_USER : 0; // TODO: also check whether the privilege number!
  vmm_page_fault_handler(tval, pf_flags);
}

extern "C" void riscv64_exception_handler(long cause, ulong epc, struct iframe_t *frame) {
    LTRACEF("hart %u cause %s epc %#lx status %#lx\n",
            arch_curr_cpu_num(), cause_to_string(cause), epc, frame->status);

    // top bit of the cause register determines if it's an interrupt or not
    if (cause < 0) {
        switch (cause & LONG_MAX) {
            case RISCV_INTERRUPT_XSWI: // machine software interrupt
                //riscv64_software_exception();
                break;
            case RISCV_INTERRUPT_XTIM: // machine timer interrupt
                riscv64_timer_exception();
                break;
            case RISCV_INTERRUPT_XEXT: // machine external interrupt
                //riscv64_platform_irq();
                break;
            default:
                fatal_exception(cause, epc, frame);
        }
    } else {
        // all synchronous traps go here
        switch (cause) {
            case RISCV_EXCEPTION_INS_PAGE_FAULT:
            case RISCV_EXCEPTION_LOAD_PAGE_FAULT:
            case RISCV_EXCEPTION_STORE_PAGE_FAULT:
                riscv64_page_fault_handler(cause, epc, frame);
		break;
            default:
                fatal_exception(cause, epc, frame);
        }
    }
}
