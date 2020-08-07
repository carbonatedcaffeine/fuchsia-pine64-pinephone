// Copyright 2020 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#ifndef ZIRCON_KERNEL_ARCH_RISCV64_INCLUDE_ARCH_SPINLOCK_H_
#define ZIRCON_KERNEL_ARCH_RISCV64_INCLUDE_ARCH_SPINLOCK_H_

#include <lib/zircon-internal/thread_annotations.h>
#include <stdbool.h>
#include <sys/types.h>
#include <zircon/compiler.h>

#include <arch/riscv64/interrupt.h>
#include <arch/riscv64/mp.h>

__BEGIN_CDECLS

#define ARCH_SPIN_LOCK_INITIAL_VALUE \
  (arch_spin_lock_t) { 0 }

typedef struct TA_CAP("mutex") spin_lock {
  unsigned long value;
} arch_spin_lock_t;

typedef unsigned int spin_lock_save_flags_t;

void arch_spin_lock(arch_spin_lock_t* lock) TA_ACQ(lock);
int arch_spin_trylock(arch_spin_lock_t* lock) TA_TRY_ACQ(false, lock);
void arch_spin_unlock(arch_spin_lock_t* lock) TA_REL(lock);

static inline uint arch_spin_lock_holder_cpu(const arch_spin_lock_t* lock) {
  return 0;
}

static inline bool arch_spin_lock_held(arch_spin_lock_t* lock) {
  return *lock != 0;
}

enum {
  /* Possible future flags:
   * SPIN_LOCK_FLAG_PMR_MASK         = 0x000000ff,
   * SPIN_LOCK_FLAG_PREEMPTION       = 0x10000000,
   * SPIN_LOCK_FLAG_SET_PMR          = 0x20000000,
   */

  /* ARM specific flags */
  SPIN_LOCK_FLAG_IRQ = 0x40000000,
  SPIN_LOCK_FLAG_FIQ = 0x80000000, /* Do not use unless IRQs are already disabled */
  SPIN_LOCK_FLAG_IRQ_FIQ = SPIN_LOCK_FLAG_IRQ | SPIN_LOCK_FLAG_FIQ,

  /* default arm flag is to just disable plain irqs */
  ARCH_DEFAULT_SPIN_LOCK_FLAG_INTERRUPTS = SPIN_LOCK_FLAG_IRQ
};

enum {
  /* private */
  SPIN_LOCK_STATE_RESTORE_IRQ = 1,
  SPIN_LOCK_STATE_RESTORE_FIQ = 2,
};

__END_CDECLS

#endif  // ZIRCON_KERNEL_ARCH_RISCV64_INCLUDE_ARCH_SPINLOCK_H_
