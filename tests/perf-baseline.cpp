#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

size_t seed = 123456789;

float square_root(float x) {
  union {
    uint32_t i;
    float x;
  } tmp;
  tmp.x = x;
  tmp.i += 127 << 23;
  tmp.i >>= 1;
  return tmp.x;
}

size_t random_size() {
  seed ^= (seed << 21);
  seed ^= (seed >> 35);
  seed ^= (seed << 4);
  float tmp = (float)(seed >> 32) / 4294967296.0;
  return ((size_t)((1 - square_root(tmp)) * 128) >> 2 << 2) + 8;
}

int main(int argc, char** argv) {
  void* current = malloc(random_size());

  void* head = current;

  void* prev = current;
  int32_t i;
  for (i = 0;  i < 100000;  i++) {
    current = malloc(random_size());
    ((size_t*)prev)[0] = (size_t)current;
    prev = current;
  }

  void* next_head;
  for (i = 0;  i < 200000000;  i++) {
    next_head = (void*)((size_t*)head)[0];
    free(head);
    head = next_head;

    current = malloc(random_size());
    ((size_t*)prev)[0] = (size_t)current;
    prev = current;
  }

  for (i = 0;  i < 100000;  i++) {
    next_head = (void*)((size_t*)head)[0];
    free(head);
    head = next_head;
  }

  // int64_t num = 1000 * 1024;
  // int32_t* arena = (int32_t*)malloc(num * sizeof(int32_t));

  // int32_t i = 0;
  // int64_t j;
  // int64_t where;

  // while (i < 200200000) {
  //   where = 0;
  //   while (where < num) {
  //     arena[where] = i;
  //     for (j = 0;  j < 150  &&  where + j < 200200000;  j++) {
  //       arena[where + j] = i;
  //     }
  //     i++;
  //     where += random_size();
  //   }
  // }

  // free(arena);

  printf("dummy %d\n", i);

  return 0;
}

// g++ --std=c++11 -O3 -Wall tests/perf-baseline.cpp -o tmp-tests  &&  time ./tmp-tests

// % systemd-run --user --scope -p MemoryMax=7000K -p MemorySwapMax=0M -E "LD_PRELOAD=/home/jpivarski/mambaforge/lib/libjemalloc.so" time ./tmp-tests
// Running scope as unit: run-r3744da54a68845338fec056fe6f3ae63.scope
// dummy 100000
// 3.32user 0.00system 0:03.33elapsed 99%CPU (0avgtext+0avgdata 9288maxresident)k

// % systemd-run --user --scope -p MemoryMax=7000K -p MemorySwapMax=0M -E "LD_PRELOADY=/home/jpivarski/mambaforge/lib/libjemalloc.so" time ./tmp-tests
// Running scope as unit: run-r94801b46b99c42638f2c60a26f0b7ec1.scope
// dummy 100000
// 3.65user 0.00system 0:03.65elapsed 99%CPU (0avgtext+0avgdata 9336maxresident)k
// 0inputs+0outputs (0major+1694minor)pagefaults 0swaps

// No-allocation, just-assignment is consistently 0.5 seconds (regardless of allocator, of course).
// So allocation itself is about 3 seconds, and allocator (consistently) makes a 10% difference.
// And allocation itself is like, 7 assignments.

// The cost of standard malloc and jemalloc are about the same as 150 local/consecutive assignments.
