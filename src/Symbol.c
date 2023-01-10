#include <zippy/data.h>

/***** helper (private) functions ********************************************/

/***** public functions ******************************************************/

ZP_Error_t ZP_Symbol_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_symbol,
  ZP_Tagged_t* old_symbol
) {
  ZP_SlabPointer_t old_pointer;
  ZP_Tagged_to_SlabPointer(&old_pointer, old_symbol);

  ZP_Tagged_t tag;
  ZP_RegionPointer_t region_pointer;
  ZP_SlabAllocator_get(&context->slab_allocator, &old_pointer, &tag.data, &region_pointer.at);

  /* As a first pass, see if you can find this Symbol in the symbol_set and just */
  /* increment that (returning it as new_symbol) instead of actually making a new one. */

  ZP_Tagged_t this_list;
  ZP_Tagged_copy(&this_list, &context->symbol_set);
  ZP_Tagged_t next_list;
  ZP_Tagged_t this_symbol;
  while (!ZP_List_is_empty(&this_list)) {
    ZP_List_peek_reverse_iterate_step(context, &next_list, &this_symbol, &this_list);

    ZP_SlabPointer_t this_pointer;
    ZP_Tagged_to_SlabPointer(&this_pointer, &this_symbol);

    ZP_RegionPointer_t this_region_pointer;
    this_region_pointer.at = ZP_SlabAllocator_get_right(&context->slab_allocator, &this_pointer);

    if (region_pointer.at == this_region_pointer.at) {
      if (context->refcnt[this_pointer.at] != ZP_REFCNT_MAX) {
        context->refcnt[this_pointer.at]++;
        new_symbol->data = this_symbol.data;
        return ZP_NO_ERROR;
      }
    }

    this_list.data = next_list.data;
  }

  /* Otherwise (the Symbol exists, but it's at ZP_REFCNT_MAX), make a new Symbol. */

  ZP_SlabPointer_t new_pointer;
  ZP_Error_t error = ZP_SlabAllocator_alloc(&context->slab_allocator, &new_pointer);
  if (error != ZP_NO_ERROR) {
    return error;
  }
  context->refcnt[new_pointer.at] = 1;

  error = ZP_Context_region_incref(context, &region_pointer);
  if (error != ZP_NO_ERROR) {
    ZP_SlabAllocator_free(&context->slab_allocator, &new_pointer);
    context->refcnt[new_pointer.at] = 0;
    return error;
  }

  ZP_SlabAllocator_set(&context->slab_allocator, &new_pointer, tag.data, region_pointer.at);

  ZP_Tagged_from_SlabPointer(new_symbol, &new_pointer, ZP_TAG_NODE_OBJECT);

  ZP_Tagged_t new_list;
  error = ZP_List_push(context, &new_list, &context->symbol_set, new_symbol);
  if (error != ZP_NO_ERROR) {
    ZP_SlabAllocator_free(&context->slab_allocator, &new_pointer);
    context->refcnt[new_pointer.at] = 0;
    ZP_Context_region_decref(context, &region_pointer);
    return error;
  }

  /* "internal" symbol_set list nodes and Symbol references don't count */
  /* only the (single, remaining) reference to the Symbol this function returns */
  ZP_Context_slab_decref(context, new_symbol);
  ZP_Context_slab_decref(context, &context->symbol_set);

  context->symbol_set.data = new_list.data;

  return ZP_NO_ERROR;
}

void ZP_Symbol_del(
  ZP_Context_t* context,
  ZP_Tagged_t* symbol
) {
  ZP_Index_t comparison = symbol->data;

  ZP_SlabPointer_t symbol_pointer;
  ZP_Tagged_to_SlabPointer(&symbol_pointer, symbol);

  ZP_RegionPointer_t region_pointer;
  region_pointer.at = ZP_SlabAllocator_get_right(&context->slab_allocator, &symbol_pointer);

  /* decref the region memory */
  ZP_Context_region_decref(context, &region_pointer);

  /* free the Symbol object */
  ZP_SlabAllocator_free(&context->slab_allocator, &symbol_pointer);
  context->refcnt[symbol_pointer.at] = 0;

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
        ZP_SlabAllocator_set_left(&context->slab_allocator, &prev_pointer, next_list.data);
      }

      /* either way, we need to remove this_list (without calling its del) */
      ZP_SlabPointer_t this_pointer;
      ZP_Tagged_to_SlabPointer(&this_pointer, &this_list);
      ZP_SlabAllocator_free(&context->slab_allocator, &this_pointer);
      context->refcnt[this_pointer.at] = 0;

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
  ZP_Error_t error = ZP_SlabAllocator_alloc(&context->slab_allocator, &pointer);
  if (error != ZP_NO_ERROR) {
    return error;
  }
  context->refcnt[pointer.at] = 1;

  ZP_Tagged_t object_type;
  ZP_Tagged_from_enum(&object_type, ZP_ENUM_OTHER_SYMBOL);

  ZP_SlabAllocator_set(&context->slab_allocator, &pointer, object_type.data, region_pointer->at);

  ZP_Tagged_from_SlabPointer(symbol, &pointer, ZP_TAG_NODE_OBJECT);

  return ZP_NO_ERROR;
}

ZP_Index_t ZP_Symbol_identifier(
  ZP_Context_t* context,
  ZP_Tagged_t* symbol
) {
  ZP_SlabPointer_t pointer;
  ZP_Tagged_to_SlabPointer(&pointer, symbol);
  return ZP_SlabAllocator_get_right(&context->slab_allocator, &pointer);
}
