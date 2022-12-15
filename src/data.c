#include <stdio.h>  /* temporary */

#include <zippy/data.h>

ZP_ERROR ZP_RegionAllocator_new(
  ZP_RegionAllocator_t* region_allocator,
  void* arena,
  size_t arena_size
) {
  if (arena_size < 2 * sizeof(ZP_RAW)) {
    return "ZP_RegionAllocator arena size must be at least 2*sizeof(ZP_RAW)";
  }
  if (arena_size % (2 * sizeof(ZP_RAW)) != 0) {
    return "ZP_RegionAllocator arena size must be a multiple of 2*sizeof(ZP_RAW)";
  }
  region_allocator->arena = arena;
  region_allocator->arena_size = arena_size;
  region_allocator->first_free = 0;
  *(ZP_RAW*)region_allocator->arena = arena_size;
  *(ZP_RAW*)(region_allocator->arena + sizeof(ZP_RAW)) = ZP_END_OF_FREE;
  return ZP_NOERROR;
}

bool ZP_RegionAllocator_has_free(
  ZP_RegionAllocator_t* region_allocator
) {
  return region_allocator->first_free != ZP_END_OF_FREE;
}

ZP_RAW ZP_RegionAllocator_this_free_size(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RAW at
) {
  return *(ZP_RAW*)(region_allocator->arena + at);
}

ZP_RAW ZP_RegionAllocator_next_free(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RAW at
) {
  return *(ZP_RAW*)(region_allocator->arena + at + sizeof(ZP_RAW));
}

ZP_RAW ZP_RegionAllocator_round_size(
  ZP_RAW request_size
) {
  const ZP_RAW denom = 2 * sizeof(ZP_RAW);
  ZP_RAW quot = request_size / denom;
  ZP_RAW rem = request_size % denom;
  return denom * (quot + (rem != 0));
}

ZP_ERROR ZP_RegionAllocator_alloc(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RAW request_size,
  ZP_RAW* pointer
) {
  ZP_RAW round_size = ZP_RegionAllocator_round_size(request_size);
  ZP_RAW prev = ZP_END_OF_FREE;
  ZP_RAW at = region_allocator->first_free;

  while (true) {
    if (at == ZP_END_OF_FREE) {
      return "out of memory in RegionAllocator";
    }

    ZP_RAW next_free = ZP_RegionAllocator_next_free(region_allocator, at);
    ZP_RAW this_free_size = ZP_RegionAllocator_this_free_size(region_allocator, at);

    if (round_size < this_free_size) {
      /* split the region */
      *pointer = at;
      at += round_size;
      this_free_size -= round_size;
      *(ZP_RAW*)(region_allocator->arena + at) = this_free_size;
      *(ZP_RAW*)(region_allocator->arena + at + sizeof(ZP_RAW)) = next_free;
      if (prev == ZP_END_OF_FREE) {
        /* update linked list head to point here */
        region_allocator->first_free = at;
      }
      else {
        /* update previous linked list node to point here */
        *(ZP_RAW*)(region_allocator->arena + prev + sizeof(ZP_RAW)) = at;
      }
      break;
    }

    else if (round_size == this_free_size) {
      return "not implemented 2";
    }

    else {
      return "not implemented 3";
    }
  }

  return ZP_NOERROR;
}

void ZP_RegionAllocator_copy_data(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RAW pointer,
  ZP_RAW request_size,
  const char* data
) {
  const ZP_RAW denom = sizeof(ZP_RAW);
  ZP_RAW quot = request_size / denom;
  ZP_RAW rem = request_size % denom;
  ZP_RAW count = (quot + (rem != 0));

  ZP_RAW i;
  for (i = 0;  i < count;  i++) {
    ((ZP_RAW*)(region_allocator->arena + pointer))[i] = ((ZP_RAW*)data)[i];
  }
}
