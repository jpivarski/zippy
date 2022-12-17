#ifndef _ZIPPY_DATA_H

#include <zippy/util.h>

typedef const char* ZP_Error_t;
typedef uint32_t ZP_Index_t;
typedef int8_t ZP_Cmp_t;
typedef uint8_t ZP_Label_t;
typedef uint8_t ZP_RCnt_t;

typedef int32_t ZP_Int_t;
typedef float ZP_Float_t;

/***** Labeled ***************************************************************/

/* enumerated labels in the first 4 bits */
#define ZP_LABEL_POINTER         0x00
#define ZP_LABEL_INTERNAL_NODE   0x01
#define ZP_LABEL_BUILTIN_ENUM    0x02
#define ZP_LABEL_STRING          0x03
#define ZP_LABEL_LIST            0x04
#define ZP_LABEL_RECORD_TYPE     0x05
#define ZP_LABEL_RECORD_OBJECT   0x06
#define ZP_LABEL_RESERVED1       0x07
#define ZP_LABEL_RESERVED2       0x08
#define ZP_LABEL_RESERVED3       0x09
#define ZP_LABEL_RESERVED4       0x0a
#define ZP_LABEL_RESERVED5       0x0b
#define ZP_LABEL_RESERVED6       0x0c
#define ZP_LABEL_RESERVED7       0x0d
#define ZP_LABEL_RESERVED8       0x0e
#define ZP_LABEL_RESERVED9       0x0f

/* special label classes in the next 4 bits */
#define ZP_LABEL_ARRAY_SIMPLE    0x10   /* 0001xxxx */
#define ZP_LABEL_ARRAY_FULL      0x20   /* 0010xxxx */
#define ZP_LABEL_ARRAY_SCALAR    0x30   /* 0011xxxx */
#define ZP_LABEL_INT             0x40   /* 01...... */
#define ZP_LABEL_FLOAT           0x80   /* 1....... */

/* dtypes for array classes in the first 4 bits */
#define ZP_LABEL_DTYPE_OBJECT    0x01
#define ZP_LABEL_DTYPE_BOOL      0x02
#define ZP_LABEL_DTYPE_UINT8     0x03
#define ZP_LABEL_DTYPE_UINT16    0x04
#define ZP_LABEL_DTYPE_UINT32    0x05
#define ZP_LABEL_DTYPE_UINT64    0x06
#define ZP_LABEL_DTYPE_INT8      0x07
#define ZP_LABEL_DTYPE_INT16     0x08
#define ZP_LABEL_DTYPE_INT32     0x09
#define ZP_LABEL_DTYPE_INT64     0x0a
#define ZP_LABEL_DTYPE_FLOAT32   0x0b
#define ZP_LABEL_DTYPE_FLOAT64   0x0c
#define ZP_LABEL_DTYPE_RESERVED1 0x0d
#define ZP_LABEL_DTYPE_RESERVED2 0x0e
#define ZP_LABEL_DTYPE_RESERVED3 0x0f

/* data that can be used by ZP_LABEL_BUILTIN_ENUM */
#define ZP_ENUM_FALSE            0
#define ZP_ENUM_TRUE             1
#define ZP_ENUM_NONE             2

struct ZP_Labeled {
  ZP_Index_t data;
};
typedef struct ZP_Labeled ZP_Labeled_t;

ZP_Label_t ZP_label_of(ZP_Labeled_t* labeled);

ZP_Index_t ZP_to_raw(ZP_Labeled_t* labeled);

ZP_Int_t ZP_to_integer(ZP_Labeled_t* labeled);
ZP_Error_t ZP_from_integer(ZP_Int_t* data, ZP_Labeled_t* labeled);

ZP_Float_t ZP_to_float(ZP_Labeled_t* labeled);
void ZP_from_float(ZP_Float_t* data, ZP_Labeled_t* labeled);

/***** RegionAllocator *******************************************************/

#define ZP_NOERROR 0

struct ZP_RegionAllocator {
  void* arena;
  size_t arena_size;
  ZP_Index_t first_free;
};
typedef struct ZP_RegionAllocator ZP_RegionAllocator_t;

ZP_Error_t ZP_RegionAllocator_new(
  ZP_RegionAllocator_t* region_allocator,
  void* arena,
  size_t arena_size
);

bool ZP_RegionAllocator_full(
  ZP_RegionAllocator_t* region_allocator
);

void* ZP_RegionAllocator_at(
  ZP_RegionAllocator_t* region_allocator,
  ZP_Index_t pointer
);

ZP_Error_t ZP_RegionAllocator_alloc(
  ZP_RegionAllocator_t* region_allocator,
  ZP_Index_t request_size,
  ZP_Index_t* pointer
);

void ZP_RegionAllocator_free(
  ZP_RegionAllocator_t* region_allocator,
  ZP_Index_t pointer,
  ZP_Index_t request_size
);

void ZP_RegionAllocator_copy_data(
  ZP_RegionAllocator_t* region_allocator,
  ZP_Index_t pointer,
  ZP_Index_t request_size,
  const char* data
);

/***** PairAllocator *********************************************************/

struct ZP_PairAllocator {
  ZP_Index_t* arena;
  ZP_RCnt_t* refcount;
  ZP_Index_t num_pairs;
  ZP_Index_t first_pointer;
  ZP_Index_t bump_pointer;
};
typedef struct ZP_PairAllocator ZP_PairAllocator_t;

struct ZP_Pair {
  ZP_Index_t first;
};
typedef struct ZP_Pair ZP_Pair_t;

ZP_Error_t ZP_PairAllocator_new(
  ZP_PairAllocator_t* pair_allocator,
  void* arena,
  void* refcount,
  size_t arena_size,
  size_t refcount_size
);

ZP_Error_t ZP_PairAllocator_alloc(
  ZP_PairAllocator_t* pair_allocator,
  ZP_Pair_t* pair
);

void ZP_PairAllocator_free(
  ZP_PairAllocator_t* pair_allocator,
  ZP_Pair_t* pair
);

void ZP_PairAllocator_get(
  ZP_PairAllocator_t* pair_allocator,
  ZP_Pair_t* pair,
  ZP_Index_t* one,
  ZP_Index_t* two
);

void ZP_PairAllocator_set(
  ZP_PairAllocator_t* pair_allocator,
  ZP_Pair_t* pair,
  ZP_Index_t one,
  ZP_Index_t two
);




/***** Symbols ***************************************************************/

struct ZP_SymbolSet {
  ZP_Index_t first_pointer;
};
typedef struct ZP_SymbolSet ZP_SymbolSet_t;

struct ZP_Symbol {
  ZP_Index_t pointer;
};
typedef struct ZP_Symbol ZP_Symbol_t;

ZP_Error_t ZP_SymbolSet_new(
  ZP_SymbolSet_t* symbol_set
);

ZP_Error_t ZP_Symbol_new(
  ZP_SymbolSet_t* symbol_set,
  ZP_RegionAllocator_t* region_allocator,
  const char* symbol_data,
  ZP_Index_t symbol_data_size,
  ZP_Symbol_t* symbol
);

#endif /* _ZIPPY_DATA_H */
