// Copyright 2020 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#ifndef ZIRCON_KERNEL_ARCH_RISCV64_INCLUDE_ARCH_RISCV64_H_
#define ZIRCON_KERNEL_ARCH_RISCV64_INCLUDE_ARCH_RISCV64_H_

#define RISCV_USER_OFFSET   (0)
#define RISCV_SUPER_OFFSET  (1)
#define RISCV_HYPER_OFFSET  (2)
#define RISCV_MACH_OFFSET   (3)

#define RISCV_XMODE_OFFSET     (RISCV_SUPER_OFFSET)
#define RISCV_XRET             sret

#define RISCV_CSR_XMODE_BITS     (RISCV_XMODE_OFFSET << 8)

// These CSRs are only in user CSR space (still readable by all modes though)
#define RISCV_CSR_CYCLE     (0xc00)
#define RISCV_CSR_TIME      (0xc01)
#define RISCV_CSR_INSRET    (0xc02)
#define RISCV_CSR_CYCLEH    (0xc80)
#define RISCV_CSR_TIMEH     (0xc81)
#define RISCV_CSR_INSRETH   (0xc82)

#define RISCV_CSR_SATP      (0x180)

#define RISCV_CSR_XSTATUS   (0x000 | RISCV_CSR_XMODE_BITS)
#define RISCV_CSR_XIE       (0x004 | RISCV_CSR_XMODE_BITS)
#define RISCV_CSR_XTVEC     (0x005 | RISCV_CSR_XMODE_BITS)
#define RISCV_CSR_XSCRATCH  (0x040 | RISCV_CSR_XMODE_BITS)
#define RISCV_CSR_XEPC      (0x041 | RISCV_CSR_XMODE_BITS)
#define RISCV_CSR_XCAUSE    (0x042 | RISCV_CSR_XMODE_BITS)
#define RISCV_CSR_XTVAL     (0x043 | RISCV_CSR_XMODE_BITS)
#define RISCV_CSR_XIP       (0x044 | RISCV_CSR_XMODE_BITS)

#define RISCV_CSR_XSTATUS_IE    (1u << (RISCV_XMODE_OFFSET + 0))
#define RISCV_CSR_XSTATUS_PIE   (1u << (RISCV_XMODE_OFFSET + 4))

#define RISCV_CSR_XIE_SIE       (1u << (RISCV_XMODE_OFFSET + 0))
#define RISCV_CSR_XIE_TIE       (1u << (RISCV_XMODE_OFFSET + 4))
#define RISCV_CSR_XIE_EIE       (1u << (RISCV_XMODE_OFFSET + 8))

#define RISCV_CSR_XIP_SIP       (1u << (RISCV_XMODE_OFFSET + 0))
#define RISCV_CSR_XIP_TIP       (1u << (RISCV_XMODE_OFFSET + 4))
#define RISCV_CSR_XIP_EIP       (1u << (RISCV_XMODE_OFFSET + 8))

// Interrupts, top bit set in cause register
#define RISCV_INTERRUPT_USWI        0       // software interrupt
#define RISCV_INTERRUPT_SSWI        1
#define RISCV_INTERRUPT_MSWI        3
#define RISCV_INTERRUPT_UTIM        4       // timer interrupt
#define RISCV_INTERRUPT_STIM        5
#define RISCV_INTERRUPT_MTIM        7
#define RISCV_INTERRUPT_UEXT        8       // external interrupt
#define RISCV_INTERRUPT_SEXT        9
#define RISCV_INTERRUPT_MEXT        11

// The 3 interupts above, for the current mode
#define RISCV_INTERRUPT_XSWI        (RISCV_XMODE_OFFSET)
#define RISCV_INTERRUPT_XTIM        (4 + RISCV_XMODE_OFFSET)
#define RISCV_INTERRUPT_XEXT        (8 + RISCV_XMODE_OFFSET)

// Exceptions
#define RISCV_EXCEPTION_IADDR_MISALIGN      0
#define RISCV_EXCEPTION_IACCESS_FAULT       1
#define RISCV_EXCEPTION_ILLEGAL_INS         2
#define RISCV_EXCEPTION_BREAKPOINT          3
#define RISCV_EXCEPTION_LOAD_ADDR_MISALIGN  4
#define RISCV_EXCEPTION_LOAD_ACCESS_FAULT   5
#define RISCV_EXCEPTION_STORE_ADDR_MISALIGN 6
#define RISCV_EXCEPTION_STORE_ACCESS_FAULT  7
#define RISCV_EXCEPTION_ENV_CALL_U_MODE     8
#define RISCV_EXCEPTION_ENV_CALL_S_MODE     9
#define RISCV_EXCEPTION_ENV_CALL_M_MODE     11
#define RISCV_EXCEPTION_INS_PAGE_FAULT      12
#define RISCV_EXCEPTION_LOAD_PAGE_FAULT     13
#define RISCV_EXCEPTION_STORE_PAGE_FAULT    15

#ifndef __ASSEMBLER__

#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <zircon/compiler.h>
#include <zircon/types.h>

#include <arch/riscv64/iframe.h>
#include <arch/defines.h>

__BEGIN_CDECLS

struct arch_exception_context {
    struct iframe_t *frame;
};

struct Thread;

__END_CDECLS

#define __ASM_STR(x)    #x

#define riscv_csr_clear(csr, bits) \
({ \
    ulong __val = bits; \
    __asm__ volatile( \
        "csrc   " __ASM_STR(csr) ", %0" \
        :: "rK" (__val) \
        : "memory"); \
})

#define riscv_csr_read_clear(csr, bits) \
({ \
    ulong __val = bits; \
    ulong __val_out; \
    __asm__ volatile( \
        "csrrc   %0, " __ASM_STR(csr) ", %1" \
        : "=r"(__val_out) \
        : "rK" (__val) \
        : "memory"); \
    __val_out; \
})

#define riscv_csr_set(csr, bits) \
({ \
    ulong __val = bits; \
    __asm__ volatile( \
        "csrs   " __ASM_STR(csr) ", %0" \
        :: "rK" (__val) \
        : "memory"); \
})

#define riscv_csr_read(csr) \
({ \
    ulong __val; \
    __asm__ volatile( \
        "csrr   %0, " __ASM_STR(csr) \
        : "=r" (__val) \
        :: "memory"); \
    __val; \
})

#define riscv_csr_write(csr, val) \
({ \
    ulong __val = (ulong)val; \
    __asm__ volatile( \
        "csrw   " __ASM_STR(csr) ", %0" \
        :: "rK" (__val) \
        : "memory"); \
    __val; \
})

#endif  // __ASSEMBLER__

#endif  // ZIRCON_KERNEL_ARCH_RISCV64_INCLUDE_ARCH_RISCV64_H_
