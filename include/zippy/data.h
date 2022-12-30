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
  #define ZP_TAG_OBJECT_MASK        0xf000   /* 1111............ 8 tags, 4095 addresses * 4-byte pairs = 16 kB */
  #define ZP_TAG_OBJECT_ANTIMASK    0x8000   /* 1000............ */
  #define ZP_TAG_OBJECT_ENUM        0x0000   /* 0000............ tag 0 */
  #define ZP_TAG_OBJECT_LIST        0x1000   /* 0001............ tag 1 */
  #define ZP_TAG_OBJECT_INSTANCE    0x2000   /* 0010............ tag 2 */
  #define ZP_TAG_OBJECT_FUNCTION    0x3000   /* 0011............ tag 3 */
  #define ZP_TAG_OBJECT_RESERVED1   0x4000   /* 0100............ tag 4 */
  #define ZP_TAG_OBJECT_RESERVED2   0x5000   /* 0101............ tag 5 */
  #define ZP_TAG_OBJECT_RESERVED3   0x6000   /* 0110............ tag 6 */
  #define ZP_TAG_OBJECT_OTHER       0x7000   /* 0111............ tag 7 */
  #define ZP_TERMINATOR             0x0fff   /* ends lists and trees, maximum number of pairs in SlabAllocator */
#elif ZIPPY_POINTER_SIZE == 32
  #define ZP_TAG_FLOAT              0x80000000   /* 1............................... 31-bit float */
  #define ZP_TAG_INT                0x40000000   /* 01.............................. 30-bit int */
  #define ZP_TAG_OBJECT_MASK        0xf8000000   /* 11111........................... (2**(32 - 5) - 1) * 8 byte = 1 GB */
  #define ZP_TAG_OBJECT_ANTIMASK    0xc0000000   /* 11000........................... */
  #define ZP_TAG_OBJECT_ENUM        0x00000000   /* 00000........................... */
  #define ZP_TAG_OBJECT_LIST        0x08000000   /* 00001........................... */
  #define ZP_TAG_OBJECT_INSTANCE    0x10000000   /* 00010........................... */
  #define ZP_TAG_OBJECT_FUNCTION    0x18000000   /* 00011........................... */
  #define ZP_TAG_OBJECT_RESERVED1   0x20000000   /* 00100........................... */
  #define ZP_TAG_OBJECT_RESERVED2   0x28000000   /* 00101........................... */
  #define ZP_TAG_OBJECT_RESERVED3   0x30000000   /* 00110........................... */
  #define ZP_TAG_OBJECT_OTHER       0x38000000   /* 00111........................... */
  #define ZP_TERMINATOR             0x07ffffff
#else /* ZIPPY_POINTER_SIZE == 64 */
  #define ZP_TAG_FLOAT              0x8000000000000000   /* 1....... 63-bit float */
  #define ZP_TAG_INT                0x4000000000000000   /* 01...... 62-bit int */
  #define ZP_TAG_OBJECT_MASK        0xf800000000000000   /* 11111... (2**(64 - 5) - 1) * 16 byte = 8 EB */
  #define ZP_TAG_OBJECT_ANTIMASK    0xc000000000000000   /* 11000... */
  #define ZP_TAG_OBJECT_ENUM        0x0000000000000000   /* 00000... */
  #define ZP_TAG_OBJECT_LIST        0x0800000000000000   /* 00001... */
  #define ZP_TAG_OBJECT_INSTANCE    0x1000000000000000   /* 00010... */
  #define ZP_TAG_OBJECT_FUNCTION    0x1800000000000000   /* 00011... */
  #define ZP_TAG_OBJECT_RESERVED1   0x2000000000000000   /* 00100... */
  #define ZP_TAG_OBJECT_RESERVED2   0x2800000000000000   /* 00101... */
  #define ZP_TAG_OBJECT_RESERVED3   0x3000000000000000   /* 00110... */
  #define ZP_TAG_OBJECT_OTHER       0x3800000000000000   /* 00111... */
  #define ZP_TERMINATOR             0x07ffffffffffffff
#endif

/* can have at most 4095 built-in enumerations */
enum {
  /* singleton objects */

  ZP_ENUM_NONE = 0,
  ZP_ENUM_FALSE,
  ZP_ENUM_TRUE,

  /* built-in type designators */

  ZP_ENUM_OTHER_SYMBOL,
  ZP_ENUM_OTHER_BYTESTRING,

  ZP_ENUM_OTHER_SIMPLE_CLASSDEF,

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

void ZP_RegionAllocator_new(
  ZP_RegionAllocator_t* region_allocator,
  void* arena,
  size_t arena_size
);

bool ZP_RegionAllocator_is_full(
  ZP_RegionAllocator_t* region_allocator
);

void* ZP_RegionAllocator_at(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer
);

ZP_Error_t ZP_RegionAllocator_alloc(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer,
  ZP_Index_t size
);

ZP_Error_t ZP_RegionAllocator_incref(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer
);

void ZP_RegionAllocator_decref(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer
);





#endif /* ZIPPY_DATA_H */
