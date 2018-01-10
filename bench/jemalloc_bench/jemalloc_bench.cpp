#include <stdint.h>
#include <stdio.h>

#include "benchmark/benchmark.h"
#include <algorithm>
#include <iostream>
#include <thread>

#include <stdlib.h>
#define JEMALLOC_NO_DEMANGLE
#include "jemalloc/jemalloc.h"
//#define malloc je_malloc
//#define free je_free


namespace {
constexpr size_t kstacksz = 1 << 16;
void *randomize_buffer[13 << 20];
constexpr int kThreadsMin = 1;
constexpr int KThreadsMax = 256;

void CustomArguments(benchmark::internal::Benchmark *b) {
  for (int i = 8; i <= 1024; i <<= 1) {
    b->Args({i});
  }
}

void CustomArgumentsSmall(benchmark::internal::Benchmark *b) {
  for (int i = 8; i <= 512; i <<= 1) {
    b->Args({i});
  }
}
}

void BM_fastpath_throughput(benchmark::State &state) {
  size_t sz = 32;
  while (state.KeepRunning()) {
    void *p = malloc(sz);
    if (!p) {
      abort();
    }
    free(p);
    // this makes next iteration use different free list. So
    // subsequent iterations may actually overlap in time.
    sz = ((sz * 8191) & 511) + 16;
  }
}

void BM_fastpath_dependent(benchmark::State &state) {
  size_t sz = 32;
  while (state.KeepRunning()) {
    void *p = malloc(sz);
    if (!p) {
      abort();
    }
    free(p);
    // this makes next iteration depend on current iteration. But this
    // iteration's free may still overlap with next iteration's malloc
    sz = ((sz | reinterpret_cast<size_t>(p)) & 511) + 16;
  }
}

void BM_fastpath_simple(benchmark::State &state) {
  size_t sz = state.range(0);

  while (state.KeepRunning()) {
    void *p = malloc(sz);
    if (!p) {
      abort();
    }
    free(p);
    // next iteration will use same free list as this iteration. So it
    // should be prevent next iterations malloc to go too far before
    // free done. But using same size will make free "too fast" since
    // we'll hit size class cache.
  }
}

void BM_fastpath_stack(benchmark::State &state) {
  void *stack[kstacksz];
  size_t sz = 64;

  long param = static_cast<long>(state.range(0));
  param &= kstacksz - 1;
  param = param ? param : 1;

  while (state.KeepRunning()) {
    for (long k = param - 1; k >= 0; k--) {
      void *p = malloc(sz);
      if (!p) {
        abort();
      }
      stack[k] = p;
      // this makes next iteration depend on result of this iteration
      sz = ((sz | reinterpret_cast<size_t>(p)) & 511) + 16;
    }
    for (long k = 0; k < param; k++) {
      free(stack[k]);
    }
  }
  // state.SetItemsProcessed(state.iterations() * state.range(0));
}

void BM_fastpath_stack_above64k(benchmark::State &state) {
  void *stack[kstacksz];
  size_t sz = 64;

  long param = static_cast<long>(state.range(0));
  param &= kstacksz - 1;
  param = param ? param : 1;

  while (state.KeepRunning()) {
    for (long k = param - 1; k >= 0; k--) {
      void *p = malloc(sz);
      if (!p) {
        abort();
      }
      stack[k] = p;
      // this makes next iteration depend on result of this iteration
      sz = (2 << 16) + ((sz | reinterpret_cast<size_t>(p)) & 511) + 16;
    }
    for (long k = 0; k < param; k++) {
      free(stack[k]);
    }
  }
  // state.SetItemsProcessed(state.iterations() * state.range(0));
}

void BM_fastpath_stack_simple(benchmark::State &state) {
  void *stack[kstacksz];
  size_t sz = 128;

  long param = static_cast<long>(state.range(0));
  param &= kstacksz - 1;
  param = param ? param : 1;

  while (state.KeepRunning()) {
    for (long k = param - 1; k >= 0; k--) {
      void *p = malloc(sz);
      if (!p) {
        abort();
      }
      stack[k] = p;
    }
    for (long k = 0; k < param; k++) {
      free(stack[k]);
    }
  }
  // state.SetItemsProcessed(state.iterations() * state.range(0));
}

void BM_fastpath_rnd_dependent(benchmark::State &state) {
  static const uintptr_t rnd_c = 1013904223;
  static const uintptr_t rnd_a = 1664525;

  void *ptrs[kstacksz];
  size_t sz = 128;
  size_t _param = state.range(0);
  if ((_param & (_param - 1))) {
    abort();
  }
  if (_param > kstacksz) {
    abort();
  }
  int param = static_cast<int>(_param);

  while (state.KeepRunning()) {
    for (int k = param - 1; k >= 0; k--) {
      void *p = malloc(sz);
      if (!p) {
        abort();
      }
      ptrs[k] = p;
      sz = ((sz | reinterpret_cast<size_t>(p)) & 511) + 16;
    }

    // this will iterate through all objects in order that is
    // unpredictable to processor's prefetchers
    uint32_t rnd = 0;
    uint32_t free_idx = 0;
    do {
      free(ptrs[free_idx]);
      rnd = rnd * rnd_a + rnd_c;
      free_idx = rnd & (param - 1);
    } while (free_idx != 0);
  }
  // state.SetItemsProcessed(state.iterations() * state.range(0));
}

// These two functions seems to be warm up code?
void randomize_one_size_class(size_t size) {
  size_t count = (100 << 20) / size;
  if (count * sizeof(randomize_buffer[0]) > sizeof(randomize_buffer)) {
    abort();
  }
  for (size_t i = 0; i < count; i++) {
    randomize_buffer[i] = malloc(size);
  }
  std::random_shuffle(randomize_buffer, randomize_buffer + count);
  for (size_t i = 0; i < count; i++) {
    free(randomize_buffer[i]);
  }
}

void randomize_size_classes() {
  randomize_one_size_class(8);
  int i;
  for (i = 16; i < 256; i += 16) {
    randomize_one_size_class(i);
  }
  for (; i < 512; i += 32) {
    randomize_one_size_class(i);
  }
  for (; i < 1024; i += 64) {
    randomize_one_size_class(i);
  }
  for (; i < (4 << 10); i += 128) {
    randomize_one_size_class(i);
  }
  for (; i < (32 << 10); i += 1024) {
    randomize_one_size_class(i);
  }
}

BENCHMARK(BM_fastpath_throughput)->ThreadRange(kThreadsMin, KThreadsMax);
BENCHMARK(BM_fastpath_dependent)->ThreadRange(kThreadsMin, KThreadsMax);
BENCHMARK(BM_fastpath_simple)
    ->Args({2 << 6})
    ->Args({2 << 11})
    ->Args({2 << 14})
    ->Args({2 << 17})
    ->ThreadRange(kThreadsMin, KThreadsMax);
BENCHMARK(BM_fastpath_stack)
    ->Apply(CustomArguments)
    ->ThreadRange(kThreadsMin, KThreadsMax);
BENCHMARK(BM_fastpath_stack_above64k)
    ->Apply(CustomArgumentsSmall)
    ->ThreadRange(kThreadsMin, KThreadsMax);
BENCHMARK(BM_fastpath_stack_simple)
    ->Args({2 << 5})
    ->Args({2 << 10})
    ->Args({2 << 15})
    ->ThreadRange(kThreadsMin, KThreadsMax);
BENCHMARK(BM_fastpath_rnd_dependent)
    ->Args({2 << 5})
    ->Args({2 << 10})
    ->Args({2 << 15})
    ->ThreadRange(kThreadsMin, KThreadsMax);

int main(int argc, char **argv) {
  ::benchmark::Initialize(&argc, argv);
  randomize_size_classes();

  if (::benchmark::ReportUnrecognizedArguments(argc, argv))
    return 1;
  ::benchmark::RunSpecifiedBenchmarks();
}
