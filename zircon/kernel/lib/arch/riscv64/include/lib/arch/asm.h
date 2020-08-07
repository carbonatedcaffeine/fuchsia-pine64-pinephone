// Copyright 2020 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#ifndef ZIRCON_KERNEL_LIB_ARCH_RISCV64_INCLUDE_LIB_ARCH_ASM_H_
#define ZIRCON_KERNEL_LIB_ARCH_RISCV64_INCLUDE_LIB_ARCH_ASM_H_

// Get the generic file.
#include_next <lib/arch/asm.h>

#ifdef __ASSEMBLER__  // clang-format off

/// Fill a register with a wide integer literal.
///
/// This emits the one to four instructions required to fill a 64-bit
/// register with a given bit pattern.  It uses as few instructions as
/// suffice for the particular value.
///
/// Parameters
///
///   * reg
///     - Required: Output 64-bit register.
///
///   * literal
///     - Required: An integer expression that can be evaluated immediately
///     without relocation.
///
.macro movlit reg, literal
.endm  // movlit

/// Materialize a symbol (with optional addend) into a register.
///
/// This emits the `adr` instruction or two-instruction sequence required
/// to materialize the address of a global variable or function symbol.
///
/// Parameters
///
///   * reg
///     - Required: Output 64-bit register.
///
///   * symbol
///     - Required: A symbolic expression requiring at most PC-relative reloc.
///
.macro adr_global reg, symbol
.endm  // adr_global

/// Load a 64-bit fixed global symbol (with optional addend) into a register.
///
/// This emits the `ldr` instruction or two-instruction sequence required to
/// load a global variable.  If multiple words are required, it's more
/// efficient to use `adr_global` and then `ldp` than to repeat `ldr_global`
/// with related locations.
///
/// Parameters
///
///   * reg
///     - Required: Output 64-bit register.
///
///   * symbol
///     - Required: A symbolic expression requiring at most PC-relative reloc.
///
.macro ldr_global reg, symbol
.endm  // adr_global

#endif  // clang-format on

#endif  // ZIRCON_KERNEL_LIB_ARCH_RISCV64_INCLUDE_LIB_ARCH_ASM_H_

