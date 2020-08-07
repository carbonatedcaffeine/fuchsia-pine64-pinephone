// Copyright 2020 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#include <lib/arch/asm.h>
#include <lib/arch/ticks.h>
#include <zircon/tls.h>

#include "zircon/kernel/phys/stack.h"

// This is the entry point from the boot loader or thereabouts.
// It receives one argument, in x0, usually a pointer (physical address).
// The caches and MMU are disabled.
//
// In a ZBI executable, this is where zbi_kernel_t::entry points and
// x0 holds the address of the data ZBI.
//
// In a boot shim, the header code jumps here after normalizing machine state
// and x0 holds what's usually a Device Tree address.
.function _start, global
.end_function

.object boot_thread_area, bss, local, align=8
  .skip (ZX_TLS_STACK_GUARD_OFFSET)
#if ZX_TLS_UNSAFE_SP_OFFSET < ZX_TLS_STACK_GUARD_OFFSET
  .error "TLS ABI layout??"
#endif
boot_thread_pointer:
.end_object

