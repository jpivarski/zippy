#ifndef ZIPPY_TEST_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <zippy/data.h>

#define handle_error(error)        \
  if (error != ZP_NO_ERROR) {      \
    printf("error: %s\n", error);  \
    return -1;                     \
  }                                \

#define handle_not_error(error)    \
  if (error == ZP_NO_ERROR) {      \
    printf("didn't encounter expected error\n"); \
    return -1;                     \
  }                                \

#define assert(obtained, expected) \
  if ((obtained) != (expected)) {  \
    printf("line %d: expected %x but obtained %x\n", __LINE__, expected, obtained); \
    return -1;                     \
  }                                \

ZP_Error_t new_context(ZP_Context_t* context, size_t slab_size, size_t region_size) {
  size_t refcnt_size = slab_size * sizeof(ZP_RefCnt_t) / (2 * sizeof(ZP_Index_t));

  void* slab_arena = malloc(slab_size);
  void* region_arena = malloc(region_size);
  void* refcnt_arena = malloc(refcnt_size);

  return ZP_Context_new(
    context,
    slab_arena,
    region_arena,
    refcnt_arena,
    slab_size,
    region_size,
    refcnt_size
  );
}

void init_Context(ZP_Context_t* context) {
  ZP_Index_t i;
  for (i = 0;  i < context->slab_allocator.num_slabs;  i++) {
    context->refcnt[i] = 0;
  }
  for (i = 0;  i < 2 * context->slab_allocator.num_slabs;  i++) {
    context->slab_allocator.arena[i] = 0;
  }
  for (i = 0;  i < context->region_allocator.arena_size;  i++) {
    ((unsigned char*)context->slab_allocator.arena)[i] = 0;
  }
}

void view_Context(ZP_Context_t* context, bool view_slab, bool view_region) {
  ZP_Index_t i = 0;
  ZP_Index_t j = 0;

  if (view_slab) {
    printf("refcnt + slab\n");
    while (i < context->slab_allocator.num_slabs) {
#if ZIPPY_POINTER_SIZE == 16
      for (j = 0;  j < 8;  j++) {
        if (i < context->slab_allocator.num_slabs) {
          printf("%9d ", context->refcnt[i]);
        }
        i++;
      }
      printf("\n");
      i -= 8;
      for (j = 0;  j < 8;  j++) {
        if (i < context->slab_allocator.num_slabs) {
          printf("%04x,%04x ", context->slab_allocator.arena[2 * i], context->slab_allocator.arena[2 * i + 1]);
        }
        i++;
      }
      printf("\n");
#elif ZIPPY_POINTER_SIZE == 32
      for (j = 0;  j < 4;  j++) {
        if (i < context->slab_allocator.num_slabs) {
          printf("%17d ", context->refcnt[i]);
        }
        i++;
      }
      printf("\n");
      i -= 4;
      for (j = 0;  j < 4;  j++) {
        if (i < context->slab_allocator.num_slabs) {
          printf("%08x,%08x ", context->slab_allocator.arena[2 * i], context->slab_allocator.arena[2 * i + 1]);
        }
        i++;
      }
      printf("\n");
#elif ZIPPY_POINTER_SIZE == 64
      for (j = 0;  j < 2;  j++) {
        if (i < context->slab_allocator.num_slabs) {
          printf("%33d ", context->refcnt[i]);
        }
        i++;
      }
      printf("\n");
      i -= 2;
      for (j = 0;  j < 2;  j++) {
        if (i < context->slab_allocator.num_slabs) {
          printf("%08x,%08x ", context->slab_allocator.arena[2 * i], context->slab_allocator.arena[2 * i + 1]);
        }
        i++;
      }
      printf("\n");
  #endif
    }
  }

  if (view_region) {
    printf("region\n");
    for (i = 0;  i < context->region_allocator.arena_size;  i++) {
      unsigned char x = ((unsigned char*)context->region_allocator.arena)[i];
      if (x == 0) {
        printf("__ ");
      }
      else if (('A' <= x  &&  x <= 'Z')  ||  ('a' <= x  &&  x <= 'z')) {
        printf("'%c ", x);
      }
      else {
        printf("%02x ", x);
      }
      if ((i + 1) % 24 == 0) {
        printf("\n");
      }
    }

    if (context->region_allocator.arena_size % 24 != 0) {
      printf("\n");
    }
  }
}




#endif /* ZIPPY_TEST_UTIL_H */
