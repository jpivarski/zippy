#include <zippy/data.h>

/***** helper (private) functions ********************************************/

/***** public functions ******************************************************/

ZP_Error_t ZP_Symbol_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* new_symbol,
  ZP_Tagged_t* old_symbol
) {
  return "not implemented Symbol 1";
}

void ZP_Symbol_del(
  ZP_Context_t* context,
  ZP_Tagged_t* symbol
) {
  ZP_Tagged_t this_list;
  ZP_Tagged_copy(&this_list, &context->symbol_set);

  ZP_Tagged_t next_list;
  ZP_Tagged_t this_symbol;
  while (!ZP_List_is_empty(&this_list)) {
    ZP_List_peek_reverse_iterate_step(context, &next_list, &this_symbol, &this_list);

    if (symbol->data == this_symbol.data) {
      printf("found it\n");


      return;
    }

    this_list.data = next_list.data;
  }
}

ZP_Error_t ZP_Symbol_new(
  ZP_Context_t* context,
  ZP_Tagged_t* symbol,
  ZP_RegionPointer_t* region_pointer
) {
  ZP_SlabPointer_t pointer;
  ZP_Error_t error = ZP_Context_new_slab(context, &pointer);
  if (error != ZP_NO_ERROR) {
    return error;
  }

  ZP_Tagged_t other_type;
  ZP_Tagged_from_enum(&other_type, ZP_ENUM_OTHER_SYMBOL);

  ZP_Context_set(context, &pointer, other_type.data, region_pointer->at);

  ZP_Tagged_from_SlabPointer(symbol, &pointer, ZP_TAG_OBJECT_OTHER);

  return ZP_NO_ERROR;
}
