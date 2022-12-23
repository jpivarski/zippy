#include <stdio.h>  /* temporary! */

#include <zippy/data.h>

/***** utilities *************************************************************/

const ZP_Index_t ZP_END_OF_FREE = -1;  /* used by RegionAllocator and FixedAllocator */
const ZP_RefCnt_t ZP_REFCNT_MAX = 0xff;  /* used by FixedAllocator */

const ZP_Index_t ZP_TAG_SHIFT = 8 * (sizeof(ZP_Index_t) - 1);
const ZP_Index_t ZP_TAG_MASK_TAG = 0xff << ZP_TAG_SHIFT;
const ZP_Index_t ZP_TAG_MASK_DATA = ~ZP_TAG_MASK_TAG;

const ZP_Index_t ZP_TAG_MASK_NUMBER = (ZP_TAG_FLOAT | ZP_TAG_INT) << ZP_TAG_SHIFT;

const ZP_Index_t ZP_FLOAT_SIGN_0 = ZP_TAG_FLOAT << ZP_TAG_SHIFT;
const ZP_Index_t ZP_FLOAT_SIGN_1 = ZP_FLOAT_SIGN_0 | 1;
const ZP_Index_t ZP_FLOAT_MASK_DATA = ~ZP_FLOAT_SIGN_0;
const ZP_Index_t ZP_FLOAT_MASK_SIGNIFICANT = ~(ZP_Index_t)1;

const ZP_Index_t ZP_INT_TAG = ZP_TAG_INT << ZP_TAG_SHIFT;
const ZP_Index_t ZP_INT_MASK_SIGN = 0xc0 << ZP_TAG_SHIFT;
const ZP_Index_t ZP_INT_MASK_DATA = ~ZP_INT_MASK_SIGN;
const ZP_Index_t ZP_INT_SIGN_TEST = 0x20 << ZP_TAG_SHIFT;
const ZP_Index_t ZP_INT_OVERFLOW_TEST = 0xe0 << ZP_TAG_SHIFT;

const ZP_Index_t ZP_INT_MAXINT = (1 << ((8 * sizeof(ZP_Index_t)) - 3)) - 1;
const ZP_Index_t ZP_INT_MININT = -ZP_INT_MAXINT - 1;

/* Note: ZP_TAG_ARRAY (0x30) == ZP_TAG_ONE_REF (0x10) | ZP_TAG_TWO_REF (0x20) */
const ZP_Index_t ZP_TAG_MASK_AT_LEAST_ONE_REF = ZP_TAG_ARRAY << ZP_TAG_SHIFT;
const ZP_Index_t ZP_TAG_MASK_HAS_TWO_REFS = ZP_TAG_TWO_REF << ZP_TAG_SHIFT;

const ZP_Index_t ZP_TAG_MASK_INT = ZP_TAG_INT << ZP_TAG_SHIFT;
const ZP_Index_t ZP_TAG_MASK_FLOAT = ZP_TAG_FLOAT << ZP_TAG_SHIFT;
const ZP_Index_t ZP_TAG_MASK_ARRAY = ZP_TAG_ARRAY << ZP_TAG_SHIFT;
const ZP_Index_t ZP_TAG_MASK_BUILTIN_ENUM = ZP_TAG_BUILTIN_ENUM << ZP_TAG_SHIFT;
const ZP_Index_t ZP_TAG_MASK_BUFFER_OWNED = ZP_TAG_BUFFER_OWNED << ZP_TAG_SHIFT;
const ZP_Index_t ZP_TAG_MASK_BUFFER_VIEW = ZP_TAG_BUFFER_VIEW << ZP_TAG_SHIFT;
const ZP_Index_t ZP_TAG_MASK_SYMBOL = ZP_TAG_SYMBOL << ZP_TAG_SHIFT;
const ZP_Index_t ZP_TAG_MASK_SCALAR_IN_ARRAY = ZP_TAG_SCALAR_IN_ARRAY << ZP_TAG_SHIFT;
const ZP_Index_t ZP_TAG_MASK_BUFFER_VIEW_WRAP = ZP_TAG_BUFFER_VIEW_WRAP << ZP_TAG_SHIFT;
const ZP_Index_t ZP_TAG_MASK_LIST = ZP_TAG_LIST << ZP_TAG_SHIFT;
const ZP_Index_t ZP_TAG_MASK_CLASS = ZP_TAG_CLASS << ZP_TAG_SHIFT;
const ZP_Index_t ZP_TAG_MASK_INSTANCE = ZP_TAG_INSTANCE << ZP_TAG_SHIFT;
const ZP_Index_t ZP_TAG_MASK_INSTANCE_NODE = ZP_TAG_INSTANCE_NODE << ZP_TAG_SHIFT;

const ZP_Index_t ZP_TERMINATOR = ZP_TAG_MASK_DATA;

void ZP_slice(
  ZP_Index_t* new_pointer,
  ZP_Index_t* new_size,
  ZP_Index_t base_pointer,
  ZP_Index_t base_size,
  ZP_Index_t slice_start,
  ZP_Index_t slice_stop
) {
  if (slice_start < 0) {
    slice_start += base_size;
  }
  if (slice_start < 0) {
    slice_start = 0;
  }
  if (slice_start >= base_size) {
    slice_start = base_size;
  }

  if (slice_stop < 0) {
    slice_stop += base_size;
  }
  if (slice_stop < 0) {
    slice_stop = 0;
  }
  if (slice_stop >= base_size) {
    slice_stop = base_size;
  }

  if (slice_stop < slice_start) {
    slice_stop = slice_start;
  }

  *new_pointer = base_pointer + slice_start;
  *new_size = slice_stop - slice_start;
}

/***** Tagged ****************************************************************/

/* used internally; does not convert ZP_Index_t -> 8-bit */
ZP_Index_t ZP_Tagged_tag_mask_of(
  ZP_Tagged_t* tagged
) {
  ZP_Index_t out = tagged->data;

  /* special tag classes need to be explicitly checked */
  if (out & ZP_TAG_MASK_FLOAT) {
    return ZP_TAG_MASK_FLOAT;
  }
  else if (out & ZP_TAG_MASK_INT) {
    return ZP_TAG_MASK_INT;
  }
  else if ((out & ZP_TAG_MASK_ARRAY) == ZP_TAG_MASK_ARRAY) {
    return out & 0xf0;  /* remove dtype from tag */
  }
  else {
    /* enumerated tag passed through as-is */
    return out & ZP_TAG_MASK_TAG;
  }
}

/* public: uses the tags defined in data.h */
ZP_Tag_t ZP_Tagged_tag_of(
  ZP_Tagged_t* tagged
) {
  return ZP_Tagged_tag_mask_of(tagged) >> ZP_TAG_SHIFT;
}

ZP_Index_t ZP_Tagged_to_raw(
  ZP_Tagged_t* tagged
) {
  return tagged->data & ZP_TAG_MASK_DATA;
}

void ZP_Tagged_copy(
  ZP_Tagged_t* output,
  ZP_Tagged_t* input
) {
  output->data = input->data;
}

ZP_Int_t ZP_Tagged_to_int(
  ZP_Tagged_t* tagged
) {
  /* remove the two tag bits in a copy of the tagged->data */
  ZP_Index_t hold = tagged->data & ZP_INT_MASK_DATA;

  /* truncating the last two bits removes the sign */
  /* so interpret the third bit as the sign and pad the rest of the integer */
  hold |= (hold & ZP_INT_SIGN_TEST) ? ZP_INT_MASK_SIGN : 0;
  return *(ZP_Int_t*)(&hold);
}

ZP_Error_t ZP_Tagged_from_int(
  ZP_Tagged_t* tagged,
  ZP_Int_t* data
) {
  ZP_Int_t test = *data & ZP_INT_OVERFLOW_TEST;
  if (test != 0  &&  test != ZP_INT_OVERFLOW_TEST) {
    return "integer overflow";
  }
  tagged->data = (*((ZP_Index_t*)data) & ZP_INT_MASK_DATA) | ZP_INT_TAG;
  return ZP_NOERROR;
}

ZP_Float_t ZP_Tagged_to_float(
  ZP_Tagged_t* tagged
) {
  /* remove the (one) tag bit in a copy of the tagged->data */
  ZP_Index_t hold = (tagged->data & ZP_FLOAT_MASK_DATA);

  /* the tag bit overlaps IEEE floating point sign data */
  /* so restore the missing sign from the least significant bit */
  hold |= (hold & 1) ? ZP_FLOAT_SIGN_0 : 0;
  ZP_Float_t* out = (ZP_Float_t*)(&hold);

  return *out;
}

void ZP_Tagged_from_float(
  ZP_Tagged_t* tagged,
  ZP_Float_t* data
) {
  /* copy the floating point data into tagged->data */
  tagged->data = *((ZP_Index_t*)data);

  /* the tag bit will overlap IEEE floating point sign data */
  /* so store the sign in the least significant bit */
  tagged->data &= ZP_FLOAT_MASK_SIGNIFICANT;
  tagged->data |= (tagged->data & ZP_FLOAT_SIGN_0) ? ZP_FLOAT_SIGN_1 : ZP_FLOAT_SIGN_0;
}

/***** FixedAllocator ********************************************************/

void ZP_Tagged_to_FixedLocation(
  ZP_FixedLocation_t* location,
  ZP_Tagged_t* tagged
) {
  location->at = tagged->data & ZP_TAG_MASK_DATA;
}

void ZP_Tagged_from_FixedLocation(
  ZP_Tagged_t* tagged,
  ZP_FixedLocation_t* location,
  ZP_Index_t tag_mask
) {
  tagged->data = location->at | tag_mask;
}

void ZP_FixedAllocator_new(
  ZP_FixedAllocator_t* fixed_allocator,
  void* arena,
  size_t arena_size
) {
  fixed_allocator->arena = (ZP_Index_t*)arena;
  fixed_allocator->num_pairs = arena_size / (2 * sizeof(ZP_Index_t));
  fixed_allocator->first_pointer = ZP_END_OF_FREE;
  fixed_allocator->bump_pointer = 0;
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
      fixed_location->at = fixed_allocator->bump_pointer;
      fixed_allocator->bump_pointer += 2;
    }
  }
  else {
    fixed_location->at = fixed_allocator->first_pointer;
    fixed_allocator->first_pointer = fixed_allocator->arena[fixed_allocator->first_pointer];
  }

  return ZP_NOERROR;
}

void ZP_FixedAllocator_free(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_FixedLocation_t* fixed_location
) {
  fixed_allocator->arena[fixed_location->at] = fixed_allocator->first_pointer;
  fixed_allocator->first_pointer = fixed_location->at;
}

void ZP_FixedAllocator_get(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_FixedLocation_t* fixed_location,
  ZP_Index_t* left,
  ZP_Index_t* right
) {
  *left = fixed_allocator->arena[fixed_location->at];
  *right = fixed_allocator->arena[fixed_location->at + 1];
}

void ZP_FixedAllocator_set(
  ZP_FixedAllocator_t* fixed_allocator,
  ZP_FixedLocation_t* fixed_location,
  ZP_Index_t left,
  ZP_Index_t right
) {
  fixed_allocator->arena[fixed_location->at] = left;
  fixed_allocator->arena[fixed_location->at + 1] = right;
}

/***** RegionAllocator *******************************************************/

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
    if (round_size == at) {
      /* immediately after this block is the first free region, so merge them */
      ZP_Index_t next_size = *(ZP_Index_t*)(region_allocator->arena + at);
      ZP_Index_t next_at = *(ZP_Index_t*)(region_allocator->arena + at + sizeof(ZP_Index_t));
      *(ZP_Index_t*)(region_allocator->arena + pointer) += next_size;
      *(ZP_Index_t*)(region_allocator->arena + pointer + sizeof(ZP_Index_t)) = next_at;
    }
    region_allocator->first_free = pointer;
  }
  else {
    ZP_Index_t prev_size = *(ZP_Index_t*)(region_allocator->arena + prev);
    if (prev + prev_size == pointer) {
      /* previous region is contiguous with this one, so enlarge it to include this */
      *(ZP_Index_t*)(region_allocator->arena + prev) += round_size;

      if (at != ZP_END_OF_FREE  &&  prev + prev_size + round_size == at) {
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
      *(ZP_Index_t*)(region_allocator->arena + prev + sizeof(ZP_Index_t)) = pointer;

      if (at != ZP_END_OF_FREE  &&  pointer + round_size == at) {
        /* this region is contiguous with the next one, so enlarge this one, */
        /* taking over the next one */
        ZP_Index_t next_size = *(ZP_Index_t*)(region_allocator->arena + at);
        ZP_Index_t next_at = *(ZP_Index_t*)(region_allocator->arena + at + sizeof(ZP_Index_t));
        *(ZP_Index_t*)(region_allocator->arena + pointer) += next_size;
        *(ZP_Index_t*)(region_allocator->arena + pointer + sizeof(ZP_Index_t)) = next_at;
      }
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

/***** Context ***************************************************************/

ZP_Error_t ZP_Context_new(
  ZP_Context_t* context,
  void* fixed_arena,
  void* region_arena,
  void* refcount_arena,
  size_t fixed_size,
  size_t region_size,
  size_t refcount_size
) {
  if (fixed_size < 2 * sizeof(ZP_Index_t)) {
    return "fixed_size must be at least 2*sizeof(ZP_Index_t)";
  }
  if (fixed_size > (1 << (8 * sizeof(ZP_Index_t) - 8))) {
    return "fixed_size must be at most 256**(sizeof(ZP_Index_t) - 1)";
  }
  if (fixed_size % (2 * sizeof(ZP_Index_t)) != 0) {
    return "fixed_size must be a multiple of 2*sizeof(ZP_Index_t)";
  }

  if (region_size < 2 * sizeof(ZP_Index_t)) {
    return "region_size must be at least 2*sizeof(ZP_Index_t)";
  }
  if (region_size % (2 * sizeof(ZP_Index_t)) != 0) {
    return "region_size must be a multiple of 2*sizeof(ZP_Index_t)";
  }

  if (fixed_size != 2 * sizeof(ZP_Index_t) * refcount_size) { /* assumes sizeof(ZP_RefCnt_t) == 1 */
    return "fixed_size must be equal to 2*sizeof(ZP_Index_t)*refcount_size";
  }

  ZP_FixedAllocator_new(
    &context->fixed_allocator,
    fixed_arena,
    fixed_size
  );

  ZP_RegionAllocator_new(
    &context->region_allocator,
    region_arena,
    region_size
  );

  context->refcount = (ZP_RefCnt_t*)refcount_arena;

  ZP_Index_t i;
  for (i = 0;  i < refcount_size;  i++) {
    context->refcount[i] = 0;
  }

  ZP_List_new_empty(&context->symbol_set.list);
  context->symbol_set.next_number = 0;

  return ZP_NOERROR;
}

ZP_RefCnt_t ZP_Context_refcnt(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged
) {
  if (tagged->data & ZP_TAG_MASK_NUMBER) {
    return 0;
  }

  ZP_Index_t tag = tagged->data & ZP_TAG_MASK_TAG;
  if (tag == ZP_TAG_MASK_BUILTIN_ENUM) {
    return 0;
  }

  ZP_Index_t location = tagged->data & ZP_TAG_MASK_DATA;
  if (location == ZP_TERMINATOR) {
    return 0;
  }

  return context->refcount[location >> 1];
}

void ZP_Context_refcnt_init(
  ZP_Context_t* context,
  ZP_FixedLocation_t* location
) {
  context->refcount[location->at >> 1] = 1;
}

ZP_Error_t ZP_Context_incref(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged
) {
  if (tagged->data & ZP_TAG_MASK_NUMBER) {
    return ZP_NOERROR;
  }

  ZP_Index_t tag_mask = tagged->data & ZP_TAG_MASK_TAG;
  if (tag_mask == ZP_TAG_MASK_BUILTIN_ENUM) {
    return ZP_NOERROR;
  }

  ZP_FixedLocation_t location;
  location.at = tagged->data & ZP_TAG_MASK_DATA;
  if (location.at == ZP_TERMINATOR) {
    return ZP_NOERROR;
  }

  if (context->refcount[location.at >> 1] == ZP_REFCNT_MAX) {
    if (tag_mask == ZP_TAG_MASK_BUFFER_OWNED) {
      return "directly owned buffers cannot be referenced more than ZP_REFCNT_MAX times";
    }

    else if (tag_mask == ZP_TAG_MASK_SYMBOL) {
      /* symbol references are also limited (transitively by the limitation on */
      /* owned buffers), but the symbol_data can be copied into a new symbol */
      /* with the same identifying number */
      ZP_Index_t pointer;
      ZP_Index_t symbol_size;
      ZP_Symbol_pointer_and_size(context, &pointer, &symbol_size, tagged);

      const char* symbol_data = (const char*)ZP_RegionAllocator_at(
        &context->region_allocator,
        pointer
      );

      return ZP_Symbol_new_or_get(context, tagged, symbol_data, symbol_size);
    }

    else {
      /* anything else can just be copied into a new reference */
      ZP_Error_t error = ZP_Context_incref_newnode(
        context,
        tagged,
        location.at,
        (tag_mask & ZP_TAG_MASK_AT_LEAST_ONE_REF) != 0,
        (tag_mask & ZP_TAG_MASK_HAS_TWO_REFS) != 0,
        tag_mask
      );
      if (error != ZP_NOERROR) {
        return error;
      }
    }
  }
  else {
    context->refcount[location.at >> 1]++;
  }

  return ZP_NOERROR;
}

ZP_Error_t ZP_Context_incref_newnode(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged,
  ZP_Index_t location,
  bool at_least_one_ref,
  bool has_two_refs,
  ZP_Index_t tag_mask
) {
  ZP_FixedLocation_t new_location;
  ZP_Error_t error = ZP_FixedAllocator_alloc(&context->fixed_allocator, &new_location);
  if (error != ZP_NOERROR) {
    return error;
  }

  ZP_FixedLocation_t old_location;
  old_location.at = location;
  ZP_Tagged_t tagged_left;
  ZP_Tagged_t tagged_right;
  ZP_FixedAllocator_get(
    &context->fixed_allocator,
    &old_location,
    &tagged_left.data,
    &tagged_right.data
  );

  if (at_least_one_ref) {
    error = ZP_Context_incref(context, &tagged_left);
    if (error != ZP_NOERROR) {
      ZP_FixedAllocator_free(&context->fixed_allocator, &new_location);
      return error;
    }
  }
  if (has_two_refs) {
    error = ZP_Context_incref(context, &tagged_right);
    if (error != ZP_NOERROR) {
      ZP_FixedAllocator_free(&context->fixed_allocator, &new_location);
      ZP_Context_decref(context, &tagged_left);
      return error;
    }
  }

  ZP_FixedAllocator_set(
    &context->fixed_allocator,
    &new_location,
    tagged_left.data,
    tagged_right.data
  );

  ZP_Tagged_from_FixedLocation(tagged, &new_location, tag_mask);
  ZP_Context_refcnt_init(context, &new_location);

  return ZP_NOERROR;
}

void ZP_Context_decref(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged
) {
  switch (ZP_Tagged_tag_mask_of(tagged)) {
  case ZP_TAG_MASK_BUFFER_OWNED:
    ZP_BufferOwned_decref(context, tagged);
    return;
  case ZP_TAG_MASK_BUFFER_VIEW:
    ZP_BufferView_decref_node(context, tagged);
    return;
  case ZP_TAG_MASK_SYMBOL:
    ZP_Symbol_decref(context, tagged);
    return;
  case ZP_TAG_MASK_BUFFER_VIEW_WRAP:
    ZP_BufferView_decref(context, tagged);
    return;
  case ZP_TAG_MASK_LIST:
    ZP_List_decref(context, tagged);
    return;
  default:
    return;
  }
}

void ZP_Context_decref_children(
  ZP_Context_t* context,
  ZP_FixedLocation_t* location,
  bool left_is_ref,
  bool right_is_ref
) {
  ZP_Tagged_t left;
  ZP_Tagged_t right;
  ZP_FixedAllocator_get(
    &context->fixed_allocator,
    location,
    &left.data,
    &right.data
  );
  if (left_is_ref) {
    ZP_Context_decref(context, &left);
  }
  if (right_is_ref) {
    ZP_Context_decref(context, &right);
  }
}

/***** Symbol ****************************************************************/

void ZP_Symbol_decref(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged
) {
  ZP_FixedLocation_t location;
  ZP_Tagged_to_FixedLocation(&location, tagged);
  context->refcount[location.at >> 1]--;

  /* cleanup happens when this reaches 1 because it's in the global symbol_set */
  if (context->refcount[location.at >> 1] == 1) {
    ZP_Tagged_t list_node;
    ZP_Tagged_copy(&list_node, &context->symbol_set.list);

    ZP_FixedLocation_t prev_node_location;
    prev_node_location.at = ZP_TERMINATOR;
    ZP_Tagged_t prev_symbol;
    prev_symbol.data = 0;  /* never accessed */

    /* find the symbol in the list and remove it */
    while (!ZP_List_is_empty(&list_node)) {

      ZP_FixedLocation_t list_node_location;
      ZP_Tagged_to_FixedLocation(&list_node_location, &list_node);

      ZP_Tagged_t list_next;
      ZP_Tagged_t this_symbol;
      ZP_FixedAllocator_get(
        &context->fixed_allocator,
        &list_node_location,
        &list_next.data,
        &this_symbol.data
      );

      if (this_symbol.data == tagged->data) {
        /* found the symbol; now remove it from the list */
        if (prev_node_location.at == ZP_TERMINATOR) {
          /* it's first in the list, set the list head to the next one */
          context->symbol_set.list = list_next;
        }
        else {
          /* it's not first in the list, make the previous point past this */
          ZP_FixedAllocator_set(
            &context->fixed_allocator,
            &prev_node_location,
            list_next.data,
            prev_symbol.data
          );
        }

        /* in both cases, the removed list node must be decrefed */
        context->refcount[list_node_location.at >> 1]--;

        break;
      }

      /* otherwise, keep walking the list */
      prev_node_location = list_node_location;
      prev_symbol = this_symbol;

      list_node = list_next;
    }

    /* get rid of the last reference (it's no longer in the list) */
    context->refcount[location.at >> 1]--;

    /* decref the buffer_owned */
    ZP_Context_decref_children(context, &location, true, false);
  }
}

void ZP_Symbol_pointer_and_size(
  ZP_Context_t* context,
  ZP_Index_t* pointer,
  ZP_Index_t* request_size,
  ZP_Tagged_t* tagged
) {
  ZP_FixedLocation_t location;
  ZP_Tagged_to_FixedLocation(&location, tagged);

  ZP_Tagged_t buffer_owned;
  ZP_Tagged_t symbol_number;
  ZP_FixedAllocator_get(
    &context->fixed_allocator,
    &location,
    &buffer_owned.data,
    &symbol_number.data
  );

  ZP_FixedLocation_t buffer_owned_location;
  ZP_Tagged_to_FixedLocation(&buffer_owned_location, &buffer_owned);

  ZP_FixedAllocator_get(
    &context->fixed_allocator,
    &buffer_owned_location,
    pointer,
    request_size
  );
}

ZP_Error_t ZP_Symbol_new_or_get(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged,
  const char* symbol_data,
  ZP_Index_t symbol_size
) {
  ZP_Tagged_t list_node;
  ZP_Tagged_copy(&list_node, &context->symbol_set.list);

  ZP_Int_t number_to_use = -1;
  bool duplicate = false;

  while (!ZP_List_is_empty(&list_node)) {
    /* extract the Symbol character data at this list_node */

    ZP_FixedLocation_t list_node_location;
    ZP_Tagged_to_FixedLocation(&list_node_location, &list_node);

    ZP_Tagged_t list_next;
    ZP_Tagged_t this_symbol;
    ZP_FixedAllocator_get(
      &context->fixed_allocator,
      &list_node_location,
      &list_next.data,
      &this_symbol.data
    );

    ZP_Index_t pointer;
    ZP_Index_t request_size;
    ZP_Symbol_pointer_and_size(context, &pointer, &request_size, &this_symbol);

    /* compare this char data with the provided char data */
    if (symbol_size == request_size) {
      const char* compare_symbol = (const char*)ZP_RegionAllocator_at(
        &context->region_allocator,
        pointer
      );

      bool same = true;
      ZP_Index_t i;
      for (i = 0;  i < symbol_size;  i++) {
        if (symbol_data[i] != compare_symbol[i]) {
          same = false;
          break;
        }
      }

      if (same) {
        ZP_FixedLocation_t this_symbol_location;
        ZP_Tagged_to_FixedLocation(&this_symbol_location, &this_symbol);

        if (context->refcount[this_symbol_location.at >> 1] == ZP_REFCNT_MAX) {
          /* if it's the same and the reference count has been exceeded, */
          /* stop searching and add a new symbol node to the list */
          ZP_Tagged_t buffer_owned;
          ZP_Tagged_t symbol_number;
          ZP_FixedAllocator_get(
            &context->fixed_allocator,
            &this_symbol_location,
            &buffer_owned.data,
            &symbol_number.data
          );
          number_to_use = ZP_Tagged_to_int(&symbol_number);
          duplicate = true;
          break;
        }
        else {
          /* if it's the same and the reference count has not been exceeded, */
          /* return a new reference to it */
          ZP_Tagged_copy(tagged, &this_symbol);
          return ZP_Context_incref(context, tagged);
        }
      }
    }

    /* otherwise, keep walking the list */
    list_node = list_next;

  } /* end loop over existing list */

  /* the new symbol is either a deliberate duplicate (because the reference count */
  /* has been used up) or a new number, which are globally unique in the context */
  if (!duplicate) {
    number_to_use = context->symbol_set.next_number;
  }

  if (number_to_use == ZP_INT_MAXINT) {
    return "no more symbols can be created in this context";
    /* FIXME: can get twice as many by skipping the invalid domain, using */
    /* negative numbers, and eventually stopping on -1 instead of ZP_INT_MAXINT */
  }

  /* first, make the buffer_owned */
  ZP_Tagged_t buffer_owned;
  ZP_Error_t error = ZP_BufferOwned_new(context, &buffer_owned, symbol_size);
  if (error != ZP_NOERROR) {
    return error;
  }

  ZP_BufferOwned_copy_data(context, &buffer_owned, symbol_data);

  /* next, make the symbol object itself */
  ZP_FixedLocation_t new_symbol_location;
  error = ZP_FixedAllocator_alloc(&context->fixed_allocator, &new_symbol_location);
  if (error != ZP_NOERROR) {
    ZP_BufferOwned_decref(context, &buffer_owned);
    return error;
  }

  ZP_Tagged_t symbol_number;
  error = ZP_Tagged_from_int(&symbol_number, &number_to_use);
  if (error != ZP_NOERROR) {
    ZP_BufferOwned_decref(context, &buffer_owned);
    ZP_FixedAllocator_free(&context->fixed_allocator, &new_symbol_location);
    return error;
  }

  ZP_FixedAllocator_set(
    &context->fixed_allocator,
    &new_symbol_location,
    buffer_owned.data,
    symbol_number.data
  );

  ZP_Tagged_t new_symbol;
  ZP_Tagged_from_FixedLocation(
    &new_symbol,
    &new_symbol_location,
    ZP_TAG_MASK_SYMBOL
  );

  error = ZP_Context_incref(context, &new_symbol);
  if (error != ZP_NOERROR) {
    ZP_BufferOwned_decref(context, &buffer_owned);
    ZP_FixedAllocator_free(&context->fixed_allocator, &new_symbol_location);
    return error;
  }

  /* finally, push it to the end of the symbol_set list */
  ZP_Tagged_t current_list_end;
  ZP_Tagged_copy(&current_list_end, &context->symbol_set.list);

  ZP_Tagged_t new_list_end;
  error = ZP_List_push(context, &new_list_end, &current_list_end, &new_symbol);
  if (error != ZP_NOERROR) {
    ZP_BufferOwned_decref(context, &buffer_owned);
    ZP_FixedAllocator_free(&context->fixed_allocator, &new_symbol_location);
    ZP_Context_decref(context, &new_symbol);
    return error;
  }

  /* this is an unusual list, inaccessible from outside, so it should not be incref'ed */
  if (!ZP_List_is_empty(&current_list_end)) {
    ZP_FixedLocation_t old_list_end_location;
    ZP_Tagged_to_FixedLocation(&old_list_end_location, &current_list_end);
    context->refcount[old_list_end_location.at >> 1]--;
  }

  ZP_Tagged_copy(tagged, &new_symbol);
  ZP_Tagged_copy(&context->symbol_set.list, &new_list_end);

  if (!duplicate) {
    /* this symbol used up one integer value (non-renewable) */
    context->symbol_set.next_number++;
  }

  return ZP_NOERROR;
}

/***** BufferOwned ***********************************************************/

void ZP_BufferOwned_decref(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged
) {
  ZP_FixedLocation_t location;
  ZP_Tagged_to_FixedLocation(&location, tagged);
  context->refcount[location.at >> 1]--;

  if (context->refcount[location.at >> 1] == 0) {
    /* children are RegionAllocator-tracked, not FixedAllocator-tracked */
    ZP_Index_t pointer;
    ZP_Index_t request_size;
    ZP_FixedAllocator_get(
      &context->fixed_allocator,
      &location,
      &pointer,
      &request_size
    );
    ZP_RegionAllocator_free(&context->region_allocator, pointer, request_size);
  }
}

ZP_Error_t ZP_BufferOwned_new(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged,
  ZP_Index_t request_size
) {
  if (request_size < 0) {
    return "ZP_BufferOwned_new(_, _, request_size) must be non-negative";
  }

  ZP_FixedLocation_t fixed_location;
  ZP_Error_t error = ZP_FixedAllocator_alloc(&context->fixed_allocator, &fixed_location);
  if (error != ZP_NOERROR) {
    return error;
  }

  ZP_Index_t pointer;
  error = ZP_RegionAllocator_alloc(&context->region_allocator, request_size, &pointer);
  if (error != ZP_NOERROR) {
    ZP_FixedAllocator_free(&context->fixed_allocator, &fixed_location);
    return error;
  }

  ZP_FixedAllocator_set(&context->fixed_allocator, &fixed_location, pointer, request_size);

  ZP_Tagged_from_FixedLocation(tagged, &fixed_location, ZP_TAG_MASK_BUFFER_OWNED);

  error = ZP_Context_incref(context, tagged);
  if (error != ZP_NOERROR) {
    ZP_FixedAllocator_free(&context->fixed_allocator, &fixed_location);
    ZP_RegionAllocator_free(&context->region_allocator, pointer, request_size);
    return error;
  }

  return ZP_NOERROR;
}

void ZP_BufferOwned_copy_data(
  ZP_Context_t* context,
  ZP_Tagged_t* buffer,
  const char* data
) {
  ZP_FixedLocation_t buffer_location;
  buffer_location.at = ZP_Tagged_to_raw(buffer);

  ZP_Index_t pointer;
  ZP_Index_t request_size;
  ZP_FixedAllocator_get(&context->fixed_allocator, &buffer_location, &pointer, &request_size);

  ZP_RegionAllocator_copy_data(&context->region_allocator, pointer, request_size, data);
}

/***** BufferView ************************************************************/

void ZP_BufferView_decref_node(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged
) {
  ZP_FixedLocation_t location;
  ZP_Tagged_to_FixedLocation(&location, tagged);
  context->refcount[location.at >> 1]--;
  /* children are neither FixedAllocator-tracked nor RegionAllocator-tracked */
}

void ZP_BufferView_decref(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged
) {
  ZP_FixedLocation_t location;
  ZP_Tagged_to_FixedLocation(&location, tagged);
  context->refcount[location.at >> 1]--;
  if (context->refcount[location.at >> 1] == 0) {
    ZP_Context_decref_children(context, &location, true, true);
  }
}

ZP_Error_t ZP_BufferView_new(
  ZP_Context_t* context,
  ZP_Tagged_t* output,
  ZP_Tagged_t* base,
  ZP_Index_t start,
  ZP_Index_t stop
) {
  ZP_Index_t base_tag = ZP_Tagged_tag_mask_of(base);
  if (
    base_tag != ZP_TAG_MASK_BUFFER_OWNED
    &&  base_tag != ZP_TAG_MASK_BUFFER_VIEW_WRAP
  ) {
    return "ZP_BufferView_new(_, _, base, _, _) must be a buffer or symbol";
  }

  ZP_Tagged_t base_increfed;
  ZP_Tagged_copy(&base_increfed, base);
  ZP_Error_t error = ZP_Context_incref(context, &base_increfed);
  if (error != ZP_NOERROR) {
    return error;
  }

  ZP_FixedLocation_t buffer_view_location;
  error = ZP_FixedAllocator_alloc(&context->fixed_allocator, &buffer_view_location);
  if (error != ZP_NOERROR) {
    ZP_List_decref(context, &base_increfed);
    return error;
  }
  ZP_Tagged_t tagged_buffer_view;
  ZP_Tagged_from_FixedLocation(
    &tagged_buffer_view,
    &buffer_view_location,
    ZP_TAG_MASK_BUFFER_VIEW
  );

  error = ZP_Context_incref(context, &tagged_buffer_view);
  if (error != ZP_NOERROR) {
    ZP_List_decref(context, &base_increfed);
    ZP_List_decref(context, &tagged_buffer_view);
    return error;
  }

  ZP_Index_t base_pointer;
  ZP_Index_t base_size;
  if (base_tag == ZP_TAG_BUFFER_OWNED) {
    ZP_FixedLocation_t base_location;
    base_location.at = ZP_Tagged_to_raw(base);
    ZP_FixedAllocator_get(
      &context->fixed_allocator,
      &base_location,
      &base_pointer,
      &base_size
    );
  }
  else {
    ZP_FixedLocation_t base_location;
    base_location.at = ZP_Tagged_to_raw(base);
    ZP_Tagged_t base_base;
    ZP_Tagged_t base_node;
    ZP_FixedAllocator_get(
      &context->fixed_allocator,
      &base_location,
      &base_base.data,
      &base_node.data
    );
    ZP_FixedLocation_t base_node_location;
    base_node_location.at = ZP_Tagged_to_raw(&base_node);
    ZP_FixedAllocator_get(
      &context->fixed_allocator,
      &base_node_location,
      &base_pointer,
      &base_size
    );
  }

  ZP_Index_t new_pointer;
  ZP_Index_t new_size;
  ZP_slice(&new_pointer, &new_size, base_pointer, base_size, start, stop);
  ZP_FixedAllocator_set(
    &context->fixed_allocator,
    &buffer_view_location,
    new_pointer,
    new_size
  );

  ZP_FixedLocation_t buffer_wrap_location;
  error = ZP_FixedAllocator_alloc(&context->fixed_allocator, &buffer_wrap_location);
  if (error != ZP_NOERROR) {
    ZP_List_decref(context, &base_increfed);
    ZP_List_decref(context, &tagged_buffer_view);
    ZP_FixedAllocator_free(&context->fixed_allocator, &buffer_view_location);
    return error;
  }

  ZP_FixedAllocator_set(
    &context->fixed_allocator,
    &buffer_wrap_location,
    base_increfed.data,
    tagged_buffer_view.data
  );

  ZP_Tagged_from_FixedLocation(
    output,
    &buffer_wrap_location,
    ZP_TAG_MASK_BUFFER_VIEW_WRAP
  );

  error = ZP_Context_incref(context, output);
  if (error != ZP_NOERROR) {
    ZP_List_decref(context, &base_increfed);
    ZP_List_decref(context, &tagged_buffer_view);
    ZP_FixedAllocator_free(&context->fixed_allocator, &buffer_view_location);
    ZP_List_decref(context, output);
    return error;
  }

  return ZP_NOERROR;
}

/***** List ******************************************************************/

void ZP_List_decref(
  ZP_Context_t* context,
  ZP_Tagged_t* tagged
) {
  ZP_FixedLocation_t location;
  ZP_Tagged_to_FixedLocation(&location, tagged);
  if (location.at != ZP_TERMINATOR) {
    context->refcount[location.at >> 1]--;
    if (context->refcount[location.at >> 1] == 0) {
      ZP_Context_decref_children(context, &location, true, true);
    }
  }
}

void ZP_List_new_empty(
  ZP_Tagged_t* list
) {
  list->data = ZP_TERMINATOR | ZP_TAG_MASK_LIST;
}

bool ZP_List_is_empty(
  ZP_Tagged_t* list
) {
  return list->data == (ZP_TERMINATOR | ZP_TAG_MASK_LIST);
}

ZP_Error_t ZP_List_push(
  ZP_Context_t* context,
  ZP_Tagged_t* output_list,
  ZP_Tagged_t* input_list,
  ZP_Tagged_t* item
) {
  if (ZP_Tagged_tag_mask_of(input_list) != ZP_TAG_MASK_LIST) {
    return "ZP_List_push(_, _, input_list, _) must be a list";
  }

  ZP_Tagged_t input_list_increfed;
  ZP_Tagged_copy(&input_list_increfed, input_list);
  ZP_Error_t error = ZP_Context_incref(context, &input_list_increfed);
  if (error != ZP_NOERROR) {
    return error;
  }

  ZP_Tagged_t item_increfed;
  ZP_Tagged_copy(&item_increfed, item);
  error = ZP_Context_incref(context, &item_increfed);
  if (error != ZP_NOERROR) {
    ZP_List_decref(context, &input_list_increfed);
    return error;
  }

  ZP_FixedLocation_t fixed_location;
  error = ZP_FixedAllocator_alloc(&context->fixed_allocator, &fixed_location);
  if (error != ZP_NOERROR) {
    ZP_List_decref(context, &input_list_increfed);
    ZP_List_decref(context, &item_increfed);
    return error;
  }

  ZP_FixedAllocator_set(
    &context->fixed_allocator,
    &fixed_location,
    input_list_increfed.data,
    item_increfed.data
  );

  ZP_Tagged_from_FixedLocation(output_list, &fixed_location, ZP_TAG_MASK_LIST);
  ZP_Context_refcnt_init(context, &fixed_location);

  return ZP_NOERROR;
}

ZP_Error_t ZP_List_pop(
  ZP_Context_t* context,
  ZP_Tagged_t* output_list,
  ZP_Tagged_t* output_item,
  ZP_Tagged_t* input_list
) {
  if (ZP_Tagged_tag_mask_of(input_list) != ZP_TAG_MASK_LIST) {
    return "ZP_List_pop(_, _, _, input_list) must be a list";
  }

  ZP_FixedLocation_t input_list_location;
  input_list_location.at = ZP_Tagged_to_raw(input_list);
  if (input_list_location.at == ZP_TERMINATOR) {
    return "cannot pop empty list";
  }

  ZP_FixedAllocator_get(
    &context->fixed_allocator,
    &input_list_location,
    &output_list->data,
    &output_item->data
  );

  ZP_Error_t error = ZP_Context_incref(context, output_list);
  if (error != ZP_NOERROR) {
    return error;
  }
  error = ZP_Context_incref(context, output_item);
  if (error != ZP_NOERROR) {
    ZP_List_decref(context, output_list);
    return error;
  }

  return ZP_NOERROR;
}

/***** Array *****************************************************************/
