// Copyright 2020 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#include <lib/arch/asm.h>
#include <lib/arch/ticks.h>
#include <lib/arch/x86/msr.h>
#include <zircon/tls.h>

#include "zircon/kernel/phys/stack.h"


// TODO(mcgrathr): Use this data later.
#define CPUID_LEAVES 8
#define CPUID_LEAF(leaf, reg) (cpuid_leaves + (leaf * 4 * 4) + (reg * 4))(%rip)
.macro cpuid_leaf n
.if \n >= CPUID_LEAVES
  .error "CPUID_LEAVES too small"
.elseif \n
  mov $\n, %eax
.else
  xor %eax, %eax
.endif
  cpuid
  mov %eax, CPUID_LEAF(\n, 0)
  mov %ebx, CPUID_LEAF(\n, 1)
  mov %ecx, CPUID_LEAF(\n, 2)
  mov %edx, CPUID_LEAF(\n, 3)
.endm

.object cpuid_leaves, bss, local, align=4
  .skip 4 * 4 * CPUID_LEAVES
.end_object

// This is the entry point from the boot loader or thereabouts.
// It receives one argument, in %rsi, usually a pointer (physical address).
//
// In a ZBI executable, this is where zbi_kernel_t::entry points and
// %rsi holds the address of the data ZBI.
.function _start, global
  // As early as possible collect the time stamp.
  sample_ticks

  // Clear frame pointer: at the root of the call stack.
  xor %ebp, %ebp

  // Save the timestamp since %rax must be clobbered below.
  mov %rax, %rbx

  // Clear bss.  Note this assumes it's aligned to 8, which is ensured
  // by the bss declaration below.
  lea _edata(%rip), %rdi
  lea _end(%rip), %rcx
  sub %rdi, %rcx
  shr $3, %rcx
  xor %eax, %eax
  cld  // Assume nothing.  ABI requires that DF be clear.
  rep stosq

  // Move first argument to PhysMain into place.
  mov %rsi, %rdi

  // The time stamp will be the second argument to PhysMain.
  mov %rbx, %rsi

  // Some CPUID bits are immediately useful.  Save the whole leaves for later.
  cpuid_leaf 0
  cmpl $1, CPUID_LEAF(0, 0)
  jl 0f
  cpuid_leaf 1
  cmpl $7, CPUID_LEAF(0, 0)
  jl 0f
  cpuid_leaf 7
0:

  // Set up the stacks and the thread pointer area.
  lea boot_thread_pointer(%rip), %rax

  testl $(1 << 0), CPUID_LEAF(7, 1)
  jz .Lno_fsgsbase
  wrgsbase %rax
  jmp .Lstack_guard
.Lno_fsgsbase:
  // Use the MSR if wrgsbase is not available.
  wrmsr64 MSR_IA32_GS_BASE

.Lstack_guard:
  // Stack guard canary value.  See what kind of randomness is available.
  testl $(1 << 18), CPUID_LEAF(1, 1)
  jz .Lno_rdseed
  rdseed %rax
  jmp .Lstack_guard_write

.Lno_rdseed:
  testl $(1 << 30), CPUID_LEAF(1, 2)
  jz .Lno_rdrand
  rdrand %rax
  jmp .Lstack_guard_write

.Lno_rdrand:
  // The only "randomness" readily available is our own load address, so
  // swizzle that in with some arbitrary bits.
  lea _start(%rip), %rcx
  movabs $0xdeadbeef1ee2d00d, %rax
  xor %rcx, %rax

.Lstack_guard_write:
  mov %rax, %gs:ZX_TLS_STACK_GUARD_OFFSET

  lea (boot_stack + BOOT_STACK_SIZE)(%rip), %rsp
#if __has_feature(safe_stack)
  lea (boot_unsafe_stack + BOOT_STACK_SIZE)(%rip), %rax
  mov %rax, %gs:ZX_TLS_UNSAFE_SP_OFFSET
#endif

  // Now the full C++ ABI is available.  This could theoretically be a tail
  // call since it's obliged never to return, but it's nice to have the
  // caller in a backtrace (and the call implicitly adjusts the stack
  // alignment as the ABI requires).
  call PhysMain

  // Trap forever just in case it does return.
0:ud2
  jmp 0b
.end_function

.object boot_thread_area, bss, local, align=8
#if ZX_TLS_UNSAFE_SP_OFFSET < ZX_TLS_STACK_GUARD_OFFSET
  .error "TLS ABI layout??"
#endif
  .skip ZX_TLS_UNSAFE_SP_OFFSET + 8
boot_thread_pointer:
.end_object
