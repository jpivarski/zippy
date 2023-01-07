#ifndef ZIPPY_DATA_H
#define ZIPPY_DATA_H

#include <zippy/defs.h>

/***** Tagged ****************************************************************/

struct ZP_Tagged {
  ZP_Index_t data;
};
typedef struct ZP_Tagged ZP_Tagged_t;

void ZP_Tagged_copy(
  ZP_Tagged_t* output,
  ZP_Tagged_t* input
);

bool ZP_Tagged_is_float(
  ZP_Tagged_t* tagged
);

bool ZP_Tagged_is_int(
  ZP_Tagged_t* tagged
);

bool ZP_Tagged_is_refcntable(
  ZP_Tagged_t* tagged
);

ZP_Index_t ZP_Tagged_to_enum(
  ZP_Tagged_t* tagged
);

void ZP_Tagged_from_enum(
  ZP_Tagged_t* tagged,
  ZP_Index_t enum_value
);

ZP_Float_t ZP_Tagged_to_float(
  ZP_Tagged_t* tagged
);

void ZP_Tagged_from_float(
  ZP_Tagged_t* tagged,
  ZP_Float_t* data
);

bool ZP_Tagged_int_is_overflow(
  ZP_Int_t data
);

ZP_Int_t ZP_Tagged_to_int(
  ZP_Tagged_t* tagged
);

void ZP_Tagged_from_int(
  ZP_Tagged_t* tagged,
  ZP_Int_t* data
);

/***** SlabAllocator *********************************************************/

struct ZP_SlabAllocator {
  ZP_Index_t* arena;
  ZP_Index_t num_slabs;
  ZP_Index_t first_free;
  ZP_Index_t bump_free;
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

ZP_Index_t ZP_SlabAllocator_get_right(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer
);

void ZP_SlabAllocator_set_left(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t left
);

void ZP_SlabAllocator_set_right(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t right
);

void ZP_SlabAllocator_get(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t* left,
  ZP_Index_t* right
);

void ZP_SlabAllocator_set(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t left,
  ZP_Index_t right
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

ZP_Index_t ZP_RegionAllocator_refcnt(
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

ZP_Index_t ZP_RegionAllocator_size(
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

ZP_Error_t ZP_Context_get_symbol(
  ZP_Context_t* context,
  ZP_Tagged_t* symbol,
  const char* symbol_data,
  ZP_Index_t symbol_size
);

ZP_Error_t ZP_Context_new_slab(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer
);

void ZP_Context_free_slab(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer
);

ZP_Index_t ZP_Context_get_left(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer
);

ZP_Index_t ZP_Context_get_right(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer
);

void ZP_Context_set_left(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t left
);

void ZP_Context_set_right(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t right
);

void ZP_Context_get(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t* left,
  ZP_Index_t* right
);

void ZP_Context_set(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t left,
  ZP_Index_t right
);

ZP_Error_t ZP_Context_slab_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_tagged,
  ZP_Tagged_t* old_tagged,
  bool incref_left,
  bool incref_right
);

ZP_RefCnt_t ZP_Context_slab_refcnt(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
);

ZP_Error_t ZP_Context_slab_incref(
  ZP_Context_t* context,
  ZP_Tagged_t* new_tagged,
  ZP_Tagged_t* old_tagged
);

void ZP_Context_slab_decref(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged
);

ZP_RefCnt_t ZP_Context_internal_refcnt(
  ZP_Context_t* context,
  ZP_Index_t internal
);

void ZP_Context_internal_incref(
  ZP_Context_t* context,
  ZP_Index_t internal
);

void ZP_Context_internal_decref(
  ZP_Context_t* context,
  ZP_Index_t internal
);

ZP_Error_t ZP_Context_new_region(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer,
  ZP_Index_t size
);

ZP_Index_t ZP_Context_region_refcnt(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
);

ZP_Error_t ZP_Context_region_incref(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
);

void ZP_Context_region_decref(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
);

ZP_Index_t ZP_Context_region_size(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
);

void* ZP_Context_region_at(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
);

void ZP_Context_region_copy_data(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer,
  const char* data
);

void ZP_Context_object_type(
  ZP_Context_t* context,
  ZP_Tagged_t* object_type,
  ZP_Tagged_t* object
);

/***** List ******************************************************************/

ZP_Error_t ZP_List_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_list,
  ZP_Tagged_t* old_list
);

void ZP_List_del(
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

void ZP_List_peek_reverse_iterate_step(
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

/***** Symbol ****************************************************************/

ZP_Error_t ZP_Symbol_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_symbol,
  ZP_Tagged_t* old_symbol
);

void ZP_Symbol_del(
  ZP_Context_t* context,
  ZP_Tagged_t* symbol
);

ZP_Error_t ZP_Symbol_new(
  ZP_Context_t* context,
  ZP_Tagged_t* symbol,
  ZP_RegionPointer_t* region_pointer
);

ZP_Index_t ZP_Symbol_identifier(
  ZP_Context_t* context,
  ZP_Tagged_t* symbol
);

/***** RandomAccessList ******************************************************/

ZP_Error_t ZP_RandomAccessList_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_random_access_list,
  ZP_Tagged_t* old_random_access_list
);

void ZP_RandomAccessList_del(
  ZP_Context_t* context,
  ZP_Tagged_t* random_access_list
);

ZP_Error_t ZP_RandomAccessList_from_List(
  ZP_Context_t* context,
  ZP_Tagged_t* random_access_list,
  ZP_Tagged_t* list
);

ZP_Error_t ZP_RandomAccessList_to_List(
  ZP_Context_t* context,
  ZP_Tagged_t* list,
  ZP_Tagged_t* random_access_list
);

/***** SimpleClass ***********************************************************/

ZP_Error_t ZP_SimpleClass_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_simple_class,
  ZP_Tagged_t* old_simple_class
);

void ZP_SimpleClass_del(
  ZP_Context_t* context,
  ZP_Tagged_t* simple_class
);

ZP_Error_t ZP_SimpleClass_new(
  ZP_Context_t* context,
  ZP_Tagged_t* simple_class,
  ZP_Tagged_t* member_name_symbols
);

ZP_Index_t ZP_SimpleClass_num_members(
  ZP_Context_t* context,
  ZP_Tagged_t* simple_class
);

ZP_Error_t ZP_SimpleClass_load_members(
  ZP_Context_t* context,
  ZP_Index_t* num_members,
  ZP_Index_t** scratch_space,
  ZP_Index_t** sorted_data,
  ZP_Tagged_t* simple_class,
  ZP_Tagged_t* members
);

void ZP_SimpleClass_symbol_identifiers(
  ZP_Context_t* context,
  ZP_Index_t* num_members,
  ZP_Index_t** symbol_identifiers,
  ZP_Tagged_t* simple_class
);

/***** SimpleClass_instance **************************************************/

ZP_Error_t ZP_SimpleClass_instance_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_instance,
  ZP_Tagged_t* old_instance
);

void ZP_SimpleClass_instance_del(
  ZP_Context_t* context,
  ZP_Tagged_t* instance
);

ZP_Error_t ZP_SimpleClass_instance_new(
  ZP_Context_t* context,
  ZP_Tagged_t* instance,
  ZP_Tagged_t* simple_class,
  ZP_Tagged_t* members
);

void ZP_SimpleClass_instance_get_member(
  ZP_Context_t* context,
  ZP_Tagged_t* attribute,
  ZP_Tagged_t* instance,
  ZP_Tagged_t* symbol
);

#endif /* ZIPPY_DATA_H */
