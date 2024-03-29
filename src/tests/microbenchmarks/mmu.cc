// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <lib/zx/vmar.h>
#include <lib/zx/vmo.h>
#include <limits.h>
#include <zircon/assert.h>
#include <zircon/syscalls.h>
#include <zircon/types.h>

#include <algorithm>

#include <fbl/algorithm.h>
#include <perftest/perftest.h>

#include "assert.h"

namespace {

#define KB(n) ((n)*1024ull)
#define MB(n) (KB(n) * 1024ull)
#define GB(n) (MB(n) * 1024ull)

constexpr size_t kSize = MB(1);

struct Helper {
  Helper() {
    ASSERT_OK(zx::vmar::root_self()->allocate2(ZX_VM_CAN_MAP_READ | ZX_VM_CAN_MAP_SPECIFIC, 0,
                                               GB(1), &vmar, &vmar_base));

    ASSERT_OK(zx::vmo::create(MB(4), 0, &vmo));
  }

  ~Helper() { vmar.destroy(); }

  // Cyclically maps the first chunk_size bytes of |vmo| into the |length| bytes
  // of vmar, starting from offset 0.   Mapping is done |chunk_size| bytes at a
  // time.  |chunk_size| and |length| must be multiples of PAGE_SIZE. As a
  // precondition, |vmar| should be empty.
  zx_status_t MapInChunks(size_t chunk_size, size_t length, bool force_into_mmu);

  zx::vmar vmar;
  zx::vmo vmo;
  uintptr_t vmar_base;
};

zx_status_t Helper::MapInChunks(size_t chunk_size, size_t length, bool force_into_mmu) {
  zx_status_t status;
  zx_vm_option_t flags = ZX_VM_SPECIFIC | ZX_VM_PERM_READ;
  if (force_into_mmu) {
    flags |= ZX_VM_MAP_RANGE;
  }

  for (size_t offset = 0; offset < length; offset += chunk_size) {
    uintptr_t addr;
    size_t len = std::min(chunk_size, length - offset);
    status = vmar.map(flags, offset, vmo, 0, len, &addr);
    if (status != ZX_OK) {
      return status;
    }
  }
  return ZX_OK;
}

// This attempts to measure the amount of time it takes to add and remove
// mappings through the kernel VM layer and the arch MMU layer.
bool MmuMapUnmapTest(perftest::RepeatState* state) {
  state->DeclareStep("map");
  state->DeclareStep("unmap");

  Helper helper;
  while (state->KeepRunning()) {
    // Map just under a large page at a time, to force small pages.  We map many
    // pages at once still, to exercise any optimizations the kernel may perform
    // for small contiguous mappings.
    ASSERT_OK(helper.MapInChunks(511 * KB(4), kSize,
                                 /* force_into_mmu */ true));

    state->NextStep();
    ASSERT_OK(helper.vmar.unmap(helper.vmar_base, kSize));
  }
  return true;
}

// This attempts to measure the amount of time it takes to add mappings in
// the kernel VM layer, page fault the mappings into the arch MMU layer, and
// then remove the mappings from both.
bool MmuMapUnmapWithFaultsTest(perftest::RepeatState* state) {
  state->DeclareStep("map");
  state->DeclareStep("fault_in");
  state->DeclareStep("unmap");

  Helper helper;
  while (state->KeepRunning()) {
    // Map just under a large page at a time, to force small pages.  We map many
    // pages at once still, to exercise any optimizations the kernel may perform
    // for small contiguous mappings.
    ASSERT_OK(helper.MapInChunks(511 * KB(4), kSize,
                                 /* force_into_mmu */ false));

    state->NextStep();
    // Read fault everything in
    auto p = reinterpret_cast<volatile uint8_t*>(helper.vmar_base);
    for (size_t offset = 0; offset < kSize; offset += PAGE_SIZE) {
      p[offset];
    }

    state->NextStep();
    ASSERT_OK(helper.vmar.unmap(helper.vmar_base, kSize));
  }
  return true;
}

void RegisterTests() {
  perftest::RegisterTest("Mmu/MapUnmap", MmuMapUnmapTest);
  perftest::RegisterTest("Mmu/MapUnmapWithFaults", MmuMapUnmapWithFaultsTest);
}
PERFTEST_CTOR(RegisterTests);

}  // namespace
