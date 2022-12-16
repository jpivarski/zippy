#ifndef _ZIPPY_DATA_H

#include <zippy/util.h>

typedef const char* ZP_ERROR;
typedef uint32_t ZP_RAW;

#define ZP_NOERROR 0
#define ZP_END_OF_FREE -1

struct ZP_RegionAllocator {
  void* arena;
  size_t arena_size;
  ZP_RAW first_free;
};
typedef struct ZP_RegionAllocator ZP_RegionAllocator_t;

ZP_ERROR ZP_RegionAllocator_new(
  ZP_RegionAllocator_t* region_allocator,
  void* arena,
  size_t arena_size
);

ZP_RAW ZP_RegionAllocator_round_size(
  ZP_RAW request_size
);

void ZP_RegionAllocator_copy_data(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RAW pointer,
  ZP_RAW request_size,
  const char* data
);

ZP_ERROR ZP_RegionAllocator_alloc(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RAW request_size,
  ZP_RAW* pointer
);

ZP_ERROR ZP_RegionAllocator_free(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RAW pointer,
  ZP_RAW request_size
);

#endif /* _ZIPPY_DATA_H */
