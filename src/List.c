#include <zippy/data.h>

/***** helper (private) functions ********************************************/

/***** public functions ******************************************************/

ZP_Error_t ZP_List_copy(
  ZP_Context_t* context,
  ZP_Tagged_t* list
) {
  return "not implemented List 1";
}

void ZP_List_del(
  ZP_Context_t* context,
  ZP_Tagged_t* list
) {

}

void ZP_List_new_empty(
  ZP_Tagged_t* list
) {
  list->data = ZP_TERMINATOR | ZP_TAG_OBJECT_LIST;
}

bool ZP_List_is_empty(
  ZP_Tagged_t* list
) {
  return list->data == (ZP_TERMINATOR | ZP_TAG_OBJECT_LIST);
}

ZP_Index_t ZP_List_size(
  ZP_Context_t* context,
  ZP_Tagged_t* list
) {
  ZP_Tagged_t this_list;
  ZP_Tagged_copy(&this_list, list);
  ZP_SlabPointer_t pointer;

  ZP_Index_t size = 0;
  while (!ZP_List_is_empty(&this_list)) {
    ZP_Tagged_to_SlabPointer(&pointer, &this_list);
    this_list.data = ZP_Context_get_left(context, &pointer);
    size++;
  }

  return size;
}

void ZP_List_peek_iterate(
  ZP_Context_t* context,
  ZP_Tagged_t* next_list,
  ZP_Tagged_t* this_item,
  ZP_Tagged_t* current_list
) {
  ZP_SlabPointer_t pointer;
  ZP_Tagged_to_SlabPointer(&pointer, current_list);
  ZP_Context_get(context, &pointer, &next_list->data, &this_item->data);
}

ZP_Error_t ZP_List_push(
  ZP_Context_t* context,
  ZP_Tagged_t* output_list,
  ZP_Tagged_t* input_list,
  ZP_Tagged_t* item
) {
  return "not implemented List 2";
}
