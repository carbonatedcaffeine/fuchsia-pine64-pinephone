// Copyright 2020 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#include <stdint.h>

#include "debug.h"

#define UART_URTX0 (0x40)
#define UART_UTS (0xb4)
#define UART_UTS_TXFULL (1 << 4)

#define UARTREG(reg) (*(volatile uint32_t*)(0x30860000 + (reg)))

void uart_pputc(char c) {
  while (UARTREG(UART_UTS) & UART_UTS_TXFULL)
    ;
  UARTREG(UART_URTX0) = c;
}
