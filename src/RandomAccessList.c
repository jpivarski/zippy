#include <zippy/data.h>

/***** helper (private) functions ********************************************/

/***** public functions ******************************************************/

ZP_Error_t ZP_RandomAccessList_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_random_access_list,
  ZP_Tagged_t* old_random_access_list
) {
  ZP_SlabPointer_t old_pointer;
  old_pointer.at = old_random_access_list->data & ZP_TERMINATOR;

  ZP_Tagged_t object_type;
  ZP_RegionPointer_t region_pointer;
  ZP_SlabAllocator_get(&context->slab_allocator, &old_pointer, &object_type.data, &region_pointer.at);

  ZP_SlabPointer_t new_pointer;
  ZP_Error_t error = ZP_SlabAllocator_alloc(&context->slab_allocator, &new_pointer);
  context->refcnt[new_pointer.at] = 1;
  if (error != ZP_NO_ERROR) {
    return error;
  }

  ZP_Index_t size = ZP_RegionAllocator_size(&context->region_allocator, &region_pointer) / sizeof(ZP_Index_t);
  ZP_Index_t* region_data = (ZP_Index_t*)ZP_RegionAllocator_at(&context->region_allocator, &region_pointer);

  ZP_Tagged_t item;
  ZP_Tagged_t item_increfed;
  ZP_Index_t i;
  bool any_new_refs = false;

  /* attempt to incref all of the items */
  for (i = 0;  i < size;  i++) {
    item.data = region_data[i];
    error = ZP_Context_slab_incref(context, &item_increfed, &item);

    /* but if there's an error, un-incref the ones that have been done so far and bail */
    if (error != ZP_NO_ERROR) {
      ZP_Index_t j;
      for (j = 0;  j < i;  j++) {
        item.data = region_data[j];
        ZP_Context_slab_decref(context, &item);
      }
      ZP_SlabAllocator_free(&context->slab_allocator, &new_pointer);
      context->refcnt[new_pointer.at] = 0;
      return error;
    }

    /* if any would be new references, un-incref them and try again below (rare case) */
    if (item.data != item_increfed.data) {
      ZP_Index_t j;
      for (j = 0;  j < i;  j++) {
        item.data = region_data[j];
        ZP_Context_slab_decref(context, &item);
      }
      ZP_Context_slab_decref(context, &item_increfed);
      any_new_refs = true;
      break;
    }
  }

  if (any_new_refs) {
    /* no item references have been increfed, need a new buffer to put them in */
    error = ZP_Context_new_region(context, &region_pointer, size * sizeof(ZP_Index_t));
    if (error != ZP_NO_ERROR) {
      ZP_SlabAllocator_free(&context->slab_allocator, &new_pointer);
      context->refcnt[new_pointer.at] = 0;
      return error;
    }
    ZP_Index_t* new_region_data = (ZP_Index_t*)ZP_RegionAllocator_at(&context->region_allocator, &region_pointer);
    for (i = 0;  i < size;  i++) {
      item.data = region_data[i];
      error = ZP_Context_slab_incref(context, &item_increfed, &item);
      if (error != ZP_NO_ERROR) {
        ZP_Index_t j;
        for (j = 0;  j < i;  j++) {
          item_increfed.data = new_region_data[j];
          ZP_Context_slab_decref(context, &item_increfed);
        }
        ZP_Context_region_decref(context, &region_pointer);
        ZP_SlabAllocator_free(&context->slab_allocator, &new_pointer);
        context->refcnt[new_pointer.at] = 0;
        return error;
      }
      new_region_data[i] = item_increfed.data;
    }
  }
  else {
    /* all item references have been increfed, can reuse the old buffer */
    error = ZP_Context_region_incref(context, &region_pointer);
    if (error != ZP_NO_ERROR) {
      ZP_Index_t j;
      for (j = 0;  j < size;  j++) {
        item.data = region_data[j];
        ZP_Context_slab_decref(context, &item);
      }
      ZP_SlabAllocator_free(&context->slab_allocator, &new_pointer);
      context->refcnt[new_pointer.at] = 0;
      return error;
    }
  }

  ZP_SlabAllocator_set(&context->slab_allocator, &new_pointer, object_type.data, region_pointer.at);

  new_random_access_list->data = new_pointer.at | ZP_TAG_NODE_OBJECT;

  return ZP_NO_ERROR;
}

void ZP_RandomAccessList_del(
  ZP_Context_t* context,
  ZP_Tagged_t* random_access_list
) {
  ZP_SlabPointer_t slab_pointer;
  slab_pointer.at = random_access_list->data & ZP_TERMINATOR;

  ZP_RegionPointer_t region_pointer;
  region_pointer.at = ZP_SlabAllocator_get_right(&context->slab_allocator, &slab_pointer);

  ZP_Index_t size = ZP_RegionAllocator_size(&context->region_allocator, &region_pointer) / sizeof(ZP_Index_t);
  ZP_Index_t* region_data = (ZP_Index_t*)ZP_RegionAllocator_at(&context->region_allocator, &region_pointer);

  ZP_Tagged_t item;
  ZP_Index_t i;
  for (i = 0;  i < size;  i++) {
    item.data = region_data[i];
    ZP_Context_slab_decref(context, &item);
  }

  ZP_Context_region_decref(context, &region_pointer);

  ZP_SlabAllocator_free(&context->slab_allocator, &slab_pointer);
  context->refcnt[slab_pointer.at] = 0;
}

ZP_Error_t ZP_RandomAccessList_from_List(
  ZP_Context_t* context,
  ZP_Tagged_t* random_access_list,
  ZP_Tagged_t* list
) {
  ZP_Index_t size = ZP_List_size(context, list);

  ZP_RegionPointer_t region_pointer;
  ZP_Error_t error = ZP_Context_new_region(context, &region_pointer, size * sizeof(ZP_Index_t));
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

  random_access_list->data = slab_pointer.at | ZP_TAG_NODE_OBJECT;

  ZP_Tagged_t object_type;
  ZP_Tagged_from_enum(&object_type, ZP_ENUM_OTHER_RANDOM_ACCESS_LIST);

  ZP_SlabAllocator_set(&context->slab_allocator, &slab_pointer, object_type.data, region_pointer.at);

  ZP_Index_t* region_data = (ZP_Index_t*)ZP_RegionAllocator_at(&context->region_allocator, &region_pointer);

  ZP_Tagged_t next_list;
  next_list.data = list->data;

  ZP_Index_t i;
  ZP_Tagged_t this_item;
  ZP_Tagged_t this_item_increfed;
  for (i = 0;  i < size;  i++) {
    ZP_List_peek_reverse_iterate_step(context, &next_list, &this_item, &next_list);

    error = ZP_Context_slab_incref(context, &this_item_increfed, &this_item);
    if (error != ZP_NO_ERROR) {
      ZP_Index_t j;
      for (j = 0;  j < i;  j++) {
        this_item_increfed.data = region_data[size - 1 - j];
        ZP_Context_slab_decref(context, &this_item_increfed);
      }
      ZP_Context_region_decref(context, &region_pointer);
      ZP_SlabAllocator_free(&context->slab_allocator, &slab_pointer);
      context->refcnt[slab_pointer.at] = 0;
      return error;
    }

    region_data[size - 1 - i] = this_item_increfed.data;
  }

  return ZP_NO_ERROR;
}

ZP_Error_t ZP_RandomAccessList_to_List(
  ZP_Context_t* context,
  ZP_Tagged_t* list,
  ZP_Tagged_t* random_access_list
) {
  ZP_SlabPointer_t slab_pointer;
  slab_pointer.at = random_access_list->data & ZP_TERMINATOR;

  ZP_RegionPointer_t region_pointer;
  region_pointer.at = ZP_SlabAllocator_get_right(&context->slab_allocator, &slab_pointer);

  ZP_Index_t size = ZP_RegionAllocator_size(&context->region_allocator, &region_pointer) / sizeof(ZP_Index_t);
  ZP_Index_t* region_data = (ZP_Index_t*)ZP_RegionAllocator_at(&context->region_allocator, &region_pointer);

  ZP_Error_t error;
  ZP_List_new_empty(list);

  ZP_Index_t i;
  ZP_Tagged_t this_item;
  for (i = 0;  i < size;  i++) {
    this_item.data = region_data[size - 1 - i];

    ZP_Tagged_t new_list;
    error = ZP_List_push(context, &new_list, list, &this_item);
    if (error != ZP_NO_ERROR) {
      ZP_List_del(context, list);
      return error;
    }

   /* no longer need a reference to the node that new_list now contains */
    ZP_Context_slab_decref(context, list);

    list->data = new_list.data;
  }

  return ZP_NO_ERROR;
}
