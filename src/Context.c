#include <zippy/data.h>

/***** helper (private) functions ********************************************/

/***** public functions ******************************************************/

ZP_Error_t ZP_Context_new(
  ZP_Context_t* context,
  void* slab_arena,
  void* region_arena,
  void* refcnt_arena,
  size_t slab_size,
  size_t region_size,
  size_t refcnt_size
) {
  if (slab_size < 2 * sizeof(ZP_Index_t)) {
    return "slab_size must be at least 2 * sizeof(ZP_Index_t)";
  }
  if (slab_size > ZP_TERMINATOR) {
    return "slab_size must be at most ZP_TERMINATOR";
  }
  if (slab_size % (2 * sizeof(ZP_Index_t)) != 0) {
    return "slab_size must be a multiple of 2 * sizeof(ZP_Index_t)";
  }

  if (region_size < 2 * sizeof(ZP_Index_t)) {
    return "region_size must be at least 2 * sizeof(ZP_Index_t)";
  }
  if (region_size % (2 * sizeof(ZP_Index_t)) != 0) {
    return "region_size must be a multiple of 2 * sizeof(ZP_Index_t)";
  }

  if (slab_size * sizeof(ZP_RefCnt_t) != refcnt_size * 2 * sizeof(ZP_Index_t)) {
    return "slab_size * sizeof(ZP_RefCnt_t) must be equal to refcnt_size * 2 * sizeof(ZP_Index_t)";
  }

  ZP_SlabAllocator_new(&context->slab_allocator, slab_arena, slab_size);
  ZP_RegionAllocator_new(&context->region_allocator, region_arena, region_size);

  context->refcnt = (ZP_RefCnt_t*)refcnt_arena;

  ZP_Index_t i;
  for (i = 0;  i < refcnt_size;  i++) {
    context->refcnt[i] = 0;
  }

  /* FIXME: initialize symbol_set */

  return ZP_NO_ERROR;
}

ZP_Error_t ZP_Context_new_or_get_symbol(
  ZP_Context_t* context,
  ZP_RegionPointer_t pointer,
  const char* symbol_data,
  ZP_Index_t symbol_size
) {
  return "not implemented 2";
}

ZP_Error_t ZP_Context_new_slab(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer
) {
  ZP_Error_t error = ZP_SlabAllocator_alloc(&context->slab_allocator, pointer);
  if (error != ZP_NO_ERROR) {
    return error;
  }
  context->refcnt[pointer->at] = 1;
  return ZP_NO_ERROR;
}

ZP_Index_t ZP_Context_get_left(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer
) {
  return ZP_SlabAllocator_get_left(&context->slab_allocator, pointer);
}

ZP_Index_t ZP_Context_get_right(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer
) {
  return ZP_SlabAllocator_get_right(&context->slab_allocator, pointer);
}

void ZP_Context_get(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t* left,
  ZP_Index_t* right
) {
  ZP_SlabAllocator_get(&context->slab_allocator, pointer, left, right);
}

void ZP_Context_set(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t left,
  ZP_Index_t right
) {
  ZP_SlabAllocator_set(&context->slab_allocator, pointer, left, right);
}

ZP_Error_t ZP_Context_slab_incref(
  ZP_Context_t* context,
  ZP_Tagged_t* new_tagged,
  ZP_Tagged_t* old_tagged
) {
  ZP_Index_t pointer;
  switch (old_tagged->data & ZP_TAG_OBJECT_MASK) {
  case ZP_TAG_OBJECT_LIST:
    pointer = old_tagged->data & (~ZP_TAG_OBJECT_MASK);
    if (pointer != ZP_TERMINATOR) {
      if (context->refcnt[pointer] != ZP_REFCNT_MAX) {
        context->refcnt[pointer]++;
      }
      else {
        ZP_List_copy(context, new_tagged, old_tagged);
      }
    }
    break;

  case ZP_TAG_OBJECT_INSTANCE:
    pointer = old_tagged->data & (~ZP_TAG_OBJECT_MASK);
    /* FIXME */
    break;

  case ZP_TAG_OBJECT_OTHER:
    pointer = old_tagged->data & (~ZP_TAG_OBJECT_MASK);
    /* FIXME */
    break;
  }

  new_tagged->data = old_tagged->data;
  return ZP_NO_ERROR;
}

ZP_Error_t ZP_Context_slab_incref_left(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer
) {
  return "not implemented 5";
}

ZP_Error_t ZP_Context_slab_incref_right(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer
) {
  return "not implemented 6";
}

ZP_Error_t ZP_Context_slab_incref_both(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer
) {
  return "not implemented 7";
}

void ZP_Context_slab_decref(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged
) {
  ZP_Index_t pointer;
  switch (tagged->data & ZP_TAG_OBJECT_MASK) {
  case ZP_TAG_OBJECT_LIST:
    pointer = tagged->data & (~ZP_TAG_OBJECT_MASK);
    if (pointer != ZP_TERMINATOR) {
      context->refcnt[pointer]--;
      if (context->refcnt[pointer] == 0) {
        ZP_List_del(context, tagged);
      }
    }
    break;

  case ZP_TAG_OBJECT_INSTANCE:
    pointer = tagged->data & (~ZP_TAG_OBJECT_MASK);
    /* FIXME */
    break;

  case ZP_TAG_OBJECT_OTHER:
    pointer = tagged->data & (~ZP_TAG_OBJECT_MASK);
    /* FIXME */
    break;
  }
}

ZP_Error_t ZP_Context_new_region(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer,
  ZP_Index_t size
) {
  return "not implemented 8";
}

void ZP_Context_copy_data(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer,
  const char* data
) {

}

ZP_Error_t ZP_Context_region_incref(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
) {
  return "not implemented 9";
}

void ZP_Context_region_decref(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
) {

}
