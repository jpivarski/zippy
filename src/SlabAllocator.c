#include <zippy/data.h>

/***** helper (private) functions ********************************************/

/***** public functions ******************************************************/

void ZP_Tagged_to_SlabPointer(
  ZP_SlabPointer_t* pointer,
  ZP_Tagged_t* tagged
) {
  pointer->at = tagged->data & ZP_TERMINATOR;
}

void ZP_Tagged_from_SlabPointer(
  ZP_Tagged_t* tagged,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t tag
) {
  tagged->data = pointer->at | tag;
}

void ZP_SlabAllocator_new(
  ZP_SlabAllocator_t* slab_allocator,
  void* arena,
  size_t arena_size
) {
  slab_allocator->arena = (ZP_Index_t*)arena;
  slab_allocator->num_slabs = arena_size / (2 * sizeof(ZP_Index_t));
  slab_allocator->first_free = ZP_END_OF_FREE;
  slab_allocator->bump_free = 0;
}

bool ZP_SlabAllocator_is_full(
  ZP_SlabAllocator_t* slab_allocator
) {
  return (
    slab_allocator->first_free == ZP_END_OF_FREE  &&
    slab_allocator->bump_free == slab_allocator->num_slabs
  );
}

ZP_Error_t ZP_SlabAllocator_alloc(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer
) {
  if (slab_allocator->first_free == ZP_END_OF_FREE) {
    if (slab_allocator->bump_free == slab_allocator->num_slabs) {
      return "not enough memory in SlabAllocator to allocate object";
    }
    else {
      pointer->at = slab_allocator->bump_free;
      slab_allocator->bump_free++;
    }
  }
  else {
    pointer->at = slab_allocator->first_free;
    slab_allocator->first_free = slab_allocator->arena[2 * slab_allocator->first_free];
  }

  return ZP_NO_ERROR;
}

void ZP_SlabAllocator_free(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer
) {
  slab_allocator->arena[2 * pointer->at] = slab_allocator->first_free;
  slab_allocator->first_free = pointer->at;
}

ZP_Index_t ZP_SlabAllocator_get_left(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer
) {
  return slab_allocator->arena[2 * pointer->at];
}

ZP_Index_t ZP_SlabAllocator_get_right(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer
) {
  return slab_allocator->arena[2 * pointer->at + 1];
}

void ZP_SlabAllocator_get(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t* left,
  ZP_Index_t* right
) {
  *left = slab_allocator->arena[2 * pointer->at];
  *right = slab_allocator->arena[2 * pointer->at + 1];
}

void ZP_SlabAllocator_set(
  ZP_SlabAllocator_t* slab_allocator,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t left,
  ZP_Index_t right
) {
  slab_allocator->arena[2 * pointer->at] = left;
  slab_allocator->arena[2 * pointer->at + 1] = right;
}
