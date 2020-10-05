// Copyright 2020 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#include <asm.h>
#include <err.h>
#include <arch/riscv64.h>
#include <lib/syscalls/zx-syscall-numbers.h>

#define REGOFF(x) ((x) * 8)

// void riscv64_context_switch(vaddr_t* old_sp, vaddr_t new_sp);
FUNCTION(riscv64_context_switch)
    sd    ra, REGOFF(0)(a0)
    sd    sp, REGOFF(1)(a0)
    sd    s0, REGOFF(2)(a0)
    sd    s1, REGOFF(3)(a0)
    sd    s2, REGOFF(4)(a0)
    sd    s3, REGOFF(5)(a0)
    sd    s4, REGOFF(6)(a0)
    sd    s5, REGOFF(7)(a0)
    sd    s6, REGOFF(8)(a0)
    sd    s7, REGOFF(9)(a0)
    sd    s8, REGOFF(10)(a0)
    sd    s9, REGOFF(11)(a0)
    sd    s10, REGOFF(12)(a0)
    sd    s11, REGOFF(13)(a0)

    ld    s11, REGOFF(13)(a1)
    ld    s10, REGOFF(12)(a1)
    ld    s9, REGOFF(11)(a1)
    ld    s8, REGOFF(10)(a1)
    ld    s7, REGOFF(9)(a1)
    ld    s6, REGOFF(8)(a1)
    ld    s5, REGOFF(7)(a1)
    ld    s4, REGOFF(6)(a1)
    ld    s3, REGOFF(5)(a1)
    ld    s2, REGOFF(4)(a1)
    ld    s1, REGOFF(3)(a1)
    ld    s0, REGOFF(2)(a1)
    ld    sp, REGOFF(1)(a1)
    ld    ra, REGOFF(0)(a1)

    ret
END_FUNCTION(riscv_context_switch)

FUNCTION(riscv64_get_current_thread)
    mv a0, tp
    ret
END_FUNCTION(riscv64_get_current_thread)

FUNCTION(riscv64_set_current_thread)
    mv tp, a0
    ret
END_FUNCTION(riscv64_set_current_thread)

// void riscv64_uspace_entry(iframe_t* iframe, vaddr_t kstack) __NO_RETURN;
FUNCTION(riscv64_uspace_entry)
    mv  sp, a1
    mv  t6, a0

    ld     sp, REGOFF(0)(t6)
    ld     t0, REGOFF(1)(t6)
    csrw   RISCV_CSR_XEPC, t0
    ld     t0, REGOFF(2)(t6)
    csrw   RISCV_CSR_XSTATUS, t0

    ld     ra, REGOFF(3)(t6)
    ld     a0, REGOFF(4)(t6)
    ld     a1, REGOFF(5)(t6)
    ld     a2, REGOFF(6)(t6)
    ld     a3, REGOFF(7)(t6)
    ld     a4, REGOFF(8)(t6)
    ld     a5, REGOFF(9)(t6)
    ld     a6, REGOFF(10)(t6)
    ld     a7, REGOFF(11)(t6)
    ld     t0, REGOFF(12)(t6)
    ld     t1, REGOFF(13)(t6)
    ld     t2, REGOFF(14)(t6)
    ld     t3, REGOFF(15)(t6)
    ld     t4, REGOFF(16)(t6)
    ld     t5, REGOFF(17)(t6)
    ld     t6, REGOFF(18)(t6)

    RISCV_XRET
END_FUNCTION(riscv64_uspace_entry)

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
    sd     t0, REGOFF(2)(t6)
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
    csrr   t6, sscratch
    ld     sp, REGOFF(0)(t6)
    ld     t0, REGOFF(1)(t6)
    csrw   RISCV_CSR_XEPC, t0
    ld     t0, REGOFF(2)(t6)
    csrw   RISCV_CSR_XSTATUS, t0

    ld     ra, REGOFF(3)(t6)
    ld     a0, REGOFF(4)(t6)
    ld     a1, REGOFF(5)(t6)
    ld     a2, REGOFF(6)(t6)
    ld     a3, REGOFF(7)(t6)
    ld     a4, REGOFF(8)(t6)
    ld     a5, REGOFF(9)(t6)
    ld     a6, REGOFF(10)(t6)
    ld     a7, REGOFF(11)(t6)
    ld     t0, REGOFF(12)(t6)
    ld     t1, REGOFF(13)(t6)
    ld     t2, REGOFF(14)(t6)
    ld     t3, REGOFF(15)(t6)
    ld     t4, REGOFF(16)(t6)
    ld     t5, REGOFF(17)(t6)
    ld     t6, REGOFF(18)(t6)

    RISCV_XRET
END_FUNCTION(riscv64_exception_entry)

//
// Syscall args are in a0-a7 already.
// pc is in t1 and needs to go in the next available register,
// or the stack if the regs are full.
//
.macro syscall_dispatcher nargs, syscall
.balign 16
.if \nargs == 8
    addi sp, sp, -8
    sd   t1, (sp)
    jal  wrapper_\syscall
    addi sp, sp, 8
.else
    mv a\nargs, t1
    jal  wrapper_\syscall
.endif
    j .Lpost_syscall
.endm

// riscv64_syscall_dispatcher receives an iframe pointer. Registers are parsed
// using the following convention:
//
// a0-a7 - contains syscall arguments
// t0    - contains syscall_num
//
FUNCTION(riscv64_syscall_dispatcher)
    addi sp, sp, -8
    sd   ra, (sp)

    ld     t1, REGOFF(1)(a0)
    ld     t0, REGOFF(12)(a0)
    ld     a7, REGOFF(11)(a0)
    ld     a6, REGOFF(10)(a0)
    ld     a5, REGOFF(9)(a0)
    ld     a4, REGOFF(8)(a0)
    ld     a3, REGOFF(7)(a0)
    ld     a2, REGOFF(6)(a0)
    ld     a1, REGOFF(5)(a0)
    ld     a0, REGOFF(4)(a0)

    // Verify syscall number and call the unknown handler if bad.
    li   t2, ZX_SYS_COUNT
    bge  t0, t2, .Lunknown_syscall

    // Jump to the right syscall wrapper. The syscall table is an
    // array of 16 byte aligned routines for each syscall. Each routine
    // marshalls some arguments, jumps to the routine, and then branches
    // back to .Lpost_syscall (see syscall_dispatcher macro above).
    slli t0, t0, 4
    lla  t2, .Lsyscall_table
    add  t2, t2, t0
    jr   t2

.Lunknown_syscall:
    mv a0, t0 // move the syscall number into the 0 arg slot
    mv a1, t1 // pc into arg 1
    jal  unknown_syscall
    // fall through

// Adds the label for the jump table.
.balign 16
.Lsyscall_table:

// One of these macros is invoked by kernel.inc for each syscall.
// These are the direct kernel entry points.
#define KERNEL_SYSCALL(name, type, attrs, nargs, arglist, prototype) \
  syscall_dispatcher nargs, name
#define INTERNAL_SYSCALL(...) KERNEL_SYSCALL(__VA_ARGS__)
#define BLOCKING_SYSCALL(...) KERNEL_SYSCALL(__VA_ARGS__)
// These don't have kernel entry points.
#define VDSO_SYSCALL(...)

#include <lib/syscalls/kernel.inc>

#undef KERNEL_SYSCALL
#undef INTERNAL_SYSCALL
#undef BLOCKING_SYSCALL
#undef VDSO_SYSCALL

.Lpost_syscall:
    ld   ra, (sp)
    addi sp, sp, 8
    ret

END_FUNCTION(riscv64_syscall_dispatcher)

FUNCTION(mexec_asm)
    ret
END_FUNCTION(mexec_asm)

DATA(mexec_asm_end)

// Riscv64UserCopyRet _riscv64_user_copy(void *dst, const void *src, size_t len, uint64_t *fault_return)
.balign 64 // Align to cache line.  This code fits in one cache line.
FUNCTION(_riscv64_user_copy)
    addi sp, sp, -24
    sd s1, 16(sp)
    sd s2, 8(sp)
    sd s3, (sp)

    // Allow supervisor accesses to user memory
    li s1, (1 << 18)
    csrs sstatus, s1

    // Save fault_return and the ra register
    mv s2, a3
    mv s3, ra

    // Set *fault_return to fault_from_user
    la t0, .Lfault_from_user
    sd t0, (a3)

    // Just call our normal memcpy.  The caller has ensured that the
    // address range is in the user portion of the address space.
    // While fault_return_ptr is set, userspace data faults will be
    // redirected to .Lfault_from_user, below.
    //
    // NOTE! We make important assumptions here about what the memcpy
    // code does: it never moves the stack pointer, and it never touches a6 and
    // a7 where we saved fault_return and ra
    jal memcpy

    // Store a successful status for the return. In this case since we do not set x1 the value of
    // the fault address in the return struct is undefined.
    li a0, ZX_OK

.Luser_copy_return:
    // Restore *fault_return and the ra register
    sd x0, (s2)
    mv ra, s3

    // Disable supervisor accesses to user memory
    csrc sstatus, s1

    ld s1, 16(sp)
    ld s2, 8(sp)
    ld s3, (sp)
    addi sp, sp, 24
    ret
END_FUNCTION(_riscv64_user_copy)

// If we are capturing faults the exception handler will have placed the faulting virtual address
// for us in a1 and the flags in a2. We do not touch a1 and rely on the caller to know if the value
// is meaningful based on whether it specified fault capture or not, we just need to construct a
// valid a0 before jmping to user_copy_return.
.Lfault_from_user:
    li a0, ZX_ERR_INVALID_ARGS
    // If we are capturing faults the flags will have been placed in a2 and we want them placed in
    // the high bits of a0. If not capturing faults then we will copy some garbage bits which will
    // be ignored by the caller.
    //bfi a0, a2, 32, 32
    j .Luser_copy_return

