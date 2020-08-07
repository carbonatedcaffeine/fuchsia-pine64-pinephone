// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fenv.h>
#include <stdint.h>
#include <zircon/compiler.h>

int fegetround(void) {
    return 0;
}

__LOCAL int __fesetround(int round) {
  return 0;
}

int feclearexcept(int mask) {
  return 0;
}

int feraiseexcept(int mask) {
  return 0;
}

int fetestexcept(int mask) {
    return 0;
}

int fegetenv(fenv_t* env) {
  return 0;
}

int fesetenv(const fenv_t* env) {
  return 0;
}
