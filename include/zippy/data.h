#ifndef ZIPPY_DATA_H

#include <zippy/util.h>

/* SlabAllocator pointers and int/float numbers packed into the same index */
#ifndef ZIPPY_POINTER_SIZE
  #define ZIPPY_POINTER_SIZE 32
#endif
#if ZIPPY_POINTER_SIZE == 16
  typedef uint16_t ZP_Index_t;
  typedef int16_t ZP_Int_t;
  typedef float ZP_Float_t;  /* inaccessible in 16-bit mode */
  #define ZP_END_OF_FREE 0xffff
  #define ZP_MAXINT 16383   /* (1 << (16 - 2)) - 1 */
  #define ZP_MININT -16384  /* -ZP_MAXINT - 1 */
#elif ZIPPY_POINTER_SIZE == 32
  typedef uint32_t ZP_Index_t;
  typedef int32_t ZP_Int_t;
  typedef float ZP_Float_t;
  #define ZP_END_OF_FREE 0xffffffff
  #define ZP_MAXINT 536870911   /* (1 << (32 - 3)) - 1 */
  #define ZP_MININT -536870912
#elif ZIPPY_POINTER_SIZE == 64
  typedef uint64_t ZP_Index_t;
  typedef int64_t ZP_Int_t;
  typedef double ZP_Float_t;
  #define ZP_END_OF_FREE 0xffffffffffffffff
  #define ZP_MAXINT 2305843009213693951   /* (1 << (64 - 3)) - 1 */
  #define ZP_MININT -2305843009213693952
#else
  #error "ZIPPY_POINTER_SIZE can only be 16, 32, or 64 (default 32)"
#endif

/* SlabAllocator reference counts */
#ifndef ZIPPY_REFCNT_SIZE
  #define ZIPPY_REFCNT_SIZE 8
#endif
#if ZIPPY_REFCNT_SIZE == 8
  typedef uint8_t ZP_RefCnt_t;
  #define ZP_REFCNT_MAX 0xff
#elif ZIPPY_REFCNT_SIZE == 16
  typedef uint16_t ZP_RefCnt_t;
  #define ZP_REFCNT_MAX 0xffff
#elif ZIPPY_REFCNT_SIZE == 32
  typedef uint32_t ZP_RefCnt_t;
  #define ZP_REFCNT_MAX 0xffffffff
#elif ZIPPY_REFCNT_SIZE == 64
  typedef uint64_t ZP_RefCnt_t;
  #define ZP_REFCNT_MAX 0xffffffffffffffff
#else
  #error "ZIPPY_REFCNT_SIZE can only be 8, 16, 32, or 64 (default 8)"
#endif

/* errors */
typedef const char* ZP_Error_t;
#define ZP_NO_ERROR 0

#if ZIPPY_POINTER_SIZE == 16
  #define ZP_TAG_FLOAT              0x8000   /* 1............... 15-bit int */
  #define ZP_TAG_INT                0x8000   /* 1............... 15-bit int */
  #define ZP_TAG_OBJECT_MASK        0xe000   /* 111............. 4 tags, 8191 addresses * 4-byte pairs = 32 kB */
  #define ZP_TAG_OBJECT_ANTIMASK    0x8000   /* 100............. */
  #define ZP_TAG_OBJECT_ENUM        0x0000   /* 000............. tag 0 */
  #define ZP_TAG_OBJECT_LIST        0x2000   /* 001............. tag 1 */
  #define ZP_TAG_OBJECT_INSTANCE    0x4000   /* 010............. tag 2 */
  #define ZP_TAG_OBJECT_OTHER       0x6000   /* 011............. tag 7 */
  #define ZP_TERMINATOR             0x1fff   /* ends lists and trees, maximum number of pairs in SlabAllocator */
#elif ZIPPY_POINTER_SIZE == 32
  #define ZP_TAG_FLOAT              0x80000000   /* 1............................... 31-bit float */
  #define ZP_TAG_INT                0x40000000   /* 01.............................. 30-bit int */
  #define ZP_TAG_OBJECT_MASK        0xf0000000   /* 1111............................ (2**(32 - 4) - 1) * 8 byte = 2 GB */
  #define ZP_TAG_OBJECT_ANTIMASK    0xc0000000   /* 1100............................ */
  #define ZP_TAG_OBJECT_ENUM        0x00000000   /* 0000............................ */
  #define ZP_TAG_OBJECT_LIST        0x10000000   /* 0001............................ */
  #define ZP_TAG_OBJECT_INSTANCE    0x20000000   /* 0010............................ */
  #define ZP_TAG_OBJECT_OTHER       0x30000000   /* 0011............................ */
  #define ZP_TERMINATOR             0x0fffffff
#else /* ZIPPY_POINTER_SIZE == 64 */
  #define ZP_TAG_FLOAT              0x8000000000000000   /* 1....... 63-bit float */
  #define ZP_TAG_INT                0x4000000000000000   /* 01...... 62-bit int */
  #define ZP_TAG_OBJECT_MASK        0xf000000000000000   /* 1111... (2**(64 - 4) - 1) * 16 byte = 16 EB */
  #define ZP_TAG_OBJECT_ANTIMASK    0xc000000000000000   /* 1100... */
  #define ZP_TAG_OBJECT_ENUM        0x0000000000000000   /* 0000... */
  #define ZP_TAG_OBJECT_LIST        0x1000000000000000   /* 0001... */
  #define ZP_TAG_OBJECT_INSTANCE    0x2000000000000000   /* 0010... */
  #define ZP_TAG_OBJECT_OTHER       0x3000000000000000   /* 0011... */
  #define ZP_TERMINATOR             0x0fffffffffffffff
#endif

/* can have at most 4095 built-in enumerations */
enum {
  /* singleton objects */

  ZP_ENUM_NONE = 0,
  ZP_ENUM_FALSE,
  ZP_ENUM_TRUE,

  /* built-in type designators */

  ZP_ENUM_OTHER_SIMPLE_CLASS,

  ZP_ENUM_OTHER_ARRAY_BOOL,
  ZP_ENUM_OTHER_ARRAY_UINT8,
  ZP_ENUM_OTHER_ARRAY_UINT16,
  ZP_ENUM_OTHER_ARRAY_UINT32,
  ZP_ENUM_OTHER_ARRAY_UINT64,
  ZP_ENUM_OTHER_ARRAY_INT8,
  ZP_ENUM_OTHER_ARRAY_INT16,
  ZP_ENUM_OTHER_ARRAY_INT32,
  ZP_ENUM_OTHER_ARRAY_INT64,
  ZP_ENUM_OTHER_ARRAY_FLOAT32,
  ZP_ENUM_OTHER_ARRAY_FLOAT64
};

/***** Tagged ****************************************************************/

struct ZP_Tagged {
  ZP_Index_t data;
};
typedef struct ZP_Tagged ZP_Tagged_t;

void ZP_Tagged_copy(
  ZP_Tagged_t* output,
  ZP_Tagged_t* input
);

ZP_Int_t ZP_Tagged_to_int(
  ZP_Tagged_t* tagged
);

ZP_Error_t ZP_Tagged_from_int(
  ZP_Tagged_t* tagged,
  ZP_Int_t* data
);

ZP_Float_t ZP_Tagged_to_float(
  ZP_Tagged_t* tagged
);

ZP_Error_t ZP_Tagged_from_float(
  ZP_Tagged_t* tagged,
  ZP_Float_t* data
);

/***** SlabAllocator *********************************************************/

struct ZP_SlabAllocator {
  ZP_Index_t* arena;
  ZP_Index_t num_pairs;
  ZP_Index_t first_pointer;
  ZP_Index_t bump_pointer;
};
typedef struct ZP_SlabAllocator ZP_SlabAllocator_t;

struct ZP_SlabPointer {
  ZP_Index_t at;
};
typedef struct ZP_SlabPointer ZP_SlabPointer_t;

void ZP_Tagged_to_SlabPointer(
  ZP_SlabPointer_t* pointer,
  ZP_Tagged_t* tagged
);

void ZP_Tagged_from_SlabPointer(
  ZP_Tagged_t* tagged,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t tag
);

void ZP_SlabAllocator_new(
  ZP_SlabAllocator_t* slab_allocator,
  void* arena,
  size_t arena_size
);

bool ZP_SlabAllocator_is_full(
  ZP_SlabAllocator_t* slab_allocator
);

ZP_Error_t ZP_SlabAllocator_alloc(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer
);

void ZP_SlabAllocator_free(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer
);

ZP_Index_t ZP_SlabAllocator_get_left(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer
);

void ZP_SlabAllocator_get(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer,
  ZP_Tagged_t* left,
  ZP_Tagged_t* right
);

void ZP_SlabAllocator_set(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer,
  ZP_Tagged_t* left,
  ZP_Tagged_t* right
);

/***** RegionAllocator *******************************************************/

struct ZP_RegionAllocator {
  void* arena;
  size_t arena_size;
  ZP_Index_t first_free;
};
typedef struct ZP_RegionAllocator ZP_RegionAllocator_t;

struct ZP_RegionPointer {
  ZP_Index_t at;
};
typedef struct ZP_RegionPointer ZP_RegionPointer_t;

void ZP_Tagged_to_RegionPointer(
  ZP_RegionPointer_t* pointer,
  ZP_Tagged_t* tagged
);

void ZP_Tagged_from_RegionPointer(
  ZP_Tagged_t* tagged,
  ZP_RegionPointer_t* pointer
);

void ZP_RegionAllocator_new(
  ZP_RegionAllocator_t* region_allocator,
  void* arena,
  size_t arena_size
);

bool ZP_RegionAllocator_is_full(
  ZP_RegionAllocator_t* region_allocator
);

ZP_Error_t ZP_RegionAllocator_alloc(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer,
  ZP_Index_t size
);

void ZP_RegionAllocator_free(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer
);

ZP_Error_t ZP_RegionAllocator_incref(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer
);

void ZP_RegionAllocator_decref(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer
);

void* ZP_RegionAllocator_at(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer
);

void ZP_RegionAllocator_copy_data(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer,
  const char* data
);

/***** Context ***************************************************************/

struct ZP_Context {
  ZP_SlabAllocator_t slab_allocator;
  ZP_RegionAllocator_t region_allocator;
  ZP_RefCnt_t* refcnt;
  ZP_Tagged_t symbol_set;  /* a list of RegionAllocator regions */
};
typedef struct ZP_Context ZP_Context_t;

ZP_Error_t ZP_Context_new(
  ZP_Context_t* context,
  void* slab_arena,
  void* region_arena,
  void* refcnt_arena,
  size_t slab_size,
  size_t region_size,
  size_t refcnt_size
);

ZP_Error_t ZP_Context_new_or_get_symbol(
  ZP_Context_t* context,
  ZP_RegionPointer_t pointer,
  const char* symbol_data,
  ZP_Index_t symbol_size
);

ZP_Error_t ZP_Context_new_slab(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer
);

void ZP_Context_get(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer,
  ZP_Tagged_t* left,
  ZP_Tagged_t* right
);

void ZP_Context_set(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer,
  ZP_Tagged_t left,
  ZP_Tagged_t right
);

ZP_Error_t ZP_Context_slab_incref(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged
);

void ZP_Context_slab_decref(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged
);

ZP_Error_t ZP_Context_new_region(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer,
  ZP_Index_t size
);

void ZP_Context_copy_data(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer,
  const char* data
);

ZP_Error_t ZP_Context_region_incref(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
);

void ZP_Context_region_decref(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
);

/***** List ******************************************************************/

void ZP_List_decref(
  ZP_Context_t* context,
  ZP_Tagged_t* list
);

void ZP_List_new_empty(
  ZP_Tagged_t* list
);

bool ZP_List_is_empty(
  ZP_Tagged_t* list
);

ZP_Index_t ZP_List_size(
  ZP_Context_t* context,
  ZP_Tagged_t* list
);

void ZP_List_peek_iterate(
  ZP_Context_t* context,
  ZP_Tagged_t* next_list,
  ZP_Tagged_t* this_item,
  ZP_Tagged_t* current_list
);

ZP_Error_t ZP_List_push(
  ZP_Context_t* context,
  ZP_Tagged_t* output_list,
  ZP_Tagged_t* input_list,
  ZP_Tagged_t* item
);

/***** SimpleClass ***********************************************************/

void ZP_SimpleClass_decref(
  ZP_Context_t* context,
  ZP_Tagged_t* simple_class
);

ZP_Error_t ZP_SimpleClass_new(
  ZP_Context_t* context,
  ZP_Tagged_t* simple_class,
  ZP_Tagged_t* list_of_symbols
);



#endif /* ZIPPY_DATA_H */
