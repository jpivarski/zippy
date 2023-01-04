#include <zippy/data.h>

/***** helper (private) functions ********************************************/

/***** public functions ******************************************************/

ZP_Error_t ZP_List_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_list,
  ZP_Tagged_t* old_list
) {
  return ZP_Context_slab_copy(context, new_list, old_list, true, true);
}

void ZP_List_del(
  ZP_Context_t* context,
  ZP_Tagged_t* list
) {
  /* iterative del through a list, to avoid deep recursion on long lists */

  ZP_Tagged_t this_list;
  ZP_Tagged_copy(&this_list, list);

  ZP_SlabPointer_t pointer;
  ZP_Tagged_to_SlabPointer(&pointer, &this_list);

  ZP_Tagged_t this_item;
  do {
    /* updates this_list (in-place) */
    ZP_Context_get(context, &pointer, &this_list.data, &this_item.data);

    /* decref this_item (whatever it is) */
    ZP_Context_slab_decref(context, &this_item);

    /* free the slab associated with the OLD this_list (pointer) */
    ZP_SlabAllocator_free(&context->slab_allocator, &pointer);

    /* if we're looking at a ZP_TERMINATOR, bail before trying to get its address */
    if (ZP_List_is_empty(&this_list)) {
      break;
    }

    /* point the pointer at the new this_list */
    ZP_Tagged_to_SlabPointer(&pointer, &this_list);

    /* decref it; it's a list object */
    context->refcnt[pointer.at]--;

    /* keep going while this_list is getting del'ed, too */
  } while (context->refcnt[pointer.at] == 0);
}

void ZP_List_new_empty(
  ZP_Tagged_t* list
) {
  list->data = ZP_TERMINATOR | ZP_TAG_OBJECT_LIST;
}

bool ZP_List_is_empty(
  ZP_Tagged_t* list
) {
  return list->data == (ZP_TERMINATOR | ZP_TAG_OBJECT_LIST);
}

ZP_Index_t ZP_List_size(
  ZP_Context_t* context,
  ZP_Tagged_t* list
) {
  ZP_Tagged_t this_list;
  ZP_Tagged_copy(&this_list, list);

  ZP_SlabPointer_t pointer;
  ZP_Index_t size = 0;
  while (!ZP_List_is_empty(&this_list)) {
    ZP_Tagged_to_SlabPointer(&pointer, &this_list);
    this_list.data = ZP_Context_get_left(context, &pointer);
    size++;
  }

  return size;
}

void ZP_List_peek_iterate(
  ZP_Context_t* context,
  ZP_Tagged_t* next_list,
  ZP_Tagged_t* this_item,
  ZP_Tagged_t* current_list
) {
  ZP_SlabPointer_t pointer;
  ZP_Tagged_to_SlabPointer(&pointer, current_list);
  ZP_Context_get(context, &pointer, &next_list->data, &this_item->data);
}

ZP_Error_t ZP_List_push(
  ZP_Context_t* context,
  ZP_Tagged_t* output_list,
  ZP_Tagged_t* input_list,
  ZP_Tagged_t* item
) {
  ZP_Tagged_t input_list_increfed;
  ZP_Error_t error = ZP_Context_slab_incref(context, &input_list_increfed, input_list);
  if (error != ZP_NO_ERROR) {
    return error;
  }

  ZP_Tagged_t item_increfed;
  error = ZP_Context_slab_incref(context, &item_increfed, item);
  if (error != ZP_NO_ERROR) {
    ZP_Context_slab_decref(context, &input_list_increfed);
    return error;
  }

  ZP_SlabPointer_t pointer;
  error = ZP_Context_new_slab(context, &pointer);
  if (error != ZP_NO_ERROR) {
    ZP_Context_slab_decref(context, &input_list_increfed);
    ZP_Context_slab_decref(context, &item_increfed);
    return error;
  }

  ZP_Context_set(context, &pointer, input_list_increfed.data, item_increfed.data);

  ZP_Tagged_from_SlabPointer(output_list, &pointer, ZP_TAG_OBJECT_LIST);

  return ZP_NO_ERROR;
}
