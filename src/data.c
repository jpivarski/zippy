#include <stdio.h>  /* temporary! */

#include <zippy/data.h>

const ZP_Index_t ZP_END_OF_FREE = -1;  /* used by RegionAllocator and FixedAllocator */
const ZP_RefCnt_t ZP_REFCNT_MAX = 0xff;  /* used by FixedAllocator */

/***** Labeled ***************************************************************/

const ZP_Index_t ZP_LABEL_SHIFT = 8 * (sizeof(ZP_Index_t) - 1);
const ZP_Index_t ZP_LABEL_MASK_LABEL = 0xff << ZP_LABEL_SHIFT;
const ZP_Index_t ZP_LABEL_MASK_DATA = ~ZP_LABEL_MASK_LABEL;

const ZP_Index_t ZP_LABEL_MASK_NUMBER = (ZP_LABEL_FLOAT | ZP_LABEL_INT) << ZP_LABEL_SHIFT;

const ZP_Index_t ZP_FLOAT_SIGN_0 = ZP_LABEL_FLOAT << ZP_LABEL_SHIFT;
const ZP_Index_t ZP_FLOAT_SIGN_1 = ZP_FLOAT_SIGN_0 | 1;
const ZP_Index_t ZP_FLOAT_MASK_DATA = ~ZP_FLOAT_SIGN_0;
const ZP_Index_t ZP_FLOAT_MASK_SIGNIFICANT = ~(ZP_Index_t)1;

const ZP_Index_t ZP_INT_LABEL = ZP_LABEL_INT << ZP_LABEL_SHIFT;
const ZP_Index_t ZP_INT_MASK_SIGN = 0xc0 << ZP_LABEL_SHIFT;
const ZP_Index_t ZP_INT_MASK_DATA = ~ZP_INT_MASK_SIGN;
const ZP_Index_t ZP_INT_SIGN_TEST = 0x20 << ZP_LABEL_SHIFT;
const ZP_Index_t ZP_INT_OVERFLOW_TEST = 0xe0 << ZP_LABEL_SHIFT;

/* Note: ZP_LABEL_ARRAY (0x30) == ZP_LABEL_ONE_REF (0x10) | ZP_LABEL_TWO_REF (0x20) */
const ZP_Index_t ZP_LABEL_MASK_AT_LEAST_ONE_REF = ZP_LABEL_ARRAY << ZP_LABEL_SHIFT;
const ZP_Index_t ZP_LABEL_MASK_HAS_TWO_REFS = ZP_LABEL_TWO_REF << ZP_LABEL_SHIFT;

const ZP_Index_t ZP_LABEL_IS_BUILTIN = ZP_LABEL_BUILTIN_ENUM << ZP_LABEL_SHIFT;
const ZP_Index_t ZP_LABEL_IS_BUFFER_OWNED = ZP_LABEL_BUFFER_OWNED << ZP_LABEL_SHIFT;
const ZP_Index_t ZP_LABEL_IS_LIST = ZP_LABEL_LIST << ZP_LABEL_SHIFT;

const ZP_Index_t ZP_TERMINATOR = ZP_LABEL_MASK_DATA;

ZP_Label_t ZP_Labeled_label_of(
  ZP_Labeled_t* labeled
) {
  ZP_Label_t out = labeled->data >> ZP_LABEL_SHIFT;

  /* special label classes need to be explicitly checked */
  if (out & ZP_LABEL_FLOAT) {
    return ZP_LABEL_FLOAT;
  }
  else if (out & ZP_LABEL_INT) {
    return ZP_LABEL_INT;
  }
  else if ((out & ZP_LABEL_ARRAY) == ZP_LABEL_ARRAY) {
    return out & 0xf0;  /* remove dtype from label */
  }
  else {
    /* enumerated label passed through as-is */
    return out;
  }
}

ZP_Index_t ZP_Labeled_to_raw(
  ZP_Labeled_t* labeled
) {
  return labeled->data & ZP_LABEL_MASK_DATA;
}

void ZP_Labeled_copy(
  ZP_Labeled_t* output,
  ZP_Labeled_t* input
) {
  output->data = input->data;
}

ZP_Int_t ZP_Labeled_to_int(
  ZP_Labeled_t* labeled
) {
  /* remove the two label bits in a copy of the labeled->data */
  ZP_Index_t hold = labeled->data & ZP_INT_MASK_DATA;

  /* truncating the last two bits removes the sign */
  /* so interpret the third bit as the sign and pad the rest of the integer */
  hold |= (hold & ZP_INT_SIGN_TEST) ? ZP_INT_MASK_SIGN : 0;
  return *(ZP_Int_t*)(&hold);
}

ZP_Error_t ZP_Labeled_from_int(
  ZP_Labeled_t* labeled,
  ZP_Int_t* data
) {
  ZP_Int_t test = *data & ZP_INT_OVERFLOW_TEST;
  if (test != 0  &&  test != ZP_INT_OVERFLOW_TEST) {
    return "integer overflow";
  }
  labeled->data = (*((ZP_Index_t*)data) & ZP_INT_MASK_DATA) | ZP_INT_LABEL;
  return ZP_NOERROR;
}

ZP_Float_t ZP_Labeled_to_float(
  ZP_Labeled_t* labeled
) {
  /* remove the (one) label bit in a copy of the labeled->data */
  ZP_Index_t hold = (labeled->data & ZP_FLOAT_MASK_DATA);

  /* the label bit overlaps IEEE floating point sign data */
  /* so restore the missing sign from the least significant bit */
  hold |= (hold & 1) ? ZP_FLOAT_SIGN_0 : 0;
  ZP_Float_t* out = (ZP_Float_t*)(&hold);

  return *out;
}

void ZP_Labeled_from_float(
  ZP_Labeled_t* labeled,
  ZP_Float_t* data
) {
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

bool ZP_RegionAllocator_is_full(
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

/***** FixedAllocator ********************************************************/

ZP_Error_t ZP_FixedAllocator_new(
  ZP_FixedAllocator_t* fixed_allocator,
  void* arena,
  void* refcount,
  size_t arena_size,
  size_t refcount_size
) {
  if (arena_size < 2 * sizeof(ZP_Index_t)) {
    return "ZP_FixedAllocator arena_size must be at least 2*sizeof(ZP_Index_t)";
  }
  if (arena_size > (1 << (8 * sizeof(ZP_Index_t) - 8))) {
    return "ZP_FixedAllocator arena_size must be at most 256**(sizeof(ZP_Index_t) - 1)";
  }
  if (arena_size % (2 * sizeof(ZP_Index_t)) != 0) {
    return "ZP_FixedAllocator arena_size must be a multiple of 2*sizeof(ZP_Index_t)";
  }
  if (arena_size != 2 * sizeof(ZP_Index_t) * refcount_size) { /* assumes sizeof(ZP_RefCnt_t) == 1 */
    return "ZP_FixedAllocator arena_size must be equal to 2*sizeof(ZP_Index_t)*refcount_size";
  }

  fixed_allocator->arena = (ZP_Index_t*)arena;
  fixed_allocator->refcount = (ZP_RefCnt_t*)refcount;
  fixed_allocator->num_pairs = refcount_size;
  fixed_allocator->first_pointer = ZP_END_OF_FREE;
  fixed_allocator->bump_pointer = 0;

  ZP_Index_t i;
  for (i = 0;  i < refcount_size;  i++) {
    fixed_allocator->refcount[i] = 0;
  }

  return ZP_NOERROR;
}

ZP_Error_t ZP_FixedAllocator_alloc(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_FixedLocation_t* fixed_location
) {
  if (fixed_allocator->first_pointer == ZP_END_OF_FREE) {
    if (fixed_allocator->bump_pointer == 2 * fixed_allocator->num_pairs) {
      return "not enough memory in FixedAllocator to allocate FixedLocation";
    }
    else {
      fixed_location->first = fixed_allocator->bump_pointer;
      fixed_allocator->bump_pointer += 2;
    }
  }
  else {
    fixed_location->first = fixed_allocator->first_pointer;
    fixed_allocator->first_pointer = fixed_allocator->arena[fixed_allocator->first_pointer];
  }

  return ZP_NOERROR;
}

void ZP_FixedAllocator_free(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_FixedLocation_t* fixed_location
) {
  fixed_allocator->arena[fixed_location->first] = fixed_allocator->first_pointer;
  fixed_allocator->first_pointer = fixed_location->first;
}

void ZP_FixedAllocator_get(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_FixedLocation_t* fixed_location,
  ZP_Index_t* one,
  ZP_Index_t* two
) {
  *one = fixed_allocator->arena[fixed_location->first];
  *two = fixed_allocator->arena[fixed_location->first + 1];
}

void ZP_FixedAllocator_set(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_FixedLocation_t* fixed_location,
  ZP_Index_t one,
  ZP_Index_t two
) {
  fixed_allocator->arena[fixed_location->first] = one;
  fixed_allocator->arena[fixed_location->first + 1] = two;
}

ZP_RefCnt_t ZP_FixedAllocator_refcnt(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_Labeled_t* labeled
) {
  if (labeled->data & ZP_LABEL_MASK_NUMBER) {
    return 0;
  }

  ZP_Index_t label = labeled->data & ZP_LABEL_MASK_LABEL;
  if (label == ZP_LABEL_IS_BUILTIN) {
    return 0;
  }

  ZP_Index_t location = labeled->data & ZP_LABEL_MASK_DATA;
  if (location == ZP_TERMINATOR) {
    return 0;
  }

  return fixed_allocator->refcount[location >> 1];
}

ZP_Error_t ZP_FixedAllocator_incref(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_Labeled_t* labeled
) {
  if (labeled->data & ZP_LABEL_MASK_NUMBER) {
    return ZP_NOERROR;
  }

  ZP_Index_t label = labeled->data & ZP_LABEL_MASK_LABEL;
  if (label == ZP_LABEL_IS_BUILTIN) {
    return ZP_NOERROR;
  }

  ZP_Index_t location = labeled->data & ZP_LABEL_MASK_DATA;
  if (location == ZP_TERMINATOR) {
    return ZP_NOERROR;
  }

  if (fixed_allocator->refcount[location >> 1] == ZP_REFCNT_MAX) {
    ZP_Error_t error = ZP_FixedAllocator_incref_newnode(
      fixed_allocator,
      labeled,
      location,
      (label & ZP_LABEL_MASK_AT_LEAST_ONE_REF) != 0,
      (label & ZP_LABEL_MASK_HAS_TWO_REFS) != 0
    );
    if (error != ZP_NOERROR) {
      return error;
    }
  }
  else {
    fixed_allocator->refcount[location >> 1]++;
  }

  return ZP_NOERROR;
}

ZP_Error_t ZP_FixedAllocator_incref_newnode(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_Labeled_t* labeled,
  ZP_Index_t location,
  bool at_least_one_ref,
  bool has_two_refs
) {
  ZP_FixedLocation_t new_location;
  ZP_Error_t error = ZP_FixedAllocator_alloc(fixed_allocator, &new_location);
  if (error != ZP_NOERROR) {
    return error;
  }

  ZP_FixedLocation_t old_location;
  old_location.first = location;
  ZP_Labeled_t labeled_left;
  ZP_Labeled_t labeled_right;
  ZP_FixedAllocator_get(
    fixed_allocator,
    &old_location,
    &labeled_left.data,
    &labeled_right.data
  );

  if (at_least_one_ref) {
    error = ZP_FixedAllocator_incref(fixed_allocator, &labeled_left);
    if (error != ZP_NOERROR) {
      ZP_FixedAllocator_free(fixed_allocator, &new_location);
      return error;
    }
  }
  if (has_two_refs) {
    error = ZP_FixedAllocator_incref(fixed_allocator, &labeled_right);
    if (error != ZP_NOERROR) {
      ZP_FixedAllocator_free(fixed_allocator, &new_location);
      ZP_FixedAllocator_decref(fixed_allocator, 0, &labeled_left);
      return error;
    }
  }

  ZP_FixedAllocator_set(
    fixed_allocator,
    &new_location,
    labeled_left.data,
    labeled_right.data
  );

  labeled->data = new_location.first | ZP_LABEL_IS_LIST;
  fixed_allocator->refcount[new_location.first >> 1] = 1;

  return ZP_NOERROR;
}

void ZP_FixedAllocator_decref(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_RegionAllocator_t* region_allocator,
  ZP_Labeled_t* labeled
) {
  switch (ZP_Labeled_label_of(labeled)) {
  case ZP_LABEL_LIST:
    ZP_List_decref(fixed_allocator, region_allocator, labeled);
    return;
  default:
    return;
  }
}

/***** BufferOwned ***********************************************************/

ZP_Error_t ZP_BufferOwned_new(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_RegionAllocator_t* region_allocator,
  ZP_Labeled_t* labeled,
  ZP_Index_t request_size
) {
  ZP_FixedLocation_t fixed_location;
  ZP_Error_t error = ZP_FixedAllocator_alloc(fixed_allocator, &fixed_location);
  if (error != ZP_NOERROR) {
    return error;
  }

  ZP_Index_t pointer;
  error = ZP_RegionAllocator_alloc(region_allocator, request_size, &pointer);
  if (error != ZP_NOERROR) {
    ZP_FixedAllocator_free(fixed_allocator, &fixed_location);
    return error;
  }

  ZP_FixedAllocator_set(fixed_allocator, &fixed_location, pointer, request_size);

  labeled->data = fixed_location.first | ZP_LABEL_IS_BUFFER_OWNED;

  error = ZP_FixedAllocator_incref(fixed_allocator, labeled);
  if (error != ZP_NOERROR) {
    ZP_FixedAllocator_free(fixed_allocator, &fixed_location);
    ZP_RegionAllocator_free(region_allocator, pointer, request_size);
    return error;
  }

  return ZP_NOERROR;
}

void ZP_BufferOwned_copy_data(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_RegionAllocator_t* region_allocator,
  ZP_Labeled_t* buffer,
  const char* data
) {
  ZP_FixedLocation_t buffer_location;
  buffer_location.first = ZP_Labeled_to_raw(buffer);

  ZP_Index_t pointer;
  ZP_Index_t request_size;
  ZP_FixedAllocator_get(fixed_allocator, &buffer_location, &pointer, &request_size);

  ZP_RegionAllocator_copy_data(region_allocator, pointer, request_size, data);
}




/***** List ******************************************************************/

void ZP_List_decref(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_RegionAllocator_t* region_allocator,
  ZP_Labeled_t* labeled
) {
  ZP_Index_t location = ZP_Labeled_to_raw(labeled);
  if (location != ZP_TERMINATOR) {
    fixed_allocator->refcount[location >> 1]--;

    if (fixed_allocator->refcount[location >> 1] == 0) {
      ZP_FixedLocation_t this_location;
      this_location.first = location;
      ZP_Labeled_t list_next;
      ZP_Labeled_t list_item;
      ZP_FixedAllocator_get(
        fixed_allocator,
        &this_location,
        &list_next.data,
        &list_item.data
      );
      ZP_List_decref(fixed_allocator, region_allocator, &list_next);
      ZP_FixedAllocator_decref(fixed_allocator, region_allocator, &list_item);
    }
  }
}

void ZP_List_new_empty(
  ZP_Labeled_t* list
) {
  list->data = ZP_TERMINATOR | ZP_LABEL_IS_LIST;
}

ZP_Error_t ZP_List_push(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_Labeled_t* output_list,
  ZP_Labeled_t* input_list,
  ZP_Labeled_t* item
) {
  if (ZP_Labeled_label_of(input_list) != ZP_LABEL_LIST) {
    return "ZP_List_push(_, _, input_list, _) must be a list";
  }

  ZP_Labeled_t input_list_copy;
  ZP_Labeled_copy(&input_list_copy, input_list);
  ZP_Error_t error = ZP_FixedAllocator_incref(fixed_allocator, &input_list_copy);
  if (error != ZP_NOERROR) {
    return error;
  }

  ZP_Labeled_t item_copy;
  ZP_Labeled_copy(&item_copy, item);
  error = ZP_FixedAllocator_incref(fixed_allocator, &item_copy);
  if (error != ZP_NOERROR) {
    ZP_List_decref(fixed_allocator, 0, &input_list_copy);
    return error;
  }

  ZP_FixedLocation_t fixed_location;
  error = ZP_FixedAllocator_alloc(fixed_allocator, &fixed_location);
  if (error != ZP_NOERROR) {
    ZP_List_decref(fixed_allocator, 0, &input_list_copy);
    ZP_List_decref(fixed_allocator, 0, &item_copy);
    return error;
  }

  ZP_FixedAllocator_set(
    fixed_allocator,
    &fixed_location,
    input_list_copy.data,
    item_copy.data
  );

  output_list->data = fixed_location.first | ZP_LABEL_IS_LIST;
  fixed_allocator->refcount[fixed_location.first >> 1] = 1;

  return ZP_NOERROR;
}

ZP_Error_t ZP_List_pop(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_Labeled_t* output_list,
  ZP_Labeled_t* output_item,
  ZP_Labeled_t* input_list
) {
  if (ZP_Labeled_label_of(input_list) != ZP_LABEL_LIST) {
    return "ZP_List_pop(_, _, _, input_list) must be a list";
  }

  ZP_FixedLocation_t input_list_location;
  input_list_location.first = ZP_Labeled_to_raw(input_list);
  if (input_list_location.first == ZP_TERMINATOR) {
    return "cannot pop empty list";
  }

  ZP_FixedAllocator_get(
    fixed_allocator,
    &input_list_location,
    &output_list->data,
    &output_item->data
  );

  ZP_Error_t error = ZP_FixedAllocator_incref(fixed_allocator, output_list);
  if (error != ZP_NOERROR) {
    return error;
  }
  error = ZP_FixedAllocator_incref(fixed_allocator, output_item);
  if (error != ZP_NOERROR) {
    ZP_List_decref(fixed_allocator, 0, output_list);
    return error;
  }

  return ZP_NOERROR;
}






/***** Symbols ***************************************************************/

/* const ZP_Index_t ZP_END_OF_SYMBOLS = -2; */

/* struct ZP_SymbolHeader { */
/*   ZP_Index_t symbol_data_size; */
/*   ZP_Index_t next_pointer; */
/* }; */
/* typedef struct ZP_SymbolHeader ZP_SymbolHeader_t; */

/* ZP_Error_t ZP_SymbolSet_new( */
/*   ZP_SymbolSet_t* symbol_set */
/* ) { */
/*   symbol_set->first_pointer = ZP_END_OF_SYMBOLS; */
/*   return ZP_NOERROR; */
/* } */

/* ZP_Cmp_t ZP_Symbol_cmp_data( */
/*   ZP_RegionAllocator_t* region_allocator, */
/*   ZP_Index_t pointer, */
/*   const char* symbol_data, */
/*   ZP_Index_t symbol_data_size */
/* ) { */
/*   ZP_SymbolHeader_t* header */
/*     = (ZP_SymbolHeader_t*)ZP_RegionAllocator_at(region_allocator, pointer); */
/*   const char* data */
/*     = (const char*)ZP_RegionAllocator_at(region_allocator, pointer + sizeof(ZP_SymbolHeader_t)); */

/*   ZP_Index_t min_size = header->symbol_data_size; */
/*   if (symbol_data_size < min_size) { */
/*     min_size = symbol_data_size; */
/*   } */

/*   ZP_Index_t i; */
/*   for (i = 0;  i < min_size;  i++) { */
/*     if (data[i] < symbol_data[i]) { */
/*       return -1; */
/*     } */
/*     else if (data[i] > symbol_data[i]) { */
/*       return 1; */
/*     } */
/*   } */

/*   if (header->symbol_data_size < symbol_data_size) { */
/*     return -1; */
/*   } */
/*   else if (header->symbol_data_size > symbol_data_size) { */
/*     return 1; */
/*   } */
/*   else { */
/*     return 0; */
/*   } */
/* } */

/* ZP_Error_t ZP_Symbol_new( */
/*   ZP_SymbolSet_t* symbol_set, */
/*   ZP_RegionAllocator_t* region_allocator, */
/*   const char* symbol_data, */
/*   ZP_Index_t symbol_data_size, */
/*   ZP_Symbol_t* symbol */
/* ) { */
/*   ZP_Index_t before = ZP_END_OF_SYMBOLS; */
/*   ZP_Index_t current = symbol_set->first_pointer; */

/*   while (current != ZP_END_OF_SYMBOLS) { */
/*     ZP_Cmp_t cmp = ZP_Symbol_cmp_data(region_allocator, current, symbol_data, symbol_data_size); */
/*     if (cmp == -1) { */
/*       ZP_SymbolHeader_t* current_header */
/*         = (ZP_SymbolHeader_t*)ZP_RegionAllocator_at(region_allocator, current); */
/*       before = current; */
/*       current = current_header->next_pointer; */
/*     } */
/*     else if (cmp == 0) { */
/*       symbol->pointer = current; */
/*       return ZP_NOERROR; */
/*     } */
/*     else if (cmp == 1) { */
/*       break; */
/*     } */
/*   } */

/*   ZP_Index_t request_size = sizeof(ZP_SymbolHeader_t) + symbol_data_size; */

/*   ZP_Index_t pointer; */
/*   ZP_Error_t error = ZP_RegionAllocator_alloc(region_allocator, request_size, &pointer); */
/*   if (error != ZP_NOERROR) { */
/*     return error; */
/*   } */

/*   ZP_SymbolHeader_t* header */
/*     = (ZP_SymbolHeader_t*)ZP_RegionAllocator_at(region_allocator, pointer); */
/*   header->symbol_data_size = symbol_data_size; */
/*   header->next_pointer = current; */

/*   ZP_RegionAllocator_copy_data( */
/*     region_allocator, */
/*     pointer + sizeof(ZP_SymbolHeader_t), */
/*     symbol_data_size, */
/*     symbol_data */
/*   ); */

/*   if (before == ZP_END_OF_SYMBOLS) { */
/*     symbol_set->first_pointer = pointer; */
/*   } */
/*   else { */
/*     ZP_SymbolHeader_t* before_header */
/*       = (ZP_SymbolHeader_t*)ZP_RegionAllocator_at(region_allocator, before); */
/*     before_header->next_pointer = pointer; */
/*   } */

/*   symbol->pointer = pointer; */

/*   return ZP_NOERROR; */
/* } */
