#include <zippy/data.h>

/***** helper (private) functions ********************************************/

ZP_Index_t ZP_RegionAllocator_actual_size(
  ZP_Index_t size
) {
  ZP_Index_t quot = size / sizeof(ZP_Index_t);
  ZP_Index_t rem = size % sizeof(ZP_Index_t);
  return (quot + (rem != 0) + 2) * sizeof(ZP_Index_t);
}

void ZP_RegionAllocator_free(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer
) {
  /* get the size from the data header (second item) */
  ZP_Index_t* size = (ZP_Index_t*)(region_allocator->arena + pointer->at + sizeof(ZP_Index_t));

  ZP_Index_t actual_size = ZP_RegionAllocator_actual_size(*size);
  ZP_Index_t prev = ZP_END_OF_FREE;
  ZP_Index_t at = region_allocator->first_free;

  /* find a place in the list to insert this, to keep the list sorted */
  /* note: as long as pointer is not already free, at != pointer */
  while (at != ZP_END_OF_FREE  &&  at < pointer->at) {
    prev = at;
    at = *(ZP_Index_t*)(region_allocator->arena + at + sizeof(ZP_Index_t));
  }

  /* insert the new free region right before the pointer, so the list stays sorted */
  /* this might not be used because the previous region might be extended over this one */
  /* but it doesn't hurt to write what WOULD go here, anyway */
  *(ZP_Index_t*)(region_allocator->arena + pointer->at) = actual_size;
  *(ZP_Index_t*)(region_allocator->arena + pointer->at + sizeof(ZP_Index_t)) = at;

  /* link the previous node (if any) to this new one */
  if (prev == ZP_END_OF_FREE) {
    if (pointer->at + actual_size == at) {
      /* immediately after this block is the first free region, so merge them */
      ZP_Index_t next_size = *(ZP_Index_t*)(region_allocator->arena + at);
      ZP_Index_t next_at = *(ZP_Index_t*)(region_allocator->arena + at + sizeof(ZP_Index_t));
      *(ZP_Index_t*)(region_allocator->arena + pointer->at) += next_size;
      *(ZP_Index_t*)(region_allocator->arena + pointer->at + sizeof(ZP_Index_t)) = next_at;
    }
    region_allocator->first_free = pointer->at;
  }
  else {
    ZP_Index_t prev_size = *(ZP_Index_t*)(region_allocator->arena + prev);
    if (prev + prev_size == pointer->at) {
      /* previous region is contiguous with this one, so enlarge it to include this */
      *(ZP_Index_t*)(region_allocator->arena + prev) += actual_size;

      if (at != ZP_END_OF_FREE  &&  prev + prev_size + actual_size == at) {
        /* previous region + this one is also contiguous with the next */
        /* so keep enlarging, and prev's next pointer takes over from at's */
        ZP_Index_t next_size = *(ZP_Index_t*)(region_allocator->arena + at);
        ZP_Index_t next_at = *(ZP_Index_t*)(region_allocator->arena + at + sizeof(ZP_Index_t));
        *(ZP_Index_t*)(region_allocator->arena + prev) += next_size;
        *(ZP_Index_t*)(region_allocator->arena + prev + sizeof(ZP_Index_t)) = next_at;
      }
    }
    else {
      /* there's non-free data between the regions; make a link in the linked list */
      *(ZP_Index_t*)(region_allocator->arena + prev + sizeof(ZP_Index_t)) = pointer->at;

      if (at != ZP_END_OF_FREE  &&  pointer->at + actual_size == at) {
        /* this region is contiguous with the next one, so enlarge this one, */
        /* taking over the next one */
        ZP_Index_t next_size = *(ZP_Index_t*)(region_allocator->arena + at);
        ZP_Index_t next_at = *(ZP_Index_t*)(region_allocator->arena + at + sizeof(ZP_Index_t));
        *(ZP_Index_t*)(region_allocator->arena + pointer->at) += next_size;
        *(ZP_Index_t*)(region_allocator->arena + pointer->at + sizeof(ZP_Index_t)) = next_at;
      }
    }
  }
}

/***** public functions ******************************************************/

void ZP_RegionAllocator_new(
  ZP_RegionAllocator_t* region_allocator,
  void* arena,
  size_t arena_size
) {
  region_allocator->arena = arena;
  region_allocator->arena_size = arena_size;
  region_allocator->first_free = 0;
  *(ZP_Index_t*)region_allocator->arena = arena_size;
  *(ZP_Index_t*)(region_allocator->arena + sizeof(ZP_Index_t)) = ZP_END_OF_FREE;
}

bool ZP_RegionAllocator_is_full(
  ZP_RegionAllocator_t* region_allocator
) {
  return region_allocator->first_free == ZP_END_OF_FREE;
}

void* ZP_RegionAllocator_at(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer
) {
  /* access data just beyond the reference count */
  return (void*)(region_allocator->arena + pointer->at + sizeof(ZP_Index_t));
}

ZP_Error_t ZP_RegionAllocator_alloc(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer,
  ZP_Index_t size
) {
  ZP_Index_t actual_size = ZP_RegionAllocator_actual_size(size);
  ZP_Index_t prev = ZP_END_OF_FREE;
  ZP_Index_t at = region_allocator->first_free;

  while (true) {
    if (at == ZP_END_OF_FREE) {
      return "not enough memory in RegionAllocator to allocate region";
    }

    ZP_Index_t next_free = *(ZP_Index_t*)(region_allocator->arena + at + sizeof(ZP_Index_t));
    ZP_Index_t this_free_size = *(ZP_Index_t*)(region_allocator->arena + at);

    if (actual_size < this_free_size) {
      /* split the region */
      pointer->at = at;
      at += actual_size;
      this_free_size -= actual_size;
      *(ZP_Index_t*)(region_allocator->arena + at) = this_free_size;
      *(ZP_Index_t*)(region_allocator->arena + at + sizeof(ZP_Index_t)) = next_free;
      break;
    }

    else if (actual_size == this_free_size) {
      /* take the entire region */
      pointer->at = at;
      at = next_free;
      break;
    }

    else {
      /* try the next free region */
      prev = at;
      at = next_free;
    }
  }

  if (prev == ZP_END_OF_FREE) {
    /* update linked list head to point here */
    region_allocator->first_free = at;
  }
  else {
    /* update previous linked list node to point here */
    *(ZP_Index_t*)(region_allocator->arena + prev + sizeof(ZP_Index_t)) = at;
  }

  /* data header: (1) reference count, (2) number of meaningful bytes */
  *(ZP_Index_t*)(region_allocator->arena + pointer->at) = 1;
  *(ZP_Index_t*)(region_allocator->arena + pointer->at + sizeof(ZP_Index_t)) = size;

  return ZP_NO_ERROR;
}

ZP_Error_t ZP_RegionAllocator_incref(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer
) {
  /* get the reference count from the data header (first item) */
  ZP_Index_t* refcnt = (ZP_Index_t*)(region_allocator->arena + pointer->at);

  if (*refcnt == ZP_END_OF_FREE) {
    return "mutable data buffers cannot be referenced more than ZP_END_OF_FREE times";
  }

  (*refcnt)++;

  return ZP_NO_ERROR;
}

void ZP_RegionAllocator_decref(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RegionPointer_t* pointer
) {
  /* get the reference count from the data header (first item) */
  ZP_Index_t* refcnt = (ZP_Index_t*)(region_allocator->arena + pointer->at);

  (*refcnt)--;

  if (*refcnt == 0) {
    ZP_RegionAllocator_free(region_allocator, pointer);
  }
}
