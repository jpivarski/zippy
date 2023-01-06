#ifndef ZIPPY_TEST_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <zippy/data.h>

#define handle_error(error)                                              \
  if (error != ZP_NO_ERROR) {                                            \
    printf("line %d: error: %s\n", __LINE__, error);                     \
    return -1;                                                           \
  }                                                                      \

#define handle_not_error(error)                                          \
  if (error == ZP_NO_ERROR) {                                            \
    printf("line %d: didn't encounter expected error\n", __LINE__);      \
    return -1;                                                           \
  }                                                                      \

#define assert(obtained, expected)                                       \
  if ((obtained) != (expected)) {                                        \
    printf("line %d: unexpected value\n", __LINE__); \
    return -1;                                                           \
  }                                                                      \

#define assert_Context_refcnt(context, size, expected)                   \
  if (!is_good_Context_refcnt(context, size, expected)) {                \
    printf("line %d: refcnt state differs from expected\n", __LINE__);   \
    return -1;                                                           \
  }                                                                      \

#define assert_Context_slab(context, size, expected)                     \
  if (!is_good_Context_slab(context, size, expected)) {                  \
    printf("line %d: slab state differs from expected\n", __LINE__);     \
    return -1;                                                           \
  }                                                                      \

#define assert_Context_region(context, size, expected)                   \
  if (!is_good_Context_region(context, size, expected)) {                \
    printf("line %d: region state differs from expected\n", __LINE__);   \
    return -1;                                                           \
  }                                                                      \


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
  ZP_Index_t k = 0;

  if (view_slab) {
    char tmp[36];

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
          sprintf(tmp, "%04x,%04x ", context->slab_allocator.arena[2 * i], context->slab_allocator.arena[2 * i + 1]);
          for (k = 0;  k < 36;  k++) {
            if (tmp[k] == '0') {
              tmp[k] = '_';
            }
          }
          printf("%s", tmp);
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
          sprintf(tmp, "%08x,%08x ", context->slab_allocator.arena[2 * i], context->slab_allocator.arena[2 * i + 1]);
          for (k = 0;  k < 36;  k++) {
            if (tmp[k] == '0') {
              tmp[k] = '_';
            }
          }
          printf("%s", tmp);
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
          sprintf(tmp, "%016lx,%016lx ", context->slab_allocator.arena[2 * i], context->slab_allocator.arena[2 * i + 1]);
          for (k = 0;  k < 36;  k++) {
            if (tmp[k] == '0') {
              tmp[k] = '_';
            }
          }
          printf("%s", tmp);
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
        printf("__");
      }
      else if (('A' <= x  &&  x <= 'Z')  ||  ('a' <= x  &&  x <= 'z')) {
        printf("'%c", x);
      }
      else {
        printf("%02x", x);
      }
      if ((i + 1) % 24 == 0) {
        printf("\n");
      }
      else if ((i + 1) % sizeof(ZP_Index_t) == 0) {
        printf("|");
      }
      else {
        printf(" ");
      }
    }

    if (context->region_allocator.arena_size % 24 != 0) {
      printf("\n");
    }
  }
}

void state_Context_refcnt(ZP_Context_t* context) {
  ZP_Index_t size = context->slab_allocator.num_slabs * sizeof(ZP_RefCnt_t);
#if ZIPPY_POINTER_SIZE == 16
  printf("  assert_Context_refcnt(&context, %d, \"", size);
#elif ZIPPY_POINTER_SIZE == 32
  printf("  assert_Context_refcnt(&context, %d, \"", size);
#else
  printf("  assert_Context_refcnt(&context, %ld, \"", size);
#endif
  ZP_Index_t i;
  for (i = 0;  i < size;  i++) {
    printf("\\x%02x", ((unsigned char*)context->refcnt)[i]);
  }
  printf("\");\n");
}

void state_Context_slab(ZP_Context_t* context) {
  ZP_Index_t size = 2 * context->slab_allocator.num_slabs * sizeof(ZP_Index_t);
#if ZIPPY_POINTER_SIZE == 16
  printf("  assert_Context_slab(&context, %d, \"", size);
#elif ZIPPY_POINTER_SIZE == 32
  printf("  assert_Context_slab(&context, %d, \"", size);
#else
  printf("  assert_Context_slab(&context, %ld, \"", size);
#endif
  ZP_Index_t i;
  for (i = 0;  i < size;  i++) {
    printf("\\x%02x", ((unsigned char*)context->slab_allocator.arena)[i]);
  }
  printf("\");\n");
}

void state_Context_region(ZP_Context_t* context) {
  ZP_Index_t size = context->region_allocator.arena_size;
#if ZIPPY_POINTER_SIZE == 16
  printf("  assert_Context_region(&context, %d, \"", size);
#elif ZIPPY_POINTER_SIZE == 32
  printf("  assert_Context_region(&context, %d, \"", size);
#else
  printf("  assert_Context_region(&context, %ld, \"", size);
#endif
  ZP_Index_t i;
  for (i = 0;  i < size;  i++) {
    printf("\\x%02x", ((unsigned char*)context->region_allocator.arena)[i]);
  }
  printf("\");\n");
}

bool is_good_Context_refcnt(ZP_Context_t* context, ZP_Index_t size, const char* expected) {
  ZP_Index_t i;
  ZP_Index_t j;
  for (i = 0;  i < size;  i++) {
    if (((unsigned char*)expected)[i] != ((unsigned char*)context->refcnt)[i]) {
      printf("expected:\n");
      for (j = 0;  j < size;  j++) {
        printf("\\x%02x", ((unsigned char*)expected)[j]);
      }
      printf("\nobtained:\n");
      for (j = 0;  j < i;  j++) {
        printf("....");
      }
      for (j = i;  j < size;  j++) {
        printf("\\x%02x", ((unsigned char*)context->refcnt)[j]);
      }
      printf("\n");
      return false;
    }
  }
  return true;
}

bool is_good_Context_slab(ZP_Context_t* context, ZP_Index_t size, const char* expected) {
  ZP_Index_t i;
  ZP_Index_t j;
  for (i = 0;  i < size;  i++) {
    if (((unsigned char*)expected)[i] != ((unsigned char*)context->slab_allocator.arena)[i]) {
      printf("expected:\n");
      for (j = 0;  j < size;  j++) {
        printf("\\x%02x", ((unsigned char*)expected)[j]);
      }
      printf("\nobtained:\n");
      for (j = 0;  j < i;  j++) {
        printf("....");
      }
      for (j = i;  j < size;  j++) {
        printf("\\x%02x", ((unsigned char*)context->slab_allocator.arena)[j]);
      }
      printf("\n");
      return false;
    }
  }
  return true;
}

bool is_good_Context_region(ZP_Context_t* context, ZP_Index_t size, const char* expected) {
  ZP_Index_t i;
  ZP_Index_t j;
  for (i = 0;  i < size;  i++) {
    if (((unsigned char*)expected)[i] != ((unsigned char*)context->region_allocator.arena)[i]) {
      printf("expected:\n");
      for (j = 0;  j < size;  j++) {
        printf("\\x%02x", ((unsigned char*)expected)[j]);
      }
      printf("\nobtained:\n");
      for (j = 0;  j < i;  j++) {
        printf("....");
      }
      for (j = i;  j < size;  j++) {
        printf("\\x%02x", ((unsigned char*)context->region_allocator.arena)[j]);
      }
      printf("\n");
      return false;
    }
  }
  return true;
}

#endif /* ZIPPY_TEST_UTIL_H */
