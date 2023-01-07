#ifndef ZIPPY_DEFS_H
#define ZIPPY_DEFS_H

/* FIXME: temporary! */
#include <stdio.h>
/* ***************** */

#include <stdint.h>
#include <stddef.h>

typedef uint8_t bool;

#define false 0
#define true 1

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
  #define ZP_TAG_NODE_MASK          0xe000   /* 111............. 4 tags, 8191 addresses * 4-byte pairs = 32 kB */
  #define ZP_TAG_NODE_ANTIMASK      0x8000   /* 100............. */
  #define ZP_TAG_NODE_ENUM          0x0000   /* 000............. tag #1 */
  #define ZP_TAG_NODE_LIST          0x2000   /* 001............. tag #2 */
  #define ZP_TAG_NODE_INTERNAL      0x4000   /* 010............. tag #3 */
  #define ZP_TAG_NODE_OBJECT        0x6000   /* 011............. tag #4 */
  #define ZP_TERMINATOR             0x1fff   /* ends lists and trees, maximum number of pairs in SlabAllocator */
#elif ZIPPY_POINTER_SIZE == 32
  #define ZP_TAG_FLOAT              0x80000000   /* 1............................... 31-bit float */
  #define ZP_TAG_INT                0x40000000   /* 01.............................. 30-bit int */
  #define ZP_TAG_NODE_MASK          0xf0000000   /* 1111............................ (2**(32 - 4) - 1) * 8 byte = 2 GB */
  #define ZP_TAG_NODE_ANTIMASK      0xc0000000   /* 1100............................ */
  #define ZP_TAG_NODE_ENUM          0x00000000   /* 0000............................ */
  #define ZP_TAG_NODE_LIST          0x10000000   /* 0001............................ */
  #define ZP_TAG_NODE_INTERNAL      0x20000000   /* 0010............................ */
  #define ZP_TAG_NODE_OBJECT        0x30000000   /* 0011............................ */
  #define ZP_TERMINATOR             0x0fffffff
#else /* ZIPPY_POINTER_SIZE == 64 */
  #define ZP_TAG_FLOAT              0x8000000000000000   /* 1....... 63-bit float */
  #define ZP_TAG_INT                0x4000000000000000   /* 01...... 62-bit int */
  #define ZP_TAG_NODE_MASK          0xf000000000000000   /* 1111... (2**(64 - 4) - 1) * 16 byte = 16 EB */
  #define ZP_TAG_NODE_ANTIMASK      0xc000000000000000   /* 1100... */
  #define ZP_TAG_NODE_ENUM          0x0000000000000000   /* 0000... */
  #define ZP_TAG_NODE_LIST          0x1000000000000000   /* 0001... */
  #define ZP_TAG_NODE_INTERNAL      0x2000000000000000   /* 0010... */
  #define ZP_TAG_NODE_OBJECT        0x3000000000000000   /* 0011... */
  #define ZP_TERMINATOR             0x0fffffffffffffff
#endif

#define ZP_ENUM_DTYPE_ARRAY_ANTIMASK    0xffc0

enum {
  /* array dtypes, up to 0x003f */
  ZP_ENUM_DTYPE_ARRAY_BOOL            = 0x0000,
  ZP_ENUM_DTYPE_ARRAY_UINT8           = 0x0001,
  ZP_ENUM_DTYPE_ARRAY_UINT16          = 0x0002,
  ZP_ENUM_DTYPE_ARRAY_UINT32          = 0x0003,
  ZP_ENUM_DTYPE_ARRAY_UINT64          = 0x0004,
  ZP_ENUM_DTYPE_ARRAY_INT8            = 0x0005,
  ZP_ENUM_DTYPE_ARRAY_INT16           = 0x0006,
  ZP_ENUM_DTYPE_ARRAY_INT32           = 0x0007,
  ZP_ENUM_DTYPE_ARRAY_INT64           = 0x0008,
  ZP_ENUM_DTYPE_ARRAY_FLOAT32         = 0x0009,
  ZP_ENUM_DTYPE_ARRAY_FLOAT64         = 0x000a,

  /* singleton objects, up to 0x00ff */
  ZP_ENUM_NONE                        = 0x0040,
  ZP_ENUM_FALSE                       = 0x0041,
  ZP_ENUM_TRUE                        = 0x0042,

  /* type designators for ZP_TAG_NODE_OBJECT, up to 0x1fff */
  ZP_ENUM_OTHER_SYMBOL                = 0x0100,
  ZP_ENUM_OTHER_BYTESTRING            = 0x0101,
  ZP_ENUM_OTHER_UTF8STRING            = 0x0102,
  ZP_ENUM_OTHER_RANDOM_ACCESS_LIST    = 0x0103,
  ZP_ENUM_OTHER_SIMPLE_CLASS          = 0x0104
};

#endif /* ZIPPY_DEFS_H */
