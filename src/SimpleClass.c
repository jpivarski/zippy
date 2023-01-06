#include <zippy/data.h>
#include <zippy/util.h>

/***** helper (private) functions ********************************************/

/***** public functions ******************************************************/

ZP_Error_t ZP_SimpleClass_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_simple_class,
  ZP_Tagged_t* old_simple_class
) {
  return "not implemented SimpleClass 1";
}

void ZP_SimpleClass_decref(
  ZP_Context_t* context,
  ZP_Tagged_t* simple_class
) {
  printf("not implemented SimpleClass 2");
}

ZP_Error_t ZP_SimpleClass_new(
  ZP_Context_t* context,
  ZP_Tagged_t* simple_class,
  ZP_Tagged_t* member_name_symbols
) {
  ZP_Index_t num_members = ZP_List_size(context, member_name_symbols);

  ZP_RegionPointer_t region_pointer;
  ZP_Error_t error = ZP_Context_new_region(context, &region_pointer, 4 * num_members * sizeof(ZP_Index_t));
  if (error != ZP_NO_ERROR) {
    return error;
  }

  ZP_SlabPointer_t slab_pointer;
  error = ZP_Context_new_slab(context, &slab_pointer);
  if (error != ZP_NO_ERROR) {
    ZP_Context_region_decref(context, &region_pointer);
    return error;
  }

  ZP_Tagged_from_SlabPointer(simple_class, &slab_pointer, ZP_TAG_OBJECT_OTHER);

  ZP_Tagged_t other_type;
  ZP_Tagged_from_enum(&other_type, ZP_ENUM_OTHER_SIMPLE_CLASS);

  ZP_Context_set(context, &slab_pointer, other_type.data, region_pointer.at);

  ZP_Index_t* region_data = (ZP_Index_t*)ZP_Context_region_at(context, &region_pointer);

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
      ZP_Context_free_slab(context, &slab_pointer);
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
  ZP_util_argsort(argsort_index, symbol_identifiers);

  /* the fourth batch is a temporary scratch space for instance constructors */
  ZP_Index_t* scratch_space = &region_data[3 * num_members];

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
  region_pointer.at = ZP_Context_get_right(context, &slab_pointer);

  return ZP_Context_region_size(context, &region_pointer) / (4 * sizeof(ZP_Index_t));
}
