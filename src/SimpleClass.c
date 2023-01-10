#include <zippy/data.h>
#include <zippy/util.h>

/***** helper (private) functions ********************************************/

/***** public functions ******************************************************/

ZP_Error_t ZP_SimpleClass_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_simple_class,
  ZP_Tagged_t* old_simple_class
) {
  ZP_SlabPointer_t old_pointer;
  ZP_Tagged_to_SlabPointer(&old_pointer, old_simple_class);

  ZP_SlabPointer_t new_pointer;
  ZP_Error_t error = ZP_SlabAllocator_alloc(&context->slab_allocator, &new_pointer);
  if (error != ZP_NO_ERROR) {
    return error;
  }
  context->refcnt[new_pointer.at] = 1;

  ZP_Tagged_t object_type;
  ZP_RegionPointer_t region_pointer;
  ZP_SlabAllocator_get(&context->slab_allocator, &old_pointer, &object_type.data, &region_pointer.at);

  error = ZP_Context_region_incref(context, &region_pointer);
  if (error != ZP_NO_ERROR) {
    ZP_SlabAllocator_free(&context->slab_allocator, &new_pointer);
    context->refcnt[new_pointer.at] = 0;
    return error;
  }

  ZP_SlabAllocator_set(&context->slab_allocator, &new_pointer, object_type.data, region_pointer.at);

  ZP_Tagged_from_SlabPointer(new_simple_class, &new_pointer, ZP_TAG_NODE_OBJECT);

  return ZP_NO_ERROR;
}

void ZP_SimpleClass_del(
  ZP_Context_t* context,
  ZP_Tagged_t* simple_class
) {
  ZP_SlabPointer_t pointer;
  ZP_Tagged_to_SlabPointer(&pointer, simple_class);

  ZP_RegionPointer_t region_pointer;
  region_pointer.at = ZP_SlabAllocator_get_right(&context->slab_allocator, &pointer);

  ZP_Index_t refcnt = ZP_Context_region_refcnt(context, &region_pointer);
  if (refcnt == 1) {
    ZP_Index_t num_members = ZP_RegionAllocator_size(&context->region_allocator, &region_pointer) / (5 * sizeof(ZP_Index_t));
    ZP_Index_t* region_data = (ZP_Index_t*)ZP_RegionAllocator_at(&context->region_allocator, &region_pointer);
    ZP_Tagged_t symbol;
    ZP_Index_t i;
    for (i = 0;  i < num_members;  i++) {
      symbol.data = region_data[i];
      ZP_Context_slab_decref(context, &symbol);
    }
  }

  ZP_Context_region_decref(context, &region_pointer);

  ZP_SlabAllocator_free(&context->slab_allocator, &pointer);
  context->refcnt[pointer.at] = 0;
}

ZP_Error_t ZP_SimpleClass_new(
  ZP_Context_t* context,
  ZP_Tagged_t* simple_class,
  ZP_Tagged_t* member_name_symbols
) {
  ZP_Index_t num_members = ZP_List_size(context, member_name_symbols);

  ZP_RegionPointer_t region_pointer;
  ZP_Error_t error = ZP_Context_new_region(context, &region_pointer, 5 * num_members * sizeof(ZP_Index_t));
  if (error != ZP_NO_ERROR) {
    return error;
  }

  ZP_SlabPointer_t slab_pointer;
  error = ZP_SlabAllocator_alloc(&context->slab_allocator, &slab_pointer);
  if (error != ZP_NO_ERROR) {
    ZP_Context_region_decref(context, &region_pointer);
    return error;
  }
  context->refcnt[slab_pointer.at] = 1;

  ZP_Tagged_from_SlabPointer(simple_class, &slab_pointer, ZP_TAG_NODE_OBJECT);

  ZP_Tagged_t object_type;
  ZP_Tagged_from_enum(&object_type, ZP_ENUM_OTHER_SIMPLE_CLASS);

  ZP_SlabAllocator_set(&context->slab_allocator, &slab_pointer, object_type.data, region_pointer.at);

  ZP_Index_t* region_data = (ZP_Index_t*)ZP_RegionAllocator_at(&context->region_allocator, &region_pointer);

  ZP_Tagged_t next_list;
  ZP_Tagged_copy(&next_list, member_name_symbols);

  /* the first batch of data are the symbol references to decrement in del */
  ZP_Index_t i;
  ZP_Tagged_t this_symbol;
  ZP_Tagged_t this_symbol_increfed;
  for (i = 0;  i < num_members;  i++) {
    ZP_List_peek_reverse_iterate_step(context, &next_list, &this_symbol, &next_list);

    error = ZP_Context_slab_incref(context, &this_symbol_increfed, &this_symbol);
    if (error != ZP_NO_ERROR) {
      ZP_Index_t j;
      for (j = 0;  j < i;  j++) {
        this_symbol_increfed.data = region_data[num_members - 1 - j];
        ZP_Context_slab_decref(context, &this_symbol_increfed);
      }
      ZP_Context_region_decref(context, &region_pointer);
      ZP_SlabAllocator_free(&context->slab_allocator, &slab_pointer);
      context->refcnt[slab_pointer.at] = 0;
      return error;
    }

    region_data[num_members - 1 - i] = this_symbol_increfed.data;
  }

  ZP_Index_t* symbol_identifiers = &region_data[num_members];

  /* the second batch are the symbol identifiers (their RegionPointers) */
  for (i = 0;  i < num_members;  i++) {
    this_symbol_increfed.data = region_data[i];
    symbol_identifiers[i] = ZP_Symbol_identifier(context, &this_symbol_increfed);
  }

  /* the third batch has an argsort array, to sort arguments of an instance constructor */
  ZP_Index_t* argsort_index = &region_data[2 * num_members];

  /* the fourth batch is a temporary scratch space for instance constructors */
  ZP_Index_t* scratch_space = &region_data[3 * num_members];

  ZP_util_argsort(scratch_space, argsort_index, symbol_identifiers, num_members);

  /* the second batch must be sorted according to that argsort array (use scratch space) */
  for (i = 0;  i < num_members;  i++) {
    scratch_space[i] = symbol_identifiers[argsort_index[i]];
  }
  for (i = 0;  i < num_members;  i++) {
    symbol_identifiers[i] = scratch_space[i];
  }

  return ZP_NO_ERROR;
}

ZP_Index_t ZP_SimpleClass_num_members(
  ZP_Context_t* context,
  ZP_Tagged_t* simple_class
) {
  ZP_SlabPointer_t slab_pointer;
  ZP_Tagged_to_SlabPointer(&slab_pointer, simple_class);

  ZP_RegionPointer_t region_pointer;
  region_pointer.at = ZP_SlabAllocator_get_right(&context->slab_allocator, &slab_pointer);

  return ZP_RegionAllocator_size(&context->region_allocator, &region_pointer) / (5 * sizeof(ZP_Index_t));
}

ZP_Error_t ZP_SimpleClass_load_members(
  ZP_Context_t* context,
  ZP_Index_t* num_members,
  ZP_Index_t** scratch_space,
  ZP_Index_t** sorted_data,
  ZP_Tagged_t* simple_class,
  ZP_Tagged_t* members
) {
  ZP_SlabPointer_t slab_pointer;
  ZP_Tagged_to_SlabPointer(&slab_pointer, simple_class);

  ZP_RegionPointer_t region_pointer;
  region_pointer.at = ZP_SlabAllocator_get_right(&context->slab_allocator, &slab_pointer);

  *num_members = ZP_RegionAllocator_size(&context->region_allocator, &region_pointer) / (5 * sizeof(ZP_Index_t));

  ZP_Index_t* region_data = (ZP_Index_t*)ZP_RegionAllocator_at(&context->region_allocator, &region_pointer);
  ZP_Index_t* argsort_index = &region_data[2 * (*num_members)];
  *scratch_space            = &region_data[3 * (*num_members)];
  *sorted_data              = &region_data[4 * (*num_members)];

  ZP_Tagged_t next_list;
  ZP_Tagged_copy(&next_list, members);

  ZP_Error_t error;
  ZP_Index_t i;
  ZP_Tagged_t this_member;
  ZP_Tagged_t this_member_increfed;
  for (i = 0;  i < (*num_members);  i++) {
    ZP_List_peek_reverse_iterate_step(context, &next_list, &this_member, &next_list);

    error = ZP_Context_slab_incref(context, &this_member_increfed, &this_member);
    if (error != ZP_NO_ERROR) {
      ZP_Index_t j;
      for (j = 0;  j < i;  j++) {
        this_member_increfed.data = (*scratch_space)[(*num_members) - 1 - j];
        ZP_Context_slab_decref(context, &this_member_increfed);
      }
      return error;
    }

    (*scratch_space)[(*num_members) - 1 - i] = this_member_increfed.data;
  }

  /* the member values need to be sorted by symbol identifiers for later retrieval */
  for (i = 0;  i < (*num_members);  i++) {
    (*sorted_data)[i] = (*scratch_space)[argsort_index[i]];
  }

  /* fill the scratch_space with ZP_TERMINATOR so that it can be used for pointers */
  for (i = 0;  i < (*num_members);  i++) {
    (*scratch_space)[i] = ZP_TERMINATOR;
  }

  return ZP_NO_ERROR;
}

void ZP_SimpleClass_symbol_identifiers(
  ZP_Context_t* context,
  ZP_Index_t* num_members,
  ZP_Index_t** symbol_identifiers,
  ZP_Tagged_t* simple_class
) {
  ZP_SlabPointer_t slab_pointer;
  ZP_Tagged_to_SlabPointer(&slab_pointer, simple_class);

  ZP_RegionPointer_t region_pointer;
  region_pointer.at = ZP_SlabAllocator_get_right(&context->slab_allocator, &slab_pointer);

  *num_members = ZP_RegionAllocator_size(&context->region_allocator, &region_pointer) / (5 * sizeof(ZP_Index_t));

  ZP_Index_t* region_data = (ZP_Index_t*)ZP_RegionAllocator_at(&context->region_allocator, &region_pointer);

  *symbol_identifiers = &region_data[(*num_members)];
}
