// Copyright 2020 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#include <asm.h>
#include <arch/riscv64.h>

#define REGOFF(x) ((x) * 8)

/* top level exception handler for riscv64 in non vectored mode */
.balign 4
FUNCTION(riscv64_exception_entry)
    csrrw  t6, sscratch, t6

    /* dump all the callee trashed regs on the per-cpu structure */
    sd     t5, REGOFF(17)(t6)
    sd     t4, REGOFF(16)(t6)
    sd     t3, REGOFF(15)(t6)
    sd     t2, REGOFF(14)(t6)
    sd     t1, REGOFF(13)(t6)
    sd     t0, REGOFF(12)(t6)
    sd     a7, REGOFF(11)(t6)
    sd     a6, REGOFF(10)(t6)
    sd     a5, REGOFF(9)(t6)
    sd     a4, REGOFF(8)(t6)
    sd     a3, REGOFF(7)(t6)
    sd     a2, REGOFF(6)(t6)
    sd     a1, REGOFF(5)(t6)
    sd     a0, REGOFF(4)(t6)
    sd     ra, REGOFF(3)(t6)
    csrr   t0, RISCV_CSR_XSTATUS
    sd     t0, REGOFF(1)(sp)
    csrr   a0, RISCV_CSR_XCAUSE
    csrr   a1, RISCV_CSR_XEPC
    sd     a1, REGOFF(1)(t6)
    sd     sp, REGOFF(0)(t6)

    mv     a2, t6
    csrr   t6, sscratch
    sd     t6, REGOFF(18)(a2)
// Restore the kernel stack pointer for user-space threads
    ld     sp, REGOFF(19)(a2)
    bnez   sp, continue
kernel_thread:
    ld     sp, REGOFF(0)(a2)
continue:
    csrw   sscratch, a2

    jal    riscv64_exception_handler

    /* put everything back */
    ld     t0, REGOFF(0)(sp)
    csrw   RISCV_CSR_XEPC, t0
    ld     t0, REGOFF(1)(sp)
    csrw   RISCV_CSR_XSTATUS, t0

    ld     ra, REGOFF(2)(sp)
    ld     a0, REGOFF(3)(sp)
    ld     a1, REGOFF(4)(sp)
    ld     a2, REGOFF(5)(sp)
    ld     a3, REGOFF(6)(sp)
    ld     a4, REGOFF(7)(sp)
    ld     a5, REGOFF(8)(sp)
    ld     a6, REGOFF(9)(sp)
    ld     a7, REGOFF(10)(sp)
    ld     t0, REGOFF(11)(sp)
    ld     t1, REGOFF(12)(sp)
    ld     t2, REGOFF(13)(sp)
    ld     t3, REGOFF(14)(sp)
    ld     t4, REGOFF(15)(sp)
    ld     t5, REGOFF(16)(sp)
    ld     t6, REGOFF(17)(sp)
    addi   sp, sp, REGOFF(20)

    RISCV_XRET
END_FUNCTION(riscv64_exception_entry)
