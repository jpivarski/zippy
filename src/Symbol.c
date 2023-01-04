#include <zippy/data.h>

/***** helper (private) functions ********************************************/

/***** public functions ******************************************************/

ZP_Error_t ZP_Symbol_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_symbol,
  ZP_Tagged_t* old_symbol
) {
  return "not implemented Symbol 1";
}

void ZP_Symbol_del(
  ZP_Context_t* context,
  ZP_Tagged_t* symbol
) {
  ZP_Index_t comparison = symbol->data;

  ZP_SlabPointer_t symbol_pointer;
  ZP_Tagged_to_SlabPointer(&symbol_pointer, symbol);

  ZP_RegionPointer_t region_pointer;
  region_pointer.at = ZP_Context_get_right(context, &symbol_pointer);

  /* decref the region memory */
  ZP_Context_region_decref(context, &region_pointer);

  /* free the Symbol object */
  ZP_Context_free_slab(context, &symbol_pointer);

  ZP_Tagged_t prev_list;
  ZP_List_new_empty(&prev_list);
  ZP_Tagged_t this_list;
  ZP_Tagged_copy(&this_list, &context->symbol_set);

  ZP_Tagged_t next_list;
  ZP_Tagged_t this_symbol;
  while (!ZP_List_is_empty(&this_list)) {
    ZP_List_peek_reverse_iterate_step(context, &next_list, &this_symbol, &this_list);

    if (comparison == this_symbol.data) {
      if (ZP_List_is_empty(&prev_list)) {
        /* we will be removing the first node in the list */
        context->symbol_set = next_list;
      }
      else {
        /* there are nodes before this node, which we will be removing */
        ZP_SlabPointer_t prev_pointer;
        ZP_Tagged_to_SlabPointer(&prev_pointer, &prev_list);
        ZP_Context_set_left(context, &prev_pointer, next_list.data);
      }

      /* either way, we need to remove this_list (without calling its del) */
      ZP_SlabPointer_t this_pointer;
      ZP_Tagged_to_SlabPointer(&this_pointer, &this_list);
      ZP_Context_free_slab(context, &this_pointer);

      return;
    }

    prev_list.data = this_list.data;
    this_list.data = next_list.data;
  }
}

ZP_Error_t ZP_Symbol_new(
  ZP_Context_t* context,
  ZP_Tagged_t* symbol,
  ZP_RegionPointer_t* region_pointer
) {
  ZP_SlabPointer_t pointer;
  ZP_Error_t error = ZP_Context_new_slab(context, &pointer);
  if (error != ZP_NO_ERROR) {
    return error;
  }

  ZP_Tagged_t other_type;
  ZP_Tagged_from_enum(&other_type, ZP_ENUM_OTHER_SYMBOL);

  ZP_Context_set(context, &pointer, other_type.data, region_pointer->at);

  ZP_Tagged_from_SlabPointer(symbol, &pointer, ZP_TAG_OBJECT_OTHER);

  return ZP_NO_ERROR;
}
