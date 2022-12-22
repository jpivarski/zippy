#ifndef _ZIPPY_DATA_H

#include <zippy/util.h>

typedef const char* ZP_Error_t;
typedef uint32_t ZP_Index_t;
typedef int8_t ZP_Cmp_t;
typedef uint8_t ZP_Label_t;
typedef uint8_t ZP_RefCnt_t;

typedef int32_t ZP_Int_t;
typedef float ZP_Float_t;

/***** utilities *************************************************************/

void ZP_slice(
  ZP_Index_t* new_pointer,
  ZP_Index_t* new_size,
  ZP_Index_t base_pointer,
  ZP_Index_t base_size,
  ZP_Index_t slice_start,
  ZP_Index_t slice_stop
);

/***** Labeled ***************************************************************/

/* for N = number of bits in ZP_Index_t (e.g. uint32_t => N = 32) */

/* special label classes occupy the high 4 bits */
#define ZP_LABEL_ZERO_REF           0x00   /* 0000.... */
#define ZP_LABEL_ONE_REF            0x10   /* 0001.... */
#define ZP_LABEL_TWO_REF            0x20   /* 0010.... */
#define ZP_LABEL_ARRAY              0x30   /* 0011xxxx where xxxx is dtype */
#define ZP_LABEL_INT                0x40   /* 01...... N-2 bit int */
#define ZP_LABEL_FLOAT              0x80   /* 1....... N-1 bit float */

/* within ZP_LABEL_ZERO_REF */
#define ZP_LABEL_BUILTIN_ENUM       0x00   /* N-8 bit data */
#define ZP_LABEL_BUFFER_OWNED       0x01   /* points to 2 region pointers */
#define ZP_LABEL_BUFFER_VIEW        0x02   /* points to 2 region pointers */

/* within ZP_LABEL_ONE_REF */
#define ZP_LABEL_SCALAR_IN_ARRAY    0x10   /* (base_ref, INT) */

/* within ZP_LABEL_TWO_REF */
#define ZP_LABEL_BUFFER_VIEW_WRAP   0x20   /* (base_ref, BUFFER_VIEW) */
#define ZP_LABEL_LIST               0x21   /* (next_list, item) */
#define ZP_LABEL_CLASS              0x22   /* (list_of_supers, array_of_symbols) */
#define ZP_LABEL_INSTANCE           0x23   /* (class_ref, root_node) */
#define ZP_LABEL_INSTANCE_NODE      0x24   /* (next_left, next_right) */

/* within ZP_LABEL_ARRAY */
#define ZP_LABEL_DTYPE_OBJECTS      0x30
#define ZP_LABEL_DTYPE_BOOL         0x31
#define ZP_LABEL_DTYPE_UINT8        0x32
#define ZP_LABEL_DTYPE_UINT16       0x33
#define ZP_LABEL_DTYPE_UINT32       0x34
#define ZP_LABEL_DTYPE_UINT64       0x35
#define ZP_LABEL_DTYPE_INT8         0x36
#define ZP_LABEL_DTYPE_INT16        0x37
#define ZP_LABEL_DTYPE_INT32        0x38
#define ZP_LABEL_DTYPE_INT64        0x39
#define ZP_LABEL_DTYPE_FLOAT32      0x3a
#define ZP_LABEL_DTYPE_FLOAT64      0x3b

/* data values for ZP_LABEL_BUILTIN_ENUM */
/* this space is inexhaustible: 256**(sizeof(ZP_Index_t) - 1) values */
#define ZP_ENUM_FALSE               0
#define ZP_ENUM_TRUE                1
#define ZP_ENUM_NONE                2

struct ZP_Labeled {
  ZP_Index_t data;
};
typedef struct ZP_Labeled ZP_Labeled_t;

ZP_Label_t ZP_Labeled_label_of(
  ZP_Labeled_t* labeled
);

ZP_Index_t ZP_Labeled_to_raw(
  ZP_Labeled_t* labeled
);

void ZP_Labeled_copy(
  ZP_Labeled_t* output,
  ZP_Labeled_t* input
);

ZP_Int_t ZP_Labeled_to_int(
  ZP_Labeled_t* labeled
);

ZP_Error_t ZP_Labeled_from_int(
  ZP_Labeled_t* labeled,
  ZP_Int_t* data
);

ZP_Float_t ZP_Labeled_to_float(
  ZP_Labeled_t* labeled
);

void ZP_Labeled_from_float(
  ZP_Labeled_t* labeled,
  ZP_Float_t* data
);

/***** FixedAllocator ********************************************************/

struct ZP_FixedAllocator {
  ZP_Index_t* arena;
  ZP_Index_t num_pairs;
  ZP_Index_t first_pointer;
  ZP_Index_t bump_pointer;
};
typedef struct ZP_FixedAllocator ZP_FixedAllocator_t;

struct ZP_FixedLocation {
  ZP_Index_t first;
};
typedef struct ZP_FixedLocation ZP_FixedLocation_t;

void ZP_Labeled_from_FixedLocation(
  ZP_Labeled_t* labeled,
  ZP_FixedLocation_t* location,
  ZP_Index_t label_mask
);

void ZP_FixedAllocator_new(
  ZP_FixedAllocator_t* fixed_allocator,
  void* arena,
  size_t arena_size
);

ZP_Error_t ZP_FixedAllocator_alloc(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_FixedLocation_t* fixed_location
);

void ZP_FixedAllocator_free(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_FixedLocation_t* fixed_location
);

void ZP_FixedAllocator_get(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_FixedLocation_t* fixed_location,
  ZP_Index_t* one,
  ZP_Index_t* two
);

void ZP_FixedAllocator_set(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_FixedLocation_t* fixed_location,
  ZP_Index_t one,
  ZP_Index_t two
);

/***** RegionAllocator *******************************************************/

#define ZP_NOERROR 0

struct ZP_RegionAllocator {
  void* arena;
  size_t arena_size;
  ZP_Index_t first_free;
};
typedef struct ZP_RegionAllocator ZP_RegionAllocator_t;

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

/***** Context ***************************************************************/

struct ZP_Context {
  ZP_FixedAllocator_t fixed_allocator;
  ZP_RegionAllocator_t region_allocator;
  ZP_RefCnt_t* refcount;
};
typedef struct ZP_Context ZP_Context_t;

ZP_Error_t ZP_Context_new(
  ZP_Context_t* context,
  void* fixed_arena,
  void* region_arena,
  void* refcount_arena,
  size_t fixed_size,
  size_t region_size,
  size_t refcount_size
);

ZP_RefCnt_t ZP_Context_refcnt(
  ZP_Context_t* context,
  ZP_Labeled_t* labeled
);

ZP_Error_t ZP_Context_incref(
  ZP_Context_t* context,
  ZP_Labeled_t* labeled
);

ZP_Error_t ZP_Context_incref_newnode(
  ZP_Context_t* context,
  ZP_Labeled_t* labeled,
  ZP_Index_t location,
  bool left_is_ref,
  bool right_is_ref,
  ZP_Index_t label_mask
);

void ZP_Context_decref(
  ZP_Context_t* context,
  ZP_Labeled_t* labeled
);

/***** BufferOwned ***********************************************************/

void ZP_BufferOwned_decref(
  ZP_Context_t* context,
  ZP_Labeled_t* labeled
);

ZP_Error_t ZP_BufferOwned_new(
  ZP_Context_t* context,
  ZP_Labeled_t* labeled,
  ZP_Index_t request_size
);

void ZP_BufferOwned_copy_data(
  ZP_Context_t* context,
  ZP_Labeled_t* labeled,
  const char* data
);

/***** BufferView ************************************************************/

void ZP_BufferView_decref_node(
  ZP_Context_t* context,
  ZP_Labeled_t* labeled
);

void ZP_BufferView_decref(
  ZP_Context_t* context,
  ZP_Labeled_t* labeled
);

ZP_Error_t ZP_BufferView_new(
  ZP_Context_t* context,
  ZP_Labeled_t* labeled,
  ZP_Labeled_t* base,
  ZP_Index_t start,
  ZP_Index_t stop
);

/***** List ******************************************************************/

void ZP_List_decref(
  ZP_Context_t* context,
  ZP_Labeled_t* labeled
);

void ZP_List_new_empty(
  ZP_Labeled_t* list
);

ZP_Error_t ZP_List_push(
  ZP_Context_t* context,
  ZP_Labeled_t* output_list,
  ZP_Labeled_t* input_list,
  ZP_Labeled_t* item
);

ZP_Error_t ZP_List_pop(
  ZP_Context_t* context,
  ZP_Labeled_t* output_list,
  ZP_Labeled_t* output_item,
  ZP_Labeled_t* input_list
);

/***** Symbols ***************************************************************/

/* struct ZP_SymbolSet { */
/*   ZP_Index_t first_pointer; */
/* }; */
/* typedef struct ZP_SymbolSet ZP_SymbolSet_t; */

/* struct ZP_Symbol { */
/*   ZP_Index_t pointer; */
/* }; */
/* typedef struct ZP_Symbol ZP_Symbol_t; */

/* ZP_Error_t ZP_SymbolSet_new( */
/*   ZP_SymbolSet_t* symbol_set */
/* ); */

/* ZP_Error_t ZP_Symbol_new( */
/*   ZP_SymbolSet_t* symbol_set, */
/*   ZP_RegionAllocator_t* region_allocator, */
/*   const char* symbol_data, */
/*   ZP_Index_t symbol_data_size, */
/*   ZP_Symbol_t* symbol */
/* ); */

#endif /* _ZIPPY_DATA_H */
