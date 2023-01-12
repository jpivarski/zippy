#include <stdlib.h>
/* #include <stdio.h> */

#include <zippy/data.h>

#define handle_error(error)                                              \
  if (error != ZP_NO_ERROR) {                                            \
    return -1;                                                           \
  }                                                                      \

int main(int argc, char** argv) {
  /* 2**18 == 262144 refcnt-able objects */

  /* ZIPPY_POINTER_SIZE == 32 and ZIPPY_REFCNT_SIZE == 32 imply */
  /*     - slab_size = 2 * sizeof(ZP_Index_t) * 2**18 = 2097152 bytes */
  /*     - refcnt_size = sizeof(ZP_RefCnt_t) * 2**18 = 1048576 bytes */
  /*     - region_size = another 1048576 would make everything fit into */
  /*   ============================= */
  /*   4 MiB = 4194304 (my L2 cache) */

  size_t slab_size = 2097152;
  size_t refcnt_size = slab_size * sizeof(ZP_RefCnt_t) / sizeof(ZP_Index_t) / 2;
  size_t region_size = 1048576;

  void* arena = malloc(4194304);
  void* slab_arena = arena;
  void* refcnt_arena = arena + slab_size;
  void* region_arena = arena + slab_size + refcnt_size;

  ZP_Context_t context;
  ZP_Error_t error = ZP_Context_new(
    &context,
    slab_arena,
    region_arena,
    refcnt_arena,
    slab_size,
    region_size,
    refcnt_size
  );
  handle_error(error);

  /* ZP_Index_t i; */
  /* ZP_Index_t j; */
  /* ZP_Index_t pointer; */
  /* ZP_Index_t dummy1 = 0; */
  /* ZP_Index_t dummy2 = 10000; */
  /* for (i = 0;  i < 420000;  i++) { */
  /*   for (j = 0;  j < 1000;  j++) { */
  /*     if (context.slab_allocator.first_free == ZP_END_OF_FREE) { */
  /*       if (context.slab_allocator.bump_free == context.slab_allocator.num_slabs) { */
  /*         return -1; */
  /*       } */
  /*       else { */
  /*         pointer = context.slab_allocator.bump_free; */
  /*         context.slab_allocator.bump_free++; */
  /*       } */
  /*     } */
  /*     else { */
  /*       pointer = context.slab_allocator.first_free; */
  /*       context.slab_allocator.first_free = context.slab_allocator.arena[2 * context.slab_allocator.first_free]; */
  /*     } */

  /*     context.refcnt[pointer]++; */

  /*     dummy1++; */
  /*     dummy2++; */
  /*     context.slab_allocator.arena[2 * pointer] = dummy1; */
  /*     context.slab_allocator.arena[2 * pointer + 1] = dummy2; */
  /*   } */
  /*   for (j = 0;  j < 1000;  j++) { */
  /*     pointer = j; */
  /*     context.refcnt[pointer]--; */

  /*     context.slab_allocator.arena[2 * pointer] = context.slab_allocator.first_free; */
  /*     context.slab_allocator.first_free = pointer; */
  /*   } */
  /* } */

  /* return 0; */

  ZP_Tagged_t symbol1;
  const char* symbol1_data = "field1";
  error = ZP_Context_get_symbol(&context, &symbol1, symbol1_data, 6);
  handle_error(error);

  ZP_Tagged_t symbol2;
  const char* symbol2_data = "field2";
  error = ZP_Context_get_symbol(&context, &symbol2, symbol2_data, 6);
  handle_error(error);

  ZP_Tagged_t symbol3;
  const char* symbol3_data = "field3";
  error = ZP_Context_get_symbol(&context, &symbol3, symbol3_data, 6);
  handle_error(error);

  ZP_Tagged_t symbol4;
  const char* symbol4_data = "field4";
  error = ZP_Context_get_symbol(&context, &symbol4, symbol4_data, 6);
  handle_error(error);

  ZP_Tagged_t symbol5;
  const char* symbol5_data = "field5";
  error = ZP_Context_get_symbol(&context, &symbol5, symbol5_data, 6);
  handle_error(error);

  ZP_Tagged_t symbol6;
  const char* symbol6_data = "field6";
  error = ZP_Context_get_symbol(&context, &symbol6, symbol6_data, 6);
  handle_error(error);

  ZP_Tagged_t list0;
  ZP_List_new_empty(&list0);

  ZP_Tagged_t list1;
  error = ZP_List_push(&context, &list1, &list0, &symbol1);
  handle_error(error);

  ZP_Tagged_t list2;
  error = ZP_List_push(&context, &list2, &list1, &symbol2);
  handle_error(error);

  ZP_Tagged_t list3;
  error = ZP_List_push(&context, &list3, &list2, &symbol3);
  handle_error(error);

  ZP_Tagged_t list4;
  error = ZP_List_push(&context, &list4, &list3, &symbol4);
  handle_error(error);

  ZP_Tagged_t list5;
  error = ZP_List_push(&context, &list5, &list4, &symbol5);
  handle_error(error);

  ZP_Tagged_t list6;
  error = ZP_List_push(&context, &list6, &list5, &symbol6);
  handle_error(error);

  ZP_Tagged_t class_2fields;
  error = ZP_SimpleClass_new(&context, &class_2fields, &list2);
  handle_error(error);

  ZP_Tagged_t class_3fields;
  error = ZP_SimpleClass_new(&context, &class_3fields, &list3);
  handle_error(error);

  ZP_Tagged_t class_4fields;
  error = ZP_SimpleClass_new(&context, &class_4fields, &list4);
  handle_error(error);

  ZP_Tagged_t class_5fields;
  error = ZP_SimpleClass_new(&context, &class_5fields, &list5);
  handle_error(error);

  ZP_Tagged_t class_6fields;
  error = ZP_SimpleClass_new(&context, &class_6fields, &list6);
  handle_error(error);

  ZP_Int_t arg_value = 0;
  ZP_Index_t count_args;
  ZP_Tagged_t arg_item;
  ZP_Tagged_t arg_list;
  ZP_Tagged_t arg_list_prev;

  ZP_Tagged_t collection_prev;
  ZP_Index_t count_instances;
  ZP_Tagged_t current_instance;

  ZP_Tagged_t collection_2fields;
  ZP_Tagged_t collection_3fields;
  ZP_Tagged_t collection_4fields;
  ZP_Tagged_t collection_5fields;
  ZP_Tagged_t collection_6fields;

  ZP_Index_t count_iterations;
  ZP_List_new_empty(&collection_6fields);

  for (count_iterations = 0;  count_iterations < 1000;  count_iterations++) {
    ZP_List_new_empty(&collection_2fields);
    collection_prev.data = collection_2fields.data;

    for (count_instances = 0;  count_instances < 10000;  count_instances++) {
      ZP_List_new_empty(&arg_list_prev);
      for (count_args = 0;  count_args < 2;  count_args++) {
        arg_value++;
        ZP_Tagged_from_int(&arg_item, &arg_value);
        error = ZP_List_push(&context, &arg_list, &arg_list_prev, &arg_item);
        handle_error(error);
        ZP_Context_slab_decref(&context, &arg_list_prev);
        ZP_Context_slab_decref(&context, &arg_item);
        arg_list_prev.data = arg_list.data;
      }

      error = ZP_SimpleClass_instance_new(&context, &current_instance, &class_2fields, &arg_list);
      handle_error(error);

      if (count_iterations == 1) {
        break;
      }

      ZP_Context_slab_decref(&context, &arg_list);

      error = ZP_List_push(&context, &collection_2fields, &collection_prev, &current_instance);
      handle_error(error);
      ZP_Context_slab_decref(&context, &collection_prev);
      ZP_Context_slab_decref(&context, &current_instance);
      collection_prev.data = collection_2fields.data;
    }

    ZP_Context_slab_decref(&context, &collection_6fields);

    ZP_List_new_empty(&collection_3fields);
    collection_prev.data = collection_3fields.data;
    for (count_instances = 0;  count_instances < 10000;  count_instances++) {
      ZP_List_new_empty(&arg_list_prev);
      for (count_args = 0;  count_args < 3;  count_args++) {
        arg_value++;
        ZP_Tagged_from_int(&arg_item, &arg_value);
        error = ZP_List_push(&context, &arg_list, &arg_list_prev, &arg_item);
        handle_error(error);
        ZP_Context_slab_decref(&context, &arg_list_prev);
        ZP_Context_slab_decref(&context, &arg_item);
        arg_list_prev.data = arg_list.data;
      }

      error = ZP_SimpleClass_instance_new(&context, &current_instance, &class_3fields, &arg_list);
      handle_error(error);

      ZP_Context_slab_decref(&context, &arg_list);

      error = ZP_List_push(&context, &collection_3fields, &collection_prev, &current_instance);
      handle_error(error);
      ZP_Context_slab_decref(&context, &collection_prev);
      ZP_Context_slab_decref(&context, &current_instance);
      collection_prev.data = collection_3fields.data;
    }

    ZP_Context_slab_decref(&context, &collection_2fields);

    ZP_List_new_empty(&collection_4fields);
    collection_prev.data = collection_4fields.data;
    for (count_instances = 0;  count_instances < 10000;  count_instances++) {
      ZP_List_new_empty(&arg_list_prev);
      for (count_args = 0;  count_args < 4;  count_args++) {
        arg_value++;
        ZP_Tagged_from_int(&arg_item, &arg_value);
        error = ZP_List_push(&context, &arg_list, &arg_list_prev, &arg_item);
        handle_error(error);
        ZP_Context_slab_decref(&context, &arg_list_prev);
        ZP_Context_slab_decref(&context, &arg_item);
        arg_list_prev.data = arg_list.data;
      }

      error = ZP_SimpleClass_instance_new(&context, &current_instance, &class_4fields, &arg_list);
      handle_error(error);

      ZP_Context_slab_decref(&context, &arg_list);

      error = ZP_List_push(&context, &collection_4fields, &collection_prev, &current_instance);
      handle_error(error);
      ZP_Context_slab_decref(&context, &collection_prev);
      ZP_Context_slab_decref(&context, &current_instance);
      collection_prev.data = collection_4fields.data;
    }

    ZP_Context_slab_decref(&context, &collection_3fields);

    ZP_List_new_empty(&collection_5fields);
    collection_prev.data = collection_5fields.data;
    for (count_instances = 0;  count_instances < 10000;  count_instances++) {
      ZP_List_new_empty(&arg_list_prev);
      for (count_args = 0;  count_args < 5;  count_args++) {
        arg_value++;
        ZP_Tagged_from_int(&arg_item, &arg_value);
        error = ZP_List_push(&context, &arg_list, &arg_list_prev, &arg_item);
        handle_error(error);
        ZP_Context_slab_decref(&context, &arg_list_prev);
        ZP_Context_slab_decref(&context, &arg_item);
        arg_list_prev.data = arg_list.data;
      }

      error = ZP_SimpleClass_instance_new(&context, &current_instance, &class_5fields, &arg_list);
      handle_error(error);

      ZP_Context_slab_decref(&context, &arg_list);

      error = ZP_List_push(&context, &collection_5fields, &collection_prev, &current_instance);
      handle_error(error);
      ZP_Context_slab_decref(&context, &collection_prev);
      ZP_Context_slab_decref(&context, &current_instance);
      collection_prev.data = collection_5fields.data;
    }

    ZP_Context_slab_decref(&context, &collection_4fields);

    ZP_List_new_empty(&collection_6fields);
    collection_prev.data = collection_6fields.data;
    for (count_instances = 0;  count_instances < 10000;  count_instances++) {
      ZP_List_new_empty(&arg_list_prev);
      for (count_args = 0;  count_args < 6;  count_args++) {
        arg_value++;
        ZP_Tagged_from_int(&arg_item, &arg_value);
        error = ZP_List_push(&context, &arg_list, &arg_list_prev, &arg_item);
        handle_error(error);
        ZP_Context_slab_decref(&context, &arg_list_prev);
        ZP_Context_slab_decref(&context, &arg_item);
        arg_list_prev.data = arg_list.data;
      }

      error = ZP_SimpleClass_instance_new(&context, &current_instance, &class_6fields, &arg_list);
      handle_error(error);

      ZP_Context_slab_decref(&context, &arg_list);

      error = ZP_List_push(&context, &collection_6fields, &collection_prev, &current_instance);
      handle_error(error);
      ZP_Context_slab_decref(&context, &collection_prev);
      ZP_Context_slab_decref(&context, &current_instance);
      collection_prev.data = collection_6fields.data;
    }

    ZP_Context_slab_decref(&context, &collection_5fields);
  }

  ZP_Context_slab_decref(&context, &collection_6fields);

  return 0;
}
