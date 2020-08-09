// Copyright 2020 The Fuchsia Authors
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#include <arch/ops.h>
#include <arch/spinlock.h>
#include <kernel/atomic.h>

// We need to disable thread safety analysis in this file, since we're
// implementing the locks themselves.  Without this, the header-level
// annotations cause Clang to detect violations.

void arch_spin_lock(arch_spin_lock_t* lock) TA_NO_THREAD_SAFETY_ANALYSIS {
  while (1) {
    if (lock->value) continue;
    int tmp = 1, busy;

    __asm__ __volatile__(
      "   amoswap.w %0, %2, %1\n"
      "   fence r , rw\n"
      : "=r"(busy), "+A"(lock->value)
      : "r" (tmp)
      : "memory"
    );
    if (!busy) break;
  }
  WRITE_PERCPU_FIELD32(num_spinlocks, READ_PERCPU_FIELD32(num_spinlocks) + 1);
}

int arch_spin_trylock(arch_spin_lock_t* lock) TA_NO_THREAD_SAFETY_ANALYSIS {
  int tmp = 1, busy;

  __asm__ __volatile__(
    "   amoswap.w %0, %2, %1\n"
    "   fence r , rw\n"
    : "=r"(busy), "+A"(lock->value)
    : "r" (tmp)
    : "memory"
  );

  if (busy) {
    WRITE_PERCPU_FIELD32(num_spinlocks, READ_PERCPU_FIELD32(num_spinlocks) + 1);
  }

  return !busy;
}

void arch_spin_unlock(arch_spin_lock_t* lock) TA_NO_THREAD_SAFETY_ANALYSIS {
  WRITE_PERCPU_FIELD32(num_spinlocks, READ_PERCPU_FIELD32(num_spinlocks) - 1);
  lock->value = 0;
}
