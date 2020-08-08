/* Copyright 2018 The Fuchsia Authors
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */

ENTRY(_start)
SECTIONS
{
    . = 0x80200000;

    .text.boot0 : {
        *(.text.boot0)
    } :load
    .text : {
        *(.text*)
        *(.gnu.linkonce.t.*)
    }

    .rodata : {
        *(.rodata* .gnu.linkonce.r.*)
    }

    .data : {
        *(.data .data.* .gnu.linkonce.d.*)
    }

    .bss : {
        *(.bss*) *(.sbss*) *(COMMON)
    }
}

PHDRS
{
    load PT_LOAD FLAGS(7); /* PF_R|PF_W|PF_X */
}
