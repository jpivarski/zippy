#include <stdio.h>
#include <stdlib.h>

#include <zippy/data.h>

#define handle_error(error) \
  if (error != ZP_noerr) {  \
    printf(error);          \
    return -1;              \
  }                         \

#define assert(obtained, expected)                               \
  if (obtained != expected) {                                    \
    printf("expected %p but obtained %p\n", expected, obtained); \
    return -1;                                                   \
  }                                                              \

int main(int argc, int** argv) {
  size_t arena_size = 1024;
  void* arena = malloc(arena_size);

  ZP_ERR error = ZP_noerr;

  printf("BEGIN tests\n");

  ZP_RegionAllocator_t region_allocator;
  error = ZP_RegionAllocator_new(&region_allocator, arena, arena_size);
  handle_error(error);

  ZP_PTR result;
  error = ZP_RegionAllocator_next_free(&region_allocator, &result);
  handle_error(error);
  printf("next_free %p\n", result);
  assert(result, 0xffffffff);

  error = ZP_RegionAllocator_free_size(&region_allocator, &result);
  handle_error(error);
  printf("free_size %p\n", result);
  assert(result, 0x400);

  printf("END tests\n");

  return 0;
}
