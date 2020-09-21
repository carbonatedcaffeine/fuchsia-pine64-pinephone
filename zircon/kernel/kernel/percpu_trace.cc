// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <lib/affine/ratio.h>
#include <lib/console.h>
#include <lib/debuglog.h>
#include <stdio.h>
#include <zircon/errors.h>

#include <kernel/auto_preempt_disabler.h>
#include <kernel/thread.h>
#include <ktl/atomic.h>
#include <ktl/optional.h>
#include <ktl/string_view.h>

extern cpu_num_t arch_curr_cpu_num_slow();

namespace {

enum class UseAtomics { No, Yes };

constexpr UseAtomics kUseAtomics = UseAtomics::No;
constexpr size_t MAX_CPUS = 6;
static constexpr uint32_t TRACE_ENTRIES = 1024;

template <typename T, UseAtomics choice>
class MaybeAtomic;

template <typename T>
class MaybeAtomic<T, UseAtomics::No> {
 public:
  explicit MaybeAtomic(T val) : val_(val) {}
  void store(T val) { val_ = val; }
  T load() { return val_; }
  T fetch_add(T val) {
    T ret = val_;
    val_ += val;
    return ret;
  }

 private:
  T val_;
};

template <typename T>
class MaybeAtomic<T, UseAtomics::Yes> {
 public:
  explicit MaybeAtomic(T val) : val_(val) {}
  void store(T val) { val_.store(val); }
  T load() { return val_.load(); }
  T fetch_add(T val) { return val_.fetch_add(val); }

 private:
  ktl::atomic<T> val_;
};

class Trace {
 public:
  void Log(const char* fname, const char* func, int line, const char* tag, uint32_t a, uint32_t b,
           uint32_t c, uint32_t d, void* ptr1, void* ptr2) {
    if (!enabled_.load()) {
      return;
    }

    uint32_t wr = wr_.fetch_add(1) % TRACE_ENTRIES;
    auto& e = entries_[wr];
    e.ts_ticks = current_ticks();
    e.thread = Thread::Current::Get();
    e.fname = fname;
    e.func = func;
    e.line = line;
    e.tag = tag;
    e.a = a;
    e.b = b;
    e.c = c;
    e.d = d;
    e.ptr1 = reinterpret_cast<uint64_t>(ptr1);
    e.ptr2 = reinterpret_cast<uint64_t>(ptr2);
  }

  void Dump(cpu_num_t cpu_id, bool reset_flag, bool disable_flag) {
    // Disable the log while we dump it
    enabled_.store(false);
    if constexpr (kUseAtomics == UseAtomics::No) {
      ktl::atomic_thread_fence(ktl::memory_order_seq_cst);
    }
    Thread::Current::SleepRelative(ZX_MSEC(10));

    size_t wr = wr_.load();
    uint32_t start = (wr >= TRACE_ENTRIES) ? (wr % TRACE_ENTRIES) : 0;
    uint32_t amt = (wr >= TRACE_ENTRIES) ? TRACE_ENTRIES : static_cast<uint32_t>(wr);

    printf("Dumping %u/%u entries from trace log for CPU %u\n", amt, TRACE_ENTRIES, cpu_id);
    affine::Ratio ticks_to_mono_ratio = platform_get_ticks_to_time_ratio();

    zx_time_t prev = ticks_to_mono_ratio.Scale(entries_[start].ts_ticks);

    for (uint32_t i = 0; i < amt; ++i) {
      const auto& e = entries_[(start + i) % TRACE_ENTRIES];
      zx_time_t now = ticks_to_mono_ratio.Scale(e.ts_ticks);

      PrintTraceEntry(e, now, prev, cpu_id);
      prev = now;
    }

    // If the reset flag was passed, reset the log.
    if (reset_flag) {
      wr_.store(0);
      memset(entries_, 0, sizeof(entries_));
    }

    // If the disable flag was not passed, re-enable the log
    if (!disable_flag) {
      enabled_.store(true);
    }
  }

  void Reset() {
    enabled_.store(false);
    Thread::Current::SleepRelative(ZX_MSEC(10));
    memset(entries_, 0, sizeof(entries_));
    wr_.store(0);
    enabled_.store(true);
  }

 private:
  struct TraceEntry {
    const char* fname = nullptr;
    const char* func = nullptr;
    const char* tag = nullptr;
    uint64_t ptr1, ptr2;
    zx_ticks_t ts_ticks;
    Thread* thread;
    int line;
    uint32_t a, b, c, d;
  };

  static void PrintTraceEntry(const TraceEntry& e, zx_time_t now_time, zx_time_t prev_time,
                              cpu_num_t cpu_id) {
    int64_t delta_usec = (now_time - prev_time) / 1000;
    int64_t delta_nsec = (now_time - prev_time) % 1000;
    char line_buf[1024];
    size_t amt = snprintf(
        line_buf, sizeof(line_buf),
        "[%15lu][%u][%p] : %08x %08x %08x %08x %016lx %016lx : (%8lu.%03lu uSec) : %s:%s:%d (%s)\n",
        now_time, cpu_id, e.thread, e.a, e.b, e.c, e.d, e.ptr1, e.ptr2, delta_usec, delta_nsec,
        TrimFilename(e.fname), e.func, e.line, e.tag);
    dlog_serial_write({line_buf, amt});
  }

  static constexpr const char* TrimFilename(ktl::string_view fname) {
    size_t pos = fname.find_last_of('/');
    return ((pos == fname.npos) ? fname : fname.substr(pos)).data();
  }

  TraceEntry entries_[TRACE_ENTRIES];
  MaybeAtomic<size_t, kUseAtomics> wr_{0};
  MaybeAtomic<bool, kUseAtomics> enabled_{true};
};

ktl::array<Trace, MAX_CPUS> g_trace;

int cmd_ptrace(int argc, const cmd_args* argv, uint32_t flags) {
  auto usage = [argv]() {
    printf("usage:\n");
    printf("%s dump [-r] [-d] <cpu_id>\n", argv[0].str);
    printf("%s reset <cpu_id>\n", argv[0].str);
    printf("\n");
    printf("Dump will dump log for the given CPU ID, keeping the log disabled during the dump. \n");
    printf("Afterwards, it will resume logging.  If the -r flag is passed, it will reset the \n");
    printf("log before resuming.  If the -d flag is passed, the log will be left in the \n");
    printf("disabled state instead of resuming.\n");
    printf("\n");
    printf("Reset will unconditionally reset and re-enable the specified log for a CPU.\n");
    return ZX_ERR_INVALID_ARGS;
  };

  enum class Op {
    Dump,
    Reset,
  };

  bool reset_flag = false;
  bool disable_flag = false;
  ktl::optional<cpu_num_t> cpu_num;
  Op op;

  if (argc < 3) {
    printf("Not enough arguments\n\n");
    return usage();
  }

  if (!strcmp(argv[1].str, "dump")) {
    op = Op::Dump;
  } else if (!strcmp(argv[1].str, "reset")) {
    op = Op::Reset;
  } else {
    printf("Unrecognized command \"%s\"\n", argv[1].str);
  }

  for (int i = 2; i < argc; ++i) {
    if (argv[i].str[0] == '-') {
      if (strlen(argv[i].str + 1) != 1) {
        printf("Invalid optional argument \"%s\"\n", argv[i].str);
        return usage();
      }

      switch (argv[i].str[1]) {
        case 'r':
          reset_flag = true;
          break;
        case 'd':
          disable_flag = true;
          break;
        case '?':
        case 'h':
          usage();
          return 0;
        default:
          printf("Invalid optional argument \"%s\"\n", argv[i].str);
          return usage();
      }
    } else {
      if (cpu_num.has_value()) {
        printf("Invalid positional argument \"%s\"\n", argv[i].str);
        return usage();
      }

      cpu_num = argv[i].u;
      if (cpu_num >= MAX_CPUS) {
        printf("Invalid CPU number argument \"%s\"\n", argv[i].str);
        return usage();
      }
    }
  }

  if (!cpu_num.has_value()) {
    printf("CPU num not specified\n");
    return usage();
  }

  DEBUG_ASSERT(cpu_num.value() < MAX_CPUS);
  auto& trace = g_trace[cpu_num.value()];

  switch (op) {
    case Op::Dump:
      trace.Dump(cpu_num.value(), reset_flag, disable_flag);
      break;

    case Op::Reset:
      if (reset_flag || disable_flag) {
        printf("Reset and disable flags are not valid for the \"reset\" command\n");
        return usage();
      }
      trace.Reset();
      break;

    default:
      printf("Internal error\n");
      return usage();
  }

  return 0;
}

inline void percpu_trace_log_common(const char* fname, const char* func, int line, const char* tag, uint32_t a,
                      uint32_t b, uint32_t c, uint32_t d, void* ptr1, void* ptr2) {
  cpu_num_t slow_cpu_id = arch_curr_cpu_num_slow();
  cpu_num_t fast_cpu_id = arch_curr_cpu_num();
  g_trace[slow_cpu_id].Log(fname, func, line, tag, a, b, c, d, ptr1, ptr2);

  if (fast_cpu_id != slow_cpu_id) {
    // We are about to panic.  Dump the trace for this CPU just before we do.
    dlog_force_panic();
    printf("CPU ID Mismatch!  Ground truth %u Fast Read %u\n", slow_cpu_id, fast_cpu_id);
    g_trace[slow_cpu_id].Dump(slow_cpu_id, false, false);
    ASSERT(false);
  }
}

}  // namespace

extern "C" {
void percpu_trace_log_low_level(const char* fname, const char* func, int line, const char* tag, uint32_t a,
                      uint32_t b, uint32_t c, uint32_t d, void* ptr1, void* ptr2) {
  percpu_trace_log_common(fname, func, line, tag, a, b, c, d, ptr1, ptr2);
}

void percpu_trace_log(const char* fname, const char* func, int line, const char* tag, uint32_t a,
                      uint32_t b, uint32_t c, uint32_t d, void* ptr1, void* ptr2) {
  AutoPreemptDisabler<APDInitialState::PREEMPT_DISABLED> ap_disabler;
  percpu_trace_log_common(fname, func, line, tag, a, b, c, d, ptr1, ptr2);
}

}

STATIC_COMMAND_START
STATIC_COMMAND("ptrace", "Percpu tracing control", &cmd_ptrace)
STATIC_COMMAND_END(zx)
