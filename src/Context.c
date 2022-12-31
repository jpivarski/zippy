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
  return "not implemented 3";
}

ZP_Index_t ZP_Context_get_left(
  ZP_Context_t* slab_allocator,
  ZP_SlabPointer_t* pointer
) {
  return 0;
}

ZP_Index_t ZP_Context_get_right(
  ZP_Context_t* slab_allocator,
  ZP_SlabPointer_t* pointer
) {
  return 0;
}

void ZP_Context_get(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t* left,
  ZP_Index_t* right
) {

}

void ZP_Context_set(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t left,
  ZP_Index_t right
) {

}

ZP_Error_t ZP_Context_slab_incref(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged
) {
  return "not implemented 4";
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
