#include <stdio.h>  /* temporary! */

#include <zippy/data.h>

/***** RegionAllocator *******************************************************/

const ZP_RAW ZP_END_OF_FREE = -1;

ZP_ERROR ZP_RegionAllocator_new(
  ZP_RegionAllocator_t* region_allocator,
  void* arena,
  size_t arena_size
) {
  if (arena_size < 2 * sizeof(ZP_RAW)) {
    return "ZP_RegionAllocator arena size must be at least 2*sizeof(ZP_RAW)";
  }
  if (arena_size > (1 << (8 * sizeof(ZP_RAW) - 8))) {
    return "ZP_RegionAllocator arena size must be at most 256**(sizeof(ZP_RAW) - 1)";
  }
  if (arena_size % (2 * sizeof(ZP_RAW)) != 0) {
    return "ZP_RegionAllocator arena size must be a multiple of 2*sizeof(ZP_RAW)";
  }
  region_allocator->arena = arena;
  region_allocator->arena_size = arena_size;
  region_allocator->first_free = 0;
  *(ZP_RAW*)region_allocator->arena = arena_size;
  *(ZP_RAW*)(region_allocator->arena + sizeof(ZP_RAW)) = ZP_END_OF_FREE;
  return ZP_NOERROR;
}

bool ZP_RegionAllocator_full(
  ZP_RegionAllocator_t* region_allocator
) {
  return region_allocator->first_free == ZP_END_OF_FREE;
}

void* ZP_RegionAllocator_at(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RAW pointer
) {
  return (void*)(region_allocator->arena + pointer);
}

ZP_RAW ZP_RegionAllocator_round_size(
  ZP_RAW request_size
) {
  const ZP_RAW denom = 2 * sizeof(ZP_RAW);
  ZP_RAW quot = request_size / denom;
  ZP_RAW rem = request_size % denom;
  return denom * (quot + (rem != 0));
}

ZP_ERROR ZP_RegionAllocator_alloc(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RAW request_size,
  ZP_RAW* pointer
) {
  ZP_RAW round_size = ZP_RegionAllocator_round_size(request_size);
  ZP_RAW prev = ZP_END_OF_FREE;
  ZP_RAW at = region_allocator->first_free;

  while (true) {
    if (at == ZP_END_OF_FREE) {
      return "not enough memory in RegionAllocator to allocate region";
    }

    ZP_RAW next_free = *(ZP_RAW*)(region_allocator->arena + at + sizeof(ZP_RAW));
    ZP_RAW this_free_size = *(ZP_RAW*)(region_allocator->arena + at);

    if (round_size < this_free_size) {
      /* split the region */
      *pointer = at;
      at += round_size;
      this_free_size -= round_size;
      *(ZP_RAW*)(region_allocator->arena + at) = this_free_size;
      *(ZP_RAW*)(region_allocator->arena + at + sizeof(ZP_RAW)) = next_free;
      break;
    }

    else if (round_size == this_free_size) {
      /* take the entire region */
      *pointer = at;
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
    *(ZP_RAW*)(region_allocator->arena + prev + sizeof(ZP_RAW)) = at;
  }

  return ZP_NOERROR;
}

ZP_ERROR ZP_RegionAllocator_free(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RAW pointer,
  ZP_RAW request_size
) {
  ZP_RAW round_size = ZP_RegionAllocator_round_size(request_size);
  ZP_RAW prev = ZP_END_OF_FREE;
  ZP_RAW at = region_allocator->first_free;

  /* find a place in the list to insert this, to keep the list sorted */
  /* note: as long as pointer is not already free, at != pointer */
  while (at != ZP_END_OF_FREE  &&  at < pointer) {
    prev = at;
    at = *(ZP_RAW*)(region_allocator->arena + at + sizeof(ZP_RAW));
  }

  /* insert the new free region right before the pointer, so the list stays sorted */
  /* this might not be used because the previous region might be extended over this one */
  /* but it doesn't hurt to write what WOULD go here, anyway */
  *(ZP_RAW*)(region_allocator->arena + pointer) = round_size;
  *(ZP_RAW*)(region_allocator->arena + pointer + sizeof(ZP_RAW)) = at;

  /* link the previous node (if any) to this new one */
  if (prev == ZP_END_OF_FREE) {
    region_allocator->first_free = pointer;
  }
  else {
    ZP_RAW prev_size = *(ZP_RAW*)(region_allocator->arena + prev);
    if (prev + prev_size == pointer) {
      /* previous region is contiguous with this one, so enlarge it to include this */
      *(ZP_RAW*)(region_allocator->arena + prev) += round_size;
    }
    else {
      /* there's non-free data between the regions; make a link in the linked list */
      *(ZP_RAW*)(region_allocator->arena + prev + sizeof(ZP_RAW)) = pointer;
    }
  }

  return ZP_NOERROR;
}

void ZP_RegionAllocator_copy_data(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RAW pointer,
  ZP_RAW request_size,
  const char* data
) {
  const ZP_RAW denom = sizeof(ZP_RAW);
  ZP_RAW quot = request_size / denom;
  ZP_RAW rem = request_size % denom;
  ZP_RAW count = (quot + (rem != 0));

  ZP_RAW i;
  for (i = 0;  i < count;  i++) {
    ((ZP_RAW*)(region_allocator->arena + pointer))[i] = ((ZP_RAW*)data)[i];
  }
}

/***** Symbols ***************************************************************/

const ZP_RAW ZP_END_OF_SYMBOLS = -2;

struct ZP_SymbolHeader {
  ZP_RAW symbol_data_size;
  ZP_RAW next_pointer;
};
typedef struct ZP_SymbolHeader ZP_SymbolHeader_t;

ZP_ERROR ZP_SymbolSet_new(
  ZP_SymbolSet_t* symbol_set
) {
  symbol_set->first_pointer = ZP_END_OF_SYMBOLS;
  return ZP_NOERROR;
}

ZP_COMPARE ZP_Symbol_cmp_data(
  ZP_RegionAllocator_t* region_allocator,
  ZP_RAW pointer,
  const char* symbol_data,
  ZP_RAW symbol_data_size
) {
  ZP_SymbolHeader_t* header
    = (ZP_SymbolHeader_t*)ZP_RegionAllocator_at(region_allocator, pointer);
  const char* data
    = (const char*)ZP_RegionAllocator_at(region_allocator, pointer + sizeof(ZP_SymbolHeader_t));

  ZP_RAW min_size = header->symbol_data_size;
  if (symbol_data_size < min_size) {
    min_size = symbol_data_size;
  }

  ZP_RAW i;
  for (i = 0;  i < min_size;  i++) {
    if (data[i] < symbol_data[i]) {
      return -1;
    }
    else if (data[i] > symbol_data[i]) {
      return 1;
    }
  }

  if (header->symbol_data_size < symbol_data_size) {
    return -1;
  }
  else if (header->symbol_data_size > symbol_data_size) {
    return 1;
  }
  else {
    return 0;
  }
}

ZP_ERROR ZP_Symbol_new(
  ZP_SymbolSet_t* symbol_set,
  ZP_RegionAllocator_t* region_allocator,
  const char* symbol_data,
  ZP_RAW symbol_data_size,
  ZP_Symbol_t* symbol
) {
  ZP_RAW before = ZP_END_OF_SYMBOLS;
  ZP_RAW current = symbol_set->first_pointer;

  while (current != ZP_END_OF_SYMBOLS) {
    ZP_COMPARE cmp = ZP_Symbol_cmp_data(region_allocator, current, symbol_data, symbol_data_size);
    if (cmp == -1) {
      ZP_SymbolHeader_t* current_header
        = (ZP_SymbolHeader_t*)ZP_RegionAllocator_at(region_allocator, current);
      before = current;
      current = current_header->next_pointer;
    }
    else if (cmp == 0) {
      symbol->pointer = current;
      return ZP_NOERROR;
    }
    else if (cmp == 1) {
      break;
    }
  }

  ZP_RAW request_size = sizeof(ZP_SymbolHeader_t) + symbol_data_size;

  ZP_RAW pointer;
  ZP_ERROR error = ZP_RegionAllocator_alloc(region_allocator, request_size, &pointer);
  if (error != ZP_NOERROR) {
    return error;
  }

  ZP_SymbolHeader_t* header
    = (ZP_SymbolHeader_t*)ZP_RegionAllocator_at(region_allocator, pointer);
  header->symbol_data_size = symbol_data_size;
  header->next_pointer = current;

  ZP_RegionAllocator_copy_data(
    region_allocator,
    pointer + sizeof(ZP_SymbolHeader_t),
    symbol_data_size,
    symbol_data
  );

  if (before == ZP_END_OF_SYMBOLS) {
    symbol_set->first_pointer = pointer;
  }
  else {
    ZP_SymbolHeader_t* before_header
      = (ZP_SymbolHeader_t*)ZP_RegionAllocator_at(region_allocator, before);
    before_header->next_pointer = pointer;
  }

  symbol->pointer = pointer;

  return ZP_NOERROR;
}
