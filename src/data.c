#include <zippy/data.h>

ZP_ERR ZP_Symbol_new(
  ZP_Symbol_t* symbol
) {
  symbol->start = 1;
  symbol->stop = 1;
  return ZP_noerr;
}

ZP_ERR ZP_RegionAllocator_new(
  ZP_RegionAllocator_t* region_allocator,
  void* arena,
  size_t arena_size
) {
  if (arena_size < 2 * sizeof(ZP_PTR)) {
    return "ZP_RegionAllocator arena size must be at least 2*sizeof(ZP_PTR)";
  }
  if (arena_size % (2 * sizeof(ZP_PTR)) != 0) {
    return "ZP_RegionAllocator arena size must be a multiple of 2*sizeof(ZP_PTR)";
  }
  region_allocator->arena = arena;
  region_allocator->arena_size = arena_size;
  region_allocator->first_free = 0;
  *(ZP_PTR*)region_allocator->arena = ZP_end_of_free;
  *(ZP_PTR*)(region_allocator->arena + sizeof(ZP_PTR)) = arena_size;
  return ZP_noerr;
}

ZP_ERR ZP_RegionAllocator_has_free(
  ZP_RegionAllocator_t* region_allocator,
  bool* result
) {
  *result = region_allocator->first_free != ZP_end_of_free;
  return ZP_noerr;
}

ZP_ERR ZP_RegionAllocator_next_free(
  ZP_RegionAllocator_t* region_allocator,
  ZP_PTR* result
) {
  *result = *(ZP_PTR*)region_allocator->arena;
  return ZP_noerr;
}

ZP_ERR ZP_RegionAllocator_free_size(
  ZP_RegionAllocator_t* region_allocator,
  ZP_PTR* result
) {
  *result = *(ZP_PTR*)(region_allocator->arena + sizeof(ZP_PTR));
  return ZP_noerr;
}
