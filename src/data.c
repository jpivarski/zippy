#include <stdio.h>  /* temporary! */

#include <zippy/data.h>

const ZP_Index_t ZP_END_OF_FREE = -1;  /* used by both RegionAllocator and PairAllocator */
const ZP_Index_t ZP_END_OF_SYMBOLS = -2;

/***** Labeled ***************************************************************/

const ZP_Index_t ZP_LABEL_SHIFT = 8 * (sizeof(ZP_Index_t) - 1);
const ZP_Index_t ZP_LABEL_MASK_DATA = ~(0xff << ZP_LABEL_SHIFT);

const ZP_Index_t ZP_INT_LABEL = ZP_LABEL_INT << ZP_LABEL_SHIFT;
const ZP_Index_t ZP_INT_MASK_SIGN = 0xc0 << ZP_LABEL_SHIFT;
const ZP_Index_t ZP_INT_MASK_DATA = ~ZP_INT_MASK_SIGN;
const ZP_Index_t ZP_INT_SIGN_TEST = 0x20 << ZP_LABEL_SHIFT;
const ZP_Index_t ZP_INT_OVERFLOW_TEST = 0xe0 << ZP_LABEL_SHIFT;

const ZP_Index_t ZP_FLOAT_SIGN_0 = ZP_LABEL_FLOAT << ZP_LABEL_SHIFT;
const ZP_Index_t ZP_FLOAT_SIGN_1 = ZP_FLOAT_SIGN_0 | 1;
const ZP_Index_t ZP_FLOAT_MASK_DATA = ~ZP_FLOAT_SIGN_0;
const ZP_Index_t ZP_FLOAT_MASK_SIGNIFICANT = ~(ZP_Index_t)1;

ZP_Label_t ZP_label_of(ZP_Labeled_t* labeled) {
  ZP_Label_t out = labeled->data >> ZP_LABEL_SHIFT;
  if (out & 0x80) {
    /* FLOAT */
    return 0x80;
  }
  else if (out & 0x40) {
    /* INTEGER */
    return 0x40;
  }
  else if (out & 0x30) {
    /* ARRAY_SIMPLE or ARRAY_FULL, remove DTYPE */
    return out & 0xf0;
  }
  else {
    /* enumerated label */
    return out;
  }
}

ZP_Index_t ZP_to_raw(ZP_Labeled_t* labeled) {
  return labeled->data & ZP_LABEL_MASK_DATA;
}

ZP_Int_t ZP_to_integer(ZP_Labeled_t* labeled) {
  /* remove the two label bits in a copy of the labeled->data */
  ZP_Index_t hold = labeled->data & ZP_INT_MASK_DATA;

  /* truncating the last two bits removes the sign */
  /* so interpret the third bit as the sign and pad the rest of the integer */
  hold |= (hold & ZP_INT_SIGN_TEST) ? ZP_INT_MASK_SIGN : 0;
  return *(ZP_Int_t*)(&hold);
}

ZP_Error_t ZP_from_integer(ZP_Int_t* data, ZP_Labeled_t* labeled) {
  ZP_Int_t test = *data & ZP_INT_OVERFLOW_TEST;
  if (test != 0  &&  test != ZP_INT_OVERFLOW_TEST) {
    return "integer overflow";
  }
  labeled->data = (*((ZP_Index_t*)data) & ZP_INT_MASK_DATA) | ZP_INT_LABEL;
  return ZP_NOERROR;
}

ZP_Float_t ZP_to_float(ZP_Labeled_t* labeled) {
  /* remove the (one) label bit in a copy of the labeled->data */
  ZP_Index_t hold = (labeled->data & ZP_FLOAT_MASK_DATA);

  /* the label bit overlaps IEEE floating point sign data */
  /* so restore the missing sign from the least significant bit */
  hold |= (hold & 1) ? ZP_FLOAT_SIGN_0 : 0;
  ZP_Float_t* out = (ZP_Float_t*)(&hold);

  return *out;
}

void ZP_from_float(ZP_Float_t* data, ZP_Labeled_t* labeled) {
  /* copy the floating point data into labeled->data */
  labeled->data = *((ZP_Index_t*)data);

  /* the label bit will overlap IEEE floating point sign data */
  /* so store the sign in the least significant bit */
  labeled->data &= ZP_FLOAT_MASK_SIGNIFICANT;
  labeled->data |= (labeled->data & ZP_FLOAT_SIGN_0) ? ZP_FLOAT_SIGN_1 : ZP_FLOAT_SIGN_0;
}

/***** RegionAllocator *******************************************************/

ZP_Error_t ZP_RegionAllocator_new(
  ZP_RegionAllocator_t* region_allocator,
  void* arena,
  size_t arena_size
) {
  if (arena_size < 2 * sizeof(ZP_Index_t)) {
    return "ZP_RegionAllocator arena_size must be at least 2*sizeof(ZP_Index_t)";
  }
  if (arena_size > (1 << (8 * sizeof(ZP_Index_t) - 8))) {
    return "ZP_RegionAllocator arena_size must be at most 256**(sizeof(ZP_Index_t) - 1)";
  }
  if (arena_size % (2 * sizeof(ZP_Index_t)) != 0) {
    return "ZP_RegionAllocator arena_size must be a multiple of 2*sizeof(ZP_Index_t)";
  }
  region_allocator->arena = arena;
  region_allocator->arena_size = arena_size;
  region_allocator->first_free = 0;
  *(ZP_Index_t*)region_allocator->arena = arena_size;
  *(ZP_Index_t*)(region_allocator->arena + sizeof(ZP_Index_t)) = ZP_END_OF_FREE;
  return ZP_NOERROR;
}

bool ZP_RegionAllocator_full(
  ZP_RegionAllocator_t* region_allocator
) {
  return region_allocator->first_free == ZP_END_OF_FREE;
}

void* ZP_RegionAllocator_at(
  ZP_RegionAllocator_t* region_allocator,
  ZP_Index_t pointer
) {
  return (void*)(region_allocator->arena + pointer);
}

ZP_Index_t ZP_RegionAllocator_round_size(
  ZP_Index_t request_size
) {
  const ZP_Index_t denom = 2 * sizeof(ZP_Index_t);
  ZP_Index_t quot = request_size / denom;
  ZP_Index_t rem = request_size % denom;
  return denom * (quot + (rem != 0));
}

ZP_Error_t ZP_RegionAllocator_alloc(
  ZP_RegionAllocator_t* region_allocator,
  ZP_Index_t request_size,
  ZP_Index_t* pointer
) {
  ZP_Index_t round_size = ZP_RegionAllocator_round_size(request_size);
  ZP_Index_t prev = ZP_END_OF_FREE;
  ZP_Index_t at = region_allocator->first_free;

  while (true) {
    if (at == ZP_END_OF_FREE) {
      return "not enough memory in RegionAllocator to allocate region";
    }

    ZP_Index_t next_free = *(ZP_Index_t*)(region_allocator->arena + at + sizeof(ZP_Index_t));
    ZP_Index_t this_free_size = *(ZP_Index_t*)(region_allocator->arena + at);

    if (round_size < this_free_size) {
      /* split the region */
      *pointer = at;
      at += round_size;
      this_free_size -= round_size;
      *(ZP_Index_t*)(region_allocator->arena + at) = this_free_size;
      *(ZP_Index_t*)(region_allocator->arena + at + sizeof(ZP_Index_t)) = next_free;
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
    *(ZP_Index_t*)(region_allocator->arena + prev + sizeof(ZP_Index_t)) = at;
  }

  return ZP_NOERROR;
}

void ZP_RegionAllocator_free(
  ZP_RegionAllocator_t* region_allocator,
  ZP_Index_t pointer,
  ZP_Index_t request_size
) {
  ZP_Index_t round_size = ZP_RegionAllocator_round_size(request_size);
  ZP_Index_t prev = ZP_END_OF_FREE;
  ZP_Index_t at = region_allocator->first_free;

  /* find a place in the list to insert this, to keep the list sorted */
  /* note: as long as pointer is not already free, at != pointer */
  while (at != ZP_END_OF_FREE  &&  at < pointer) {
    prev = at;
    at = *(ZP_Index_t*)(region_allocator->arena + at + sizeof(ZP_Index_t));
  }

  /* insert the new free region right before the pointer, so the list stays sorted */
  /* this might not be used because the previous region might be extended over this one */
  /* but it doesn't hurt to write what WOULD go here, anyway */
  *(ZP_Index_t*)(region_allocator->arena + pointer) = round_size;
  *(ZP_Index_t*)(region_allocator->arena + pointer + sizeof(ZP_Index_t)) = at;

  /* link the previous node (if any) to this new one */
  if (prev == ZP_END_OF_FREE) {
    region_allocator->first_free = pointer;
  }
  else {
    ZP_Index_t prev_size = *(ZP_Index_t*)(region_allocator->arena + prev);
    if (prev + prev_size == pointer) {
      /* previous region is contiguous with this one, so enlarge it to include this */
      *(ZP_Index_t*)(region_allocator->arena + prev) += round_size;
    }
    else {
      /* there's non-free data between the regions; make a link in the linked list */
      *(ZP_Index_t*)(region_allocator->arena + prev + sizeof(ZP_Index_t)) = pointer;
    }
  }
}

void ZP_RegionAllocator_copy_data(
  ZP_RegionAllocator_t* region_allocator,
  ZP_Index_t pointer,
  ZP_Index_t request_size,
  const char* data
) {
  const ZP_Index_t denom = sizeof(ZP_Index_t);
  ZP_Index_t quot = request_size / denom;
  ZP_Index_t rem = request_size % denom;
  ZP_Index_t count = (quot + (rem != 0));

  ZP_Index_t i;
  for (i = 0;  i < count;  i++) {
    ((ZP_Index_t*)(region_allocator->arena + pointer))[i] = ((ZP_Index_t*)data)[i];
  }
}

/***** PairAllocator *********************************************************/

ZP_Error_t ZP_PairAllocator_new(
  ZP_PairAllocator_t* pair_allocator,
  void* arena,
  void* refcount,
  size_t arena_size,
  size_t refcount_size
) {
  if (arena_size < 2 * sizeof(ZP_Index_t)) {
    return "ZP_PairAllocator arena_size must be at least 2*sizeof(ZP_Index_t)";
  }
  if (arena_size > (1 << (8 * sizeof(ZP_Index_t) - 8))) {
    return "ZP_PairAllocator arena_size must be at most 256**(sizeof(ZP_Index_t) - 1)";
  }
  if (arena_size % (2 * sizeof(ZP_Index_t)) != 0) {
    return "ZP_PairAllocator arena_size must be a multiple of 2*sizeof(ZP_Index_t)";
  }
  if (arena_size != 2 * sizeof(ZP_Index_t) * refcount_size) { /* assumes sizeof(ZP_RCnt_t) == 1 */
    return "ZP_PairAllocator arena_size must be equal to 2*sizeof(ZP_Index_t)*refcount_size";
  }

  pair_allocator->arena = (ZP_Index_t*)arena;
  pair_allocator->refcount = (ZP_RCnt_t*)refcount;
  pair_allocator->num_pairs = refcount_size;
  pair_allocator->first_pointer = ZP_END_OF_FREE;
  pair_allocator->bump_pointer = 0;

  return ZP_NOERROR;
}

ZP_Error_t ZP_PairAllocator_alloc(
  ZP_PairAllocator_t* pair_allocator,
  ZP_Pair_t* pair
) {
  if (pair_allocator->first_pointer == ZP_END_OF_FREE) {
    if (pair_allocator->bump_pointer == 2 * pair_allocator->num_pairs) {
      return "not enough memory in PairAllocator to allocate pair";
    }
    else {
      pair->first = pair_allocator->bump_pointer;
      pair_allocator->bump_pointer += 2;
    }
  }
  else {
    pair->first = pair_allocator->first_pointer;
    pair_allocator->first_pointer = pair_allocator->arena[pair_allocator->first_pointer];
  }

  return ZP_NOERROR;
}

void ZP_PairAllocator_free(
  ZP_PairAllocator_t* pair_allocator,
  ZP_Pair_t* pair
) {
  pair_allocator->arena[pair->first] = pair_allocator->first_pointer;
  pair_allocator->first_pointer = pair->first;
}

void ZP_PairAllocator_get(
  ZP_PairAllocator_t* pair_allocator,
  ZP_Pair_t* pair,
  ZP_Index_t* one,
  ZP_Index_t* two
) {
  *one = pair_allocator->arena[pair->first];
  *two = pair_allocator->arena[pair->first + 1];
}

void ZP_PairAllocator_set(
  ZP_PairAllocator_t* pair_allocator,
  ZP_Pair_t* pair,
  ZP_Index_t one,
  ZP_Index_t two
) {
  pair_allocator->arena[pair->first] = one;
  pair_allocator->arena[pair->first + 1] = two;
}



/***** Symbols ***************************************************************/

struct ZP_SymbolHeader {
  ZP_Index_t symbol_data_size;
  ZP_Index_t next_pointer;
};
typedef struct ZP_SymbolHeader ZP_SymbolHeader_t;

ZP_Error_t ZP_SymbolSet_new(
  ZP_SymbolSet_t* symbol_set
) {
  symbol_set->first_pointer = ZP_END_OF_SYMBOLS;
  return ZP_NOERROR;
}

ZP_Cmp_t ZP_Symbol_cmp_data(
  ZP_RegionAllocator_t* region_allocator,
  ZP_Index_t pointer,
  const char* symbol_data,
  ZP_Index_t symbol_data_size
) {
  ZP_SymbolHeader_t* header
    = (ZP_SymbolHeader_t*)ZP_RegionAllocator_at(region_allocator, pointer);
  const char* data
    = (const char*)ZP_RegionAllocator_at(region_allocator, pointer + sizeof(ZP_SymbolHeader_t));

  ZP_Index_t min_size = header->symbol_data_size;
  if (symbol_data_size < min_size) {
    min_size = symbol_data_size;
  }

  ZP_Index_t i;
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

ZP_Error_t ZP_Symbol_new(
  ZP_SymbolSet_t* symbol_set,
  ZP_RegionAllocator_t* region_allocator,
  const char* symbol_data,
  ZP_Index_t symbol_data_size,
  ZP_Symbol_t* symbol
) {
  ZP_Index_t before = ZP_END_OF_SYMBOLS;
  ZP_Index_t current = symbol_set->first_pointer;

  while (current != ZP_END_OF_SYMBOLS) {
    ZP_Cmp_t cmp = ZP_Symbol_cmp_data(region_allocator, current, symbol_data, symbol_data_size);
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

  ZP_Index_t request_size = sizeof(ZP_SymbolHeader_t) + symbol_data_size;

  ZP_Index_t pointer;
  ZP_Error_t error = ZP_RegionAllocator_alloc(region_allocator, request_size, &pointer);
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
