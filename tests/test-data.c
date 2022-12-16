#include <stdio.h>
#include <stdlib.h>

#include <zippy/data.h>

#define handle_error(error)   \
  if (error != ZP_NOERROR) {  \
    printf(error);            \
    return -1;                \
  }                           \

#define assert(obtained, expected) \
  if ((obtained) != (expected)) {  \
    printf("expected %x but obtained %x\n", expected, obtained); \
    return -1;                     \
  }                                \

void init(void* arena, size_t arena_size) {
  ZP_RAW i;
  for (i = 0;  i < arena_size;  i++) {
    ((unsigned char*)arena)[i] = 0;
  }
}

void view(void* arena, size_t arena_size) {
  ZP_RAW i;
  for (i = 0;  i < arena_size;  i++) {
    unsigned char x = ((char*)arena)[i];
    if (('A' <= x  &&  x <= 'Z')  ||  ('a' <= x  &&  x <= 'z')) {
      printf("'%c ", x);
    }
    else {
      printf("%02x ", x);
    }
  }
  printf("\n");
}

int main(int argc, char** argv) {
  size_t arena_size = 64;
  void* arena = malloc(arena_size);
  init(arena, arena_size);

  ZP_ERROR error = ZP_NOERROR;

  printf("BEGIN tests\n\n");

  ZP_RegionAllocator_t region_allocator;
  error = ZP_RegionAllocator_new(&region_allocator, arena, arena_size);
  handle_error(error);
  view(arena, arena_size);
  printf("\n");

  ZP_RAW request_size;
  ZP_RAW round_size;
  for (request_size = 0;  request_size < 30;  request_size++) {
    round_size = ZP_RegionAllocator_round_size(request_size);
    assert(round_size >= request_size, true);
  }

  ZP_RAW pointer_one;
  ZP_RAW request_size_one = 11;
  error = ZP_RegionAllocator_alloc(&region_allocator, request_size_one, &pointer_one);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&region_allocator, pointer_one, request_size_one, "hello there");
  view(arena, arena_size);
  printf("\n");

  ZP_RAW pointer_two;
  ZP_RAW request_size_two = 8;
  error = ZP_RegionAllocator_alloc(&region_allocator, request_size_two, &pointer_two);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&region_allocator, pointer_two, request_size_two, "you guys");
  view(arena, arena_size);
  printf("\n");

  ZP_RAW pointer_three;
  ZP_RAW request_size_three = 23;
  error = ZP_RegionAllocator_alloc(&region_allocator, request_size_three, &pointer_three);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&region_allocator, pointer_three, request_size_three, "WHAT SHOULD HAPPEN NOW?");
  view(arena, arena_size);
  printf("\n");

  error = ZP_RegionAllocator_free(&region_allocator, pointer_two, request_size_two);
  handle_error(error);
  view(arena, arena_size);
  printf("\n");

  ZP_RAW pointer_four;
  ZP_RAW request_size_four = 15;
  error = ZP_RegionAllocator_alloc(&region_allocator, request_size_four, &pointer_four);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&region_allocator, pointer_four, request_size_four, "Something else.");
  view(arena, arena_size);
  printf("\n");




  printf("\nEND tests\n");

  return 0;
}
