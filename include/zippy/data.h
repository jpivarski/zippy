#ifndef _ZIPPY_DATA_H

#include <zippy/util.h>

typedef const char* ZP_ERROR;
typedef uint32_t ZP_RAW;
typedef int32_t ZP_INTEGER;
typedef float ZP_FLOAT;

/***** Labeled ***************************************************************/

typedef int8_t ZP_COMPARE;

typedef uint8_t ZP_LABEL;

/* enumerated labels in the first 4 bits */
#define ZP_LABEL_POINTER         0x00
#define ZP_LABEL_NONE            0x01
#define ZP_LABEL_BOOL            0x02
#define ZP_LABEL_STRING          0x03
#define ZP_LABEL_LIST            0x04
#define ZP_LABEL_RECORD_TYPE     0x05
#define ZP_LABEL_RECORD_OBJECT   0x06
#define ZP_LABEL_RECORD_NODE     0x07
#define ZP_LABEL_RESERVED1       0x08
#define ZP_LABEL_RESERVED2       0x09
#define ZP_LABEL_RESERVED3       0x0a
#define ZP_LABEL_RESERVED4       0x0b
#define ZP_LABEL_RESERVED5       0x0c
#define ZP_LABEL_RESERVED6       0x0d
#define ZP_LABEL_RESERVED7       0x0e
#define ZP_LABEL_RESERVED8       0x0f

/* special label classes in the next 4 bits */
#define ZP_LABEL_ARRAY_SIMPLE    0x10
#define ZP_LABEL_ARRAY_FULL      0x20
#define ZP_LABEL_INTEGER         0x40
#define ZP_LABEL_FLOAT           0x80

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

struct ZP_Labeled {
  ZP_RAW data;
};
typedef struct ZP_Labeled ZP_Labeled_t;

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

/***** ConsAllocator *********************************************************/










/***** Symbols ***************************************************************/

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
