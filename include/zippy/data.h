#ifndef _ZIPPY_DATA_H

#include <zippy/util.h>

typedef const char* ZP_ERROR;
typedef uint32_t ZP_RAW;

/***** RegionAllocator *******************************************************/

#define ZP_NOERROR 0

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

bool ZP_RegionAllocator_full(
  ZP_RegionAllocator_t* region_allocator
);

void* ZP_RegionAllocator_at(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RAW pointer
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

void ZP_RegionAllocator_copy_data(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RAW pointer,
  ZP_RAW request_size,
  const char* data
);

/***** Symbols ***************************************************************/

typedef int8_t ZP_COMPARE;

struct ZP_SymbolSet {
  ZP_RAW first_pointer;
};
typedef struct ZP_SymbolSet ZP_SymbolSet_t;

struct ZP_Symbol {
  ZP_RAW pointer;
};
typedef struct ZP_Symbol ZP_Symbol_t;

ZP_ERROR ZP_SymbolSet_new(
  ZP_SymbolSet_t* symbol_set
);

ZP_ERROR ZP_Symbol_new(
  ZP_SymbolSet_t* symbol_set,
  ZP_RegionAllocator_t* region_allocator,
  const char* symbol_data,
  ZP_RAW symbol_data_size,
  ZP_Symbol_t* symbol
);

#endif /* _ZIPPY_DATA_H */
