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

  /* not an ordinary list: mutable, references managed globally, items are RegionPointers */
  ZP_List_new_empty(&context->symbol_set);

  return ZP_NO_ERROR;
}

ZP_Error_t ZP_Context_get_symbol(
  ZP_Context_t* context,
  ZP_Tagged_t* symbol,
  const char* symbol_data,
  ZP_Index_t symbol_size
) {
  ZP_Tagged_t this_list;
  ZP_Tagged_copy(&this_list, &context->symbol_set);

  ZP_Error_t error;
  ZP_Tagged_t next_list;
  ZP_Tagged_t this_symbol;

  while (!ZP_List_is_empty(&this_list)) {
    ZP_List_peek_reverse_iterate_step(context, &next_list, &this_symbol, &this_list);

    ZP_SlabPointer_t this_symbol_pointer;
    ZP_Tagged_to_SlabPointer(&this_symbol_pointer, &this_symbol);

    ZP_RegionPointer_t this_pointer;
    this_pointer.at = ZP_Context_get_right(context, &this_symbol_pointer);

    /* check to see if the symbol at this position in the list is the desired one */
    ZP_Index_t this_size = ZP_Context_region_size(context, &this_pointer);
    if (this_size == symbol_size) {
      const char* this_data = (const char*)ZP_Context_region_at(context, &this_pointer);
      bool same = true;
      ZP_Index_t i;
      for (i = 0;  i < this_size;  i++) {
        if (this_data[i] != symbol_data[i]) {
          same = false;
          break;
        }
      }
      if (same) {
        /* it is! return it */
        return ZP_Context_slab_incref(context, symbol, &this_symbol);
      }
    }

    this_list.data = next_list.data;
  }

  /* the desired symbol was not found in the list; create it and push it into the list */

  ZP_RegionPointer_t region_pointer;
  error = ZP_Context_new_region(context, &region_pointer, symbol_size);
  if (error != ZP_NO_ERROR) {
    return error;
  }

  ZP_Context_region_copy_data(context, &region_pointer, symbol_data);

  error = ZP_Symbol_new(context, symbol, &region_pointer);
  if (error != ZP_NO_ERROR) {
    ZP_Context_region_decref(context, &region_pointer);
    return error;
  }

  ZP_Tagged_t new_list;
  error = ZP_List_push(context, &new_list, &context->symbol_set, symbol);
  if (error != ZP_NO_ERROR) {
    ZP_Context_region_decref(context, &region_pointer);
    ZP_Symbol_del(context, symbol);
    return error;
  }

  /* "internal" symbol_set list nodes and Symbol references don't count */
  /* only the (single, remaining) reference to the Symbol this function returns */
  ZP_Context_slab_decref(context, symbol);
  ZP_Context_slab_decref(context, &context->symbol_set);

  context->symbol_set.data = new_list.data;

  return ZP_NO_ERROR;
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

void ZP_Context_free_slab(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer
) {
  ZP_SlabAllocator_free(&context->slab_allocator, pointer);
  context->refcnt[pointer->at] = 0;
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

void ZP_Context_set_left(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t left
) {
  ZP_SlabAllocator_set_left(&context->slab_allocator, pointer, left);
}

void ZP_Context_set_right(
  ZP_Context_t* context,
  ZP_SlabPointer_t* pointer,
  ZP_Index_t right
) {
  ZP_SlabAllocator_set_right(&context->slab_allocator, pointer, right);
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

ZP_Error_t ZP_Context_slab_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_tagged,
  ZP_Tagged_t* old_tagged,
  bool incref_left,
  bool incref_right
) {
  ZP_SlabPointer_t new_pointer;
  ZP_Error_t error = ZP_Context_new_slab(context, &new_pointer);
  if (error != ZP_NO_ERROR) {
    return error;
  }

  ZP_SlabPointer_t old_pointer;
  ZP_Tagged_to_SlabPointer(&old_pointer, old_tagged);

  ZP_Tagged_t left;
  ZP_Tagged_t right;
  ZP_Context_get(context, &old_pointer, &left.data, &right.data);

  ZP_Tagged_t new_left;
  if (incref_left) {
    error = ZP_Context_slab_incref(context, &new_left, &left);
    if (error != ZP_NO_ERROR) {
      ZP_Context_free_slab(context, &new_pointer);
      return error;
    }
  }
  else {
    ZP_Tagged_copy(&new_left, &left);
  }

  ZP_Tagged_t new_right;
  if (incref_right) {
    error = ZP_Context_slab_incref(context, &new_right, &right);
    if (error != ZP_NO_ERROR) {
      ZP_Context_free_slab(context, &new_pointer);
      if (incref_left) {
        ZP_Context_slab_decref(context, &new_left);
      }
      return error;
    }
  }
  else {
    ZP_Tagged_copy(&new_right, &right);
  }

  ZP_Context_set(context, &new_pointer, new_left.data, new_right.data);

  ZP_Tagged_from_SlabPointer(
    new_tagged,
    &new_pointer,
    old_tagged->data & ZP_TAG_OBJECT_MASK
  );

  return ZP_NO_ERROR;
}

ZP_RefCnt_t ZP_Context_slab_refcnt(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
) {
  return context->refcnt[pointer->at];
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
        /* fast path: just increment the refcnt */
        context->refcnt[pointer]++;
      }
      else {
        /* slow path: need to copy (rarely) */
        return ZP_List_copy(context, new_tagged, old_tagged);
      }
    }
    break;

  case ZP_TAG_OBJECT_INTERNAL:
    pointer = old_tagged->data & (~ZP_TAG_OBJECT_MASK);
    if (context->refcnt[pointer] != ZP_REFCNT_MAX) {
      /* fast path: just increment the refcnt */
      context->refcnt[pointer]++;
    }
    else {
      /* FIXME: handle this */
    }
    break;

  case ZP_TAG_OBJECT_OTHER:
    pointer = old_tagged->data & (~ZP_TAG_OBJECT_MASK);
    if (context->refcnt[pointer] != ZP_REFCNT_MAX) {
      /* fast path: just increment the refcnt */
      context->refcnt[pointer]++;
    }
    else {
      /* slow path: need to copy (rarely) */
      ZP_Tagged_t object_type;
      ZP_Context_object_type(context, &object_type, old_tagged);

      if (object_type.data & ZP_TAG_OBJECT_OTHER) {
        ZP_Tagged_t class_type;
        ZP_Context_object_type(context, &class_type, &object_type);

        switch (class_type.data) {
        case ZP_ENUM_OTHER_SIMPLE_CLASS:
          return ZP_SimpleClass_instance_copy(context, new_tagged, old_tagged);
        /* FIXME: handle more types, as they get defined */
        }
      }
      else {
        switch (ZP_Tagged_to_enum(&object_type)) {
        case ZP_ENUM_OTHER_SYMBOL:
          return ZP_Symbol_copy(context, new_tagged, old_tagged);
        case ZP_ENUM_OTHER_RANDOM_ACCESS_LIST:
          return ZP_RandomAccessList_copy(context, new_tagged, old_tagged);
        case ZP_ENUM_OTHER_SIMPLE_CLASS:
          return ZP_SimpleClass_copy(context, new_tagged, old_tagged);
        /* FIXME: handle more types, as they get defined */
        }
      }
    }
    break;
  }

  new_tagged->data = old_tagged->data;
  return ZP_NO_ERROR;
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

  case ZP_TAG_OBJECT_INTERNAL:
    pointer = tagged->data & (~ZP_TAG_OBJECT_MASK);
    context->refcnt[pointer]--;

    if (context->refcnt[pointer] == 0) {
      /* FIXME: handle this */
    }
    break;

  case ZP_TAG_OBJECT_OTHER:
    pointer = tagged->data & (~ZP_TAG_OBJECT_MASK);
    context->refcnt[pointer]--;

    if (context->refcnt[pointer] == 0) {
      ZP_Tagged_t object_type;
      ZP_Context_object_type(context, &object_type, tagged);

      if (object_type.data & ZP_TAG_OBJECT_OTHER) {
        ZP_Tagged_t class_type;
        ZP_Context_object_type(context, &class_type, &object_type);

        switch (class_type.data) {
        case ZP_ENUM_OTHER_SIMPLE_CLASS:
          ZP_SimpleClass_instance_del(context, tagged);
          break;
        /* FIXME: handle more types, as they get defined */
        }
      }
      else {
        switch (ZP_Tagged_to_enum(&object_type)) {
        case ZP_ENUM_OTHER_SYMBOL:
          ZP_Symbol_del(context, tagged);
          break;
        case ZP_ENUM_OTHER_RANDOM_ACCESS_LIST:
          ZP_RandomAccessList_del(context, tagged);
          break;
        case ZP_ENUM_OTHER_SIMPLE_CLASS:
          ZP_SimpleClass_del(context, tagged);
          break;
        /* FIXME: handle more types, as they get defined */
        }
      }
    }
    break;
  }
}

ZP_RefCnt_t ZP_Context_internal_refcnt(
  ZP_Context_t* context,
  ZP_Index_t internal
) {
  return context->refcnt[internal];
}

void ZP_Context_internal_incref(
  ZP_Context_t* context,
  ZP_Index_t internal
) {
  context->refcnt[internal]++;
}

void ZP_Context_internal_decref(
  ZP_Context_t* context,
  ZP_Index_t internal
) {
  context->refcnt[internal]--;
}

ZP_Error_t ZP_Context_new_region(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer,
  ZP_Index_t size
) {
  return ZP_RegionAllocator_alloc(&context->region_allocator, pointer, size);
}

ZP_Index_t ZP_Context_region_refcnt(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
) {
  return ZP_RegionAllocator_refcnt(&context->region_allocator, pointer);
}

ZP_Error_t ZP_Context_region_incref(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
) {
  return ZP_RegionAllocator_incref(&context->region_allocator, pointer);
}

void ZP_Context_region_decref(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
) {
  ZP_RegionAllocator_decref(&context->region_allocator, pointer);
}

ZP_Index_t ZP_Context_region_size(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
) {
  return ZP_RegionAllocator_size(&context->region_allocator, pointer);
}

void* ZP_Context_region_at(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer
) {
  return ZP_RegionAllocator_at(&context->region_allocator, pointer);
}

void ZP_Context_region_copy_data(
  ZP_Context_t* context,
  ZP_RegionPointer_t* pointer,
  const char* data
) {
  ZP_RegionAllocator_copy_data(&context->region_allocator, pointer, data);
}

void ZP_Context_object_type(
  ZP_Context_t* context,
  ZP_Tagged_t* object_type,
  ZP_Tagged_t* object
) {
  ZP_SlabPointer_t pointer;
  ZP_Tagged_to_SlabPointer(&pointer, object);

  object_type->data = ZP_Context_get_left(context, &pointer);
}
