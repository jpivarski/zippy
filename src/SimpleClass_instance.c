#include <zippy/data.h>
#include <zippy/util.h>

/***** helper (private) functions ********************************************/

void ZP_SimpleClass_instance_decref_walk(
  ZP_Context_t* context,
  ZP_Index_t start,
  ZP_Index_t stop,
  ZP_Tagged_t* internal
) {
  if (start >= stop) {
    return;
  }

  ZP_SlabPointer_t internal_pointer;
  ZP_Tagged_to_SlabPointer(&internal_pointer, internal);

  context->refcnt[internal_pointer.at]--;
  if (context->refcnt[internal_pointer.at] != 0) {
    return;
  }

  ZP_Tagged_t left;
  ZP_Tagged_t right;
  ZP_SlabAllocator_get(&context->slab_allocator, &internal_pointer, &left.data, &right.data);

  ZP_SlabAllocator_free(&context->slab_allocator, &internal_pointer);
  context->refcnt[internal_pointer.at] = 0;

  if (start + 1 == stop) {
    ZP_Context_slab_decref(context, &left);
  }
  else if (start + 2 == stop) {
    ZP_Context_slab_decref(context, &left);
    ZP_Context_slab_decref(context, &right);
  }
  else {
    ZP_Index_t step = ZP_util_next_power_2((stop - start) / 2);
    ZP_SimpleClass_instance_decref_walk(context, start, start + step, &left);
    ZP_SimpleClass_instance_decref_walk(context, start + step, stop, &right);
  }
}

ZP_Error_t ZP_SimpleClass_instance_new_tree(
  ZP_Context_t* context,
  ZP_Tagged_t* internal,
  ZP_Index_t start,
  ZP_Index_t stop,
  ZP_Index_t* scratch_space_filled,
  ZP_Index_t* scratch_space,
  ZP_Index_t* sorted_data
) {
  if (start >= stop) {
    ZP_Context_new_empty_internal(internal);
    return ZP_NO_ERROR;
  }

  ZP_SlabPointer_t internal_pointer;
  ZP_Error_t error = ZP_SlabAllocator_alloc(&context->slab_allocator, &internal_pointer);
  if (error != ZP_NO_ERROR) {
    return error;
  }
  context->refcnt[internal_pointer.at] = 1;
  scratch_space[(*scratch_space_filled)] = internal_pointer.at;
  (*scratch_space_filled)++;

  ZP_Tagged_from_SlabPointer(internal, &internal_pointer, ZP_TAG_NODE_INTERNAL);

  ZP_Tagged_t left;
  ZP_Tagged_t right;
  if (start + 1 == stop) {
    left.data = sorted_data[start];
    ZP_Context_new_empty_internal(&right);
  }
  else if (start + 2 == stop) {
    left.data = sorted_data[start];
    right.data = sorted_data[start + 1];
  }
  else {
    ZP_Index_t step = ZP_util_next_power_2((stop - start) / 2);
    error = ZP_SimpleClass_instance_new_tree(
      context,
      &left,
      start,
      start + step,
      scratch_space_filled,
      scratch_space,
      sorted_data
    );
    if (error != ZP_NO_ERROR) {
      return error;
    }
    error = ZP_SimpleClass_instance_new_tree(
      context,
      &right,
      start + step,
      stop,
      scratch_space_filled,
      scratch_space,
      sorted_data
    );
    if (error != ZP_NO_ERROR) {
      return error;
    }
  }

  ZP_SlabAllocator_set(&context->slab_allocator, &internal_pointer, left.data, right.data);

  return ZP_NO_ERROR;
}

void ZP_SimpleClass_instance_search_for_member(
  ZP_Context_t* context,
  ZP_Tagged_t* member,
  ZP_Index_t start,
  ZP_Index_t stop,
  ZP_Index_t* symbol_identifiers,
  ZP_Index_t symbol_identifier,
  ZP_Tagged_t* internal
) {
  if (start >= stop) {
    return;
  }

  ZP_SlabPointer_t internal_pointer;
  ZP_Tagged_to_SlabPointer(&internal_pointer, internal);

  ZP_Tagged_t left;
  ZP_Tagged_t right;
  ZP_SlabAllocator_get(&context->slab_allocator, &internal_pointer, &left.data, &right.data);

  if (start + 1 == stop) {
    if (symbol_identifier == symbol_identifiers[start]) {
      member->data = left.data;
    }
    return;
  }
  else if (start + 2 == stop) {
    if (symbol_identifier == symbol_identifiers[start]) {
      member->data = left.data;
    }
    if (symbol_identifier == symbol_identifiers[start + 1]) {
      member->data = right.data;
    }
    return;
  }
  else {
    ZP_Index_t step = ZP_util_next_power_2((stop - start) / 2);
    if (symbol_identifier < symbol_identifiers[start + step]) {
      ZP_SimpleClass_instance_search_for_member(
        context,
        member,
        start,
        start + step,
        symbol_identifiers,
        symbol_identifier,
        &left
      );
    }
    else {
      ZP_SimpleClass_instance_search_for_member(
        context,
        member,
        start + step,
        stop,
        symbol_identifiers,
        symbol_identifier,
        &right
      );
    }
    return;
  }
}

/***** public functions ******************************************************/

ZP_Error_t ZP_SimpleClass_instance_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_instance,
  ZP_Tagged_t* old_instance
) {
  return "not implemented SimpleClass_instance 1";
}

void ZP_SimpleClass_instance_del(
  ZP_Context_t* context,
  ZP_Tagged_t* instance
) {
  ZP_SlabPointer_t pointer;
  ZP_Tagged_to_SlabPointer(&pointer, instance);

  ZP_Tagged_t simple_class;
  ZP_Tagged_t internal;
  ZP_SlabAllocator_get(&context->slab_allocator, &pointer, &simple_class.data, &internal.data);

  ZP_Index_t num_members = ZP_SimpleClass_num_members(context, &simple_class);

  ZP_Context_slab_decref(context, &simple_class);

  ZP_SimpleClass_instance_decref_walk(
    context,
    0,
    num_members,
    &internal
  );

  ZP_SlabAllocator_free(&context->slab_allocator, &pointer);
  context->refcnt[pointer.at] = 0;
}

ZP_Error_t ZP_SimpleClass_instance_new(
  ZP_Context_t* context,
  ZP_Tagged_t* instance,
  ZP_Tagged_t* simple_class,
  ZP_Tagged_t* members
) {
  ZP_SlabPointer_t pointer;
  ZP_Error_t error = ZP_SlabAllocator_alloc(&context->slab_allocator, &pointer);
  if (error != ZP_NO_ERROR) {
    return error;
  }
  context->refcnt[pointer.at] = 1;

  ZP_Tagged_t simple_class_increfed;
  error = ZP_Context_slab_incref(context, &simple_class_increfed, simple_class);
  if (error != ZP_NO_ERROR) {
    ZP_SlabAllocator_free(&context->slab_allocator, &pointer);
    context->refcnt[pointer.at] = 0;
    return error;
  }

  ZP_Index_t num_members;
  ZP_Index_t* scratch_space;
  ZP_Index_t* sorted_data;
  error = ZP_SimpleClass_load_members(
    context,
    &num_members,
    &scratch_space,
    &sorted_data,
    simple_class,
    members
  );
  if (error != ZP_NO_ERROR) {
    ZP_SlabAllocator_free(&context->slab_allocator, &pointer);
    context->refcnt[pointer.at] = 0;
    ZP_Context_slab_decref(context, &simple_class_increfed);
    return error;
  }

  ZP_Index_t scratch_space_filled = 0;
  ZP_Tagged_t internal;
  error = ZP_SimpleClass_instance_new_tree(
    context,
    &internal,
    0,
    num_members,
    &scratch_space_filled,
    scratch_space,
    sorted_data
  );
  if (error != ZP_NO_ERROR) {
    ZP_SlabPointer_t internal_pointer;
    ZP_Tagged_t this_member;
    ZP_Index_t i;
    for (i = 0;  i < scratch_space_filled;  i++) {
      internal_pointer.at = scratch_space[i];
      ZP_SlabAllocator_free(&context->slab_allocator, &internal_pointer);
      context->refcnt[internal_pointer.at] = 0;
    }
    for (i = 0;  i < num_members;  i++) {
      this_member.data = sorted_data[i];
      ZP_Context_slab_decref(context, &this_member);
    }
    ZP_SlabAllocator_free(&context->slab_allocator, &pointer);
    context->refcnt[pointer.at] = 0;
    ZP_Context_slab_decref(context, &simple_class_increfed);
    return error;
  }

  ZP_SlabAllocator_set(&context->slab_allocator, &pointer, simple_class_increfed.data, internal.data);

  ZP_Tagged_from_SlabPointer(instance, &pointer, ZP_TAG_NODE_OBJECT);

  return ZP_NO_ERROR;
}

void ZP_SimpleClass_instance_get_member(
  ZP_Context_t* context,
  ZP_Tagged_t* member,
  ZP_Tagged_t* instance,
  ZP_Tagged_t* symbol
) {
  ZP_SlabPointer_t pointer;
  ZP_Tagged_to_SlabPointer(&pointer, instance);

  ZP_Tagged_t simple_class;
  ZP_Tagged_t internal;
  ZP_SlabAllocator_get(&context->slab_allocator, &pointer, &simple_class.data, &internal.data);

  ZP_Index_t num_members;
  ZP_Index_t* symbol_identifiers;
  ZP_SimpleClass_symbol_identifiers(context, &num_members, &symbol_identifiers, &simple_class);

  member->data = ZP_TERMINATOR;

  ZP_Index_t symbol_identifier = ZP_Symbol_identifier(context, symbol);

  ZP_SimpleClass_instance_search_for_member(
    context,
    member,
    0,
    num_members,
    symbol_identifiers,
    symbol_identifier,
    &internal
  );
}
