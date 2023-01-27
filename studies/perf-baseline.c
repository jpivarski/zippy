#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define SLAB_SIZE 8

struct ZP_SlabAllocator {
  size_t* arena;
  size_t* arena_end;
  size_t* first_free;
  size_t* bump_free;
};
typedef struct ZP_SlabAllocator ZP_SlabAllocator_t;

struct ZP_SlabPointer {
  size_t* at;
};
typedef struct ZP_SlabPointer ZP_SlabPointer_t;

void ZP_SlabAllocator_new(
  ZP_SlabAllocator_t* slab_allocator,
  size_t num_slabs
) {
  slab_allocator->arena = (size_t*)malloc(num_slabs * SLAB_SIZE * sizeof(size_t));
  slab_allocator->arena_end = &slab_allocator->arena[num_slabs * SLAB_SIZE];
  slab_allocator->first_free = NULL;
  slab_allocator->bump_free = slab_allocator->arena;
}

void ZP_SlabAllocator_del(
  ZP_SlabAllocator_t* slab_allocator
) {
  free((void*)slab_allocator->arena);
}

void ZP_SlabAllocator_alloc(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer
) {
  if (slab_allocator->first_free == NULL) {
    if (slab_allocator->bump_free == slab_allocator->arena_end) {
      pointer->at = NULL;
    }
    else {
      pointer->at = slab_allocator->bump_free;
      slab_allocator->bump_free += SLAB_SIZE;
    }
  }
  else {
    pointer->at = slab_allocator->first_free;
    slab_allocator->first_free = (size_t*)(*slab_allocator->first_free);
  }
}

void ZP_SlabAllocator_free(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer
) {
  *(pointer->at) = (size_t)slab_allocator->first_free;
  slab_allocator->first_free = pointer->at;
}

int main(int argc, char** argv) {
  ZP_SlabPointer_t current;
  ZP_SlabPointer_t head;
  ZP_SlabPointer_t prev;
  size_t i = 0;
  ZP_SlabPointer_t next_head;

  /* current = (size_t*)malloc(64); */
  /* head = current; */
  /* prev = current; */
  /* for (i = 0;  i < 100000;  i++) { */
  /*   current = (size_t*)malloc(64); */
  /*   prev[0] = (size_t)current; */
  /*   prev = current; */
  /* } */

  /* for (i = 0;  i < 2000000000;  i++) { */
  /*   next_head = (size_t*)head[0]; */
  /*   free((void*)head); */
  /*   head = next_head; */

  /*   current = (size_t*)malloc(64); */
  /*   prev[0] = (size_t)current; */
  /*   prev = current; */
  /* } */

  /* for (i = 0;  i < 100000;  i++) { */
  /*   next_head = (size_t*)head[0]; */
  /*   free((void*)head); */
  /*   head = next_head; */
  /* } */

  ZP_SlabAllocator_t slab_allocator;
  ZP_SlabAllocator_new(&slab_allocator, SLAB_SIZE * 100000);

  ZP_SlabAllocator_alloc(&slab_allocator, &current);
  head.at = current.at;
  prev.at = current.at;
  for (i = 0;  i < 100000;  i++) {
    ZP_SlabAllocator_alloc(&slab_allocator, &current);
    prev.at[0] = (size_t)current.at;
    prev.at = current.at;
  }

  for (i = 0;  i < 2000000000;  i++) {
    next_head.at = (size_t*)head.at[0];
    ZP_SlabAllocator_free(&slab_allocator, &head);
    head.at = next_head.at;

    ZP_SlabAllocator_alloc(&slab_allocator, &current);
    prev.at[0] = (size_t)current.at;
    prev.at = current.at;
  }

  for (i = 0;  i < 100000;  i++) {
    next_head.at = (size_t*)head.at[0];
    ZP_SlabAllocator_free(&slab_allocator, &head);
    head.at = next_head.at;
  }

  ZP_SlabAllocator_del(&slab_allocator);

  printf("dummy %ld\n", i);

  return 0;
}

/* gcc --std=c99 -O3 -Wall tests/perf-baseline.c -o tmp-tests */

/* With malloc or jemalloc (top block of code), the time is 1.6 or 2.2 seconds, respectively. */

/* % systemd-run --user --scope -p MemoryMax=10000K -p MemorySwapMax=0M -E "LD_PRELOADY=/home/jpivarski/mambaforge/lib/libjemalloc.so" time ./tmp-tests */
/* Running scope as unit: run-r69115aa136734a6bb5c93ae8e2c36dd1.scope */
/* dummy 100000 */
/* 1.64user 0.01system 0:01.66elapsed 99%CPU (0avgtext+0avgdata 9408maxresident)k */
/* 0inputs+0outputs (0major+2026minor)pagefaults 0swaps */

/* % systemd-run --user --scope -p MemoryMax=10000K -p MemorySwapMax=0M -E "LD_PRELOAD=/home/jpivarski/mambaforge/lib/libjemalloc.so" time ./tmp-tests */
/* Running scope as unit: run-r310bcfefc44c4fec9d4531cee340aec3.scope */
/* dummy 100000 */
/* 2.18user 0.00system 0:02.18elapsed 99%CPU (0avgtext+0avgdata 10188maxresident)k */
/* 0inputs+0outputs (0major+1878minor)pagefaults 0swaps */

/* With the slab allocator (bottom block of code), the time is 0.2 seconds. */

/* % systemd-run --user --scope -p MemoryMax=10000K -p MemorySwapMax=0M -E "LD_PRELOADY=/home/jpivarski/mambaforge/lib/libjemalloc.so" time ./tmp-tests */
/* Running scope as unit: run-r0575cd34175a467e887746f9c353f91e.scope */
/* dummy 100000 */
/* 0.23user 0.00system 0:00.24elapsed 99%CPU (0avgtext+0avgdata 7204maxresident)k */
/* 0inputs+0outputs (0major+1636minor)pagefaults 0swaps */

/* % systemd-run --user --scope -p MemoryMax=10000K -p MemorySwapMax=0M -E "LD_PRELOAD=/home/jpivarski/mambaforge/lib/libjemalloc.so" time ./tmp-tests */
/* Running scope as unit: run-r90571125f6744e0daf94bc73421a1c5a.scope */
/* dummy 100000 */
/* 0.22user 0.00system 0:00.23elapsed 99%CPU (0avgtext+0avgdata 9584maxresident)k */
/* 0inputs+0outputs (0major+1789minor)pagefaults 0swaps */

/* The factor of 10 advantage is a factor of 2 advantage if the arena is larger than L3. */
