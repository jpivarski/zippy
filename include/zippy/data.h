#ifndef _ZIPPY_DATA_H

#include <zippy/util.h>

typedef const char* ZP_ERR;
typedef uint32_t ZP_PTR;

#define ZP_noerr 0
#define ZP_end_of_free -1

struct ZP_Symbol {
  ZP_PTR start;
  ZP_PTR stop;
};
typedef struct ZP_Symbol ZP_Symbol_t;

ZP_ERR ZP_Symbol_new(
  ZP_Symbol_t* symbol
);

struct ZP_RegionAllocator {
  void* arena;
  size_t arena_size;
  ZP_PTR first_free;
};
typedef struct ZP_RegionAllocator ZP_RegionAllocator_t;

ZP_ERR ZP_RegionAllocator_new(
  ZP_RegionAllocator_t* region_allocator,
  void* arena,
  size_t arena_size
);

ZP_ERR ZP_RegionAllocator_has_free(
  ZP_RegionAllocator_t* region_allocator,
  bool* result
);

ZP_ERR ZP_RegionAllocator_next_free(
  ZP_RegionAllocator_t* region_allocator,
  ZP_PTR* result
);

ZP_ERR ZP_RegionAllocator_free_size(
  ZP_RegionAllocator_t* region_allocator,
  ZP_PTR* result
);





#endif /* _ZIPPY_DATA_H */
