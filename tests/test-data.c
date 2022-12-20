#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <zippy/data.h>

#define handle_error(error)   \
  if (error != ZP_NOERROR) {  \
    printf("%s\n", error);    \
    return -1;                \
  }                           \

#define handle_not_error(error)  \
  if (error == ZP_NOERROR) {     \
    printf("didn't encounter expected error\n"); \
    return -1;                   \
  }                              \

#define assert(obtained, expected) \
  if ((obtained) != (expected)) {  \
    printf("line %d: expected %x but obtained %x\n", __LINE__, expected, obtained); \
    return -1;                     \
  }                                \

void init(void* arena, size_t arena_size) {
  ZP_Index_t i;
  for (i = 0;  i < arena_size;  i++) {
    ((unsigned char*)arena)[i] = 0;
  }
}

void view(void* arena, size_t arena_size) {
  ZP_Index_t i;
  for (i = 0;  i < arena_size;  i++) {
    unsigned char x = ((unsigned char*)arena)[i];
    if (('A' <= x  &&  x <= 'Z')  ||  ('a' <= x  &&  x <= 'z')  ||  ('0' <= x  &&  x <= '9')) {
      printf("'%c ", x);
    }
    else {
      printf("%02x ", x);
    }
  }
  printf("\n");
}

void viewhex(void* arena, size_t arena_size, size_t join) {
  ZP_Index_t i;
  for (i = 0;  i < arena_size;  i++) {
    printf("%02x", ((unsigned char*)arena)[i]);
    if ((i + 1) % join == 0) {
      printf(" ");
    }
    else {
      printf("_");
    }
  }
  printf("\n");
}

int test_RegionAllocator() {
  printf("BEGIN test_RegionAllocator\n\n");

  size_t arena_size = 64;
  void* arena = malloc(arena_size);
  init(arena, arena_size);

  ZP_Error_t error = ZP_NOERROR;

  ZP_RegionAllocator_t region_allocator;
  error = ZP_RegionAllocator_new(&region_allocator, arena, arena_size);
  handle_error(error);
  view(arena, arena_size);
  printf("\n");

  ZP_Index_t pointer_one;
  ZP_Index_t request_size_one = 11;
  error = ZP_RegionAllocator_alloc(&region_allocator, request_size_one, &pointer_one);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&region_allocator, pointer_one, request_size_one, "hello there");
  view(arena, arena_size);
  printf("\n");

  ZP_Index_t pointer_two;
  ZP_Index_t request_size_two = 8;
  error = ZP_RegionAllocator_alloc(&region_allocator, request_size_two, &pointer_two);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&region_allocator, pointer_two, request_size_two, "you guys");
  view(arena, arena_size);
  printf("\n");

  ZP_Index_t pointer_three;
  ZP_Index_t request_size_three = 23;
  error = ZP_RegionAllocator_alloc(&region_allocator, request_size_three, &pointer_three);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&region_allocator, pointer_three, request_size_three, "WHAT SHOULD HAPPEN NOW?");
  view(arena, arena_size);
  printf("\n");

  ZP_RegionAllocator_free(&region_allocator, pointer_two, request_size_two);
  view(arena, arena_size);
  printf("\n");

  ZP_Index_t pointer_four;
  ZP_Index_t request_size_four = 15;
  error = ZP_RegionAllocator_alloc(&region_allocator, request_size_four, &pointer_four);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&region_allocator, pointer_four, request_size_four, "Something else.");
  view(arena, arena_size);
  printf("\n");

  ZP_RegionAllocator_free(&region_allocator, pointer_three, request_size_three);
  view(arena, arena_size);
  printf("\n");

  error = ZP_RegionAllocator_alloc(&region_allocator, request_size_three, &pointer_three);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&region_allocator, pointer_three, request_size_three, "WHAT SHOULD HAPPEN NOW?");
  view(arena, arena_size);
  printf("\n");

  error = ZP_RegionAllocator_alloc(&region_allocator, request_size_two, &pointer_two);
  handle_error(error);
  ZP_RegionAllocator_copy_data(&region_allocator, pointer_two, request_size_two, "you guys");
  view(arena, arena_size);
  printf("\n");

  printf("END test_RegionAllocator\n");

  return 0;
}

int test_FixedAllocator() {
  printf("BEGIN test_FixedAllocator\n\n");

  size_t arena_size = 64;
  void* arena = malloc(arena_size);
  init(arena, arena_size);

  size_t refcount_size = 8;
  void* refcount = malloc(refcount_size);

  ZP_Error_t error = ZP_NOERROR;

  ZP_FixedAllocator_t fixed_allocator;
  error = ZP_FixedAllocator_new(
    &fixed_allocator,
    arena,
    refcount,
    arena_size,
    refcount_size
  );
  handle_error(error);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  printf("\n");

  ZP_FixedLocation_t fixed1;
  error = ZP_FixedAllocator_alloc(&fixed_allocator, &fixed1);
  handle_error(error);
  ZP_FixedAllocator_set(&fixed_allocator, &fixed1, 5, 6);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  printf("\n");

  ZP_Index_t one;
  ZP_Index_t two;
  ZP_FixedAllocator_get(&fixed_allocator, &fixed1, &one, &two);
  assert(one, 5);
  assert(two, 6);

  ZP_FixedLocation_t fixed2;
  error = ZP_FixedAllocator_alloc(&fixed_allocator, &fixed2);
  handle_error(error);
  ZP_FixedAllocator_set(&fixed_allocator, &fixed2, 0xeeeeee01, 0xeeeeee02);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  printf("\n");

  ZP_FixedLocation_t fixed3;
  error = ZP_FixedAllocator_alloc(&fixed_allocator, &fixed3);
  handle_error(error);
  ZP_FixedAllocator_set(&fixed_allocator, &fixed3, 0xdddddd01, 0xdddddd02);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  printf("\n");

  ZP_FixedLocation_t fixed4;
  error = ZP_FixedAllocator_alloc(&fixed_allocator, &fixed4);
  handle_error(error);
  ZP_FixedAllocator_set(&fixed_allocator, &fixed4, 0xaaaaaa01, 0xaaaaaa02);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  printf("\n");

  ZP_FixedAllocator_get(&fixed_allocator, &fixed3, &one, &two);
  assert(one, 0xdddddd01);
  assert(two, 0xdddddd02);

  ZP_FixedAllocator_free(&fixed_allocator, &fixed3);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  printf("first_pointer %d\n", fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedLocation_t fixed5;
  error = ZP_FixedAllocator_alloc(&fixed_allocator, &fixed5);
  handle_error(error);
  ZP_FixedAllocator_set(&fixed_allocator, &fixed5, 0xbbbbbb01, 0xbbbbbb02);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  printf("first_pointer %d\n", fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedAllocator_free(&fixed_allocator, &fixed5);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  printf("first_pointer %d\n", fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedAllocator_free(&fixed_allocator, &fixed2);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  printf("first_pointer %d\n", fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedAllocator_free(&fixed_allocator, &fixed4);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  printf("first_pointer %d\n", fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedLocation_t fixed6;
  error = ZP_FixedAllocator_alloc(&fixed_allocator, &fixed6);
  handle_error(error);
  ZP_FixedAllocator_set(&fixed_allocator, &fixed6, 0xcccccc08, 0xcccccc09);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  printf("first_pointer %d\n", fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedLocation_t fixed7;
  error = ZP_FixedAllocator_alloc(&fixed_allocator, &fixed7);
  handle_error(error);
  ZP_FixedAllocator_set(&fixed_allocator, &fixed7, 0xcccccc06, 0xcccccc07);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  printf("first_pointer %d\n", fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedLocation_t fixed8;
  error = ZP_FixedAllocator_alloc(&fixed_allocator, &fixed8);
  handle_error(error);
  ZP_FixedAllocator_set(&fixed_allocator, &fixed8, 0xcccccc04, 0xcccccc05);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  printf("first_pointer %d\n", fixed_allocator.first_pointer);
  printf("\n");

  ZP_FixedLocation_t fixed9;
  error = ZP_FixedAllocator_alloc(&fixed_allocator, &fixed9);
  handle_error(error);
  ZP_FixedAllocator_set(&fixed_allocator, &fixed9, 0xcccccc02, 0xcccccc03);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  printf("first_pointer %d\n", fixed_allocator.first_pointer);
  printf("\n");

  printf("END test_FixedAllocator\n");

  return 0;
}

int test_int_labeling() {
  printf("BEGIN test_int_labeling\n");

  const ZP_Index_t SHIFT = (8 * sizeof(ZP_Index_t)) - 3;
  const ZP_Index_t MAXINT = (1 << SHIFT) - 1;
  const ZP_Index_t MININT = -MAXINT - 1;

  ZP_Int_t myint_m2 = -2;
  ZP_Int_t myint_m1 = -1;
  ZP_Int_t myint_ze = 0;
  ZP_Int_t myint_p1 = 1;
  ZP_Int_t myint_p2 = 2;

  ZP_Int_t myint_high_1 = MAXINT - 2;
  ZP_Int_t myint_high_2 = MAXINT - 1;
  ZP_Int_t myint_high_3 = MAXINT;
  ZP_Int_t myint_high_err = MAXINT + 1;

  ZP_Int_t myint_low_err = MININT - 1;
  ZP_Int_t myint_low_1 = MININT;
  ZP_Int_t myint_low_2 = MININT + 1;
  ZP_Int_t myint_low_3 = MININT + 2;

  ZP_Labeled_t labeled;
  ZP_Error_t error;
  ZP_Int_t integer_copy;
  ZP_Index_t labeled_data_copy;

  integer_copy = myint_m2;
  error = ZP_Labeled_from_int(&labeled, &myint_m2);
  handle_error(error);
  assert(integer_copy, myint_m2);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_Labeled_to_int(&labeled), myint_m2);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_m1;
  error = ZP_Labeled_from_int(&labeled, &myint_m1);
  handle_error(error);
  assert(integer_copy, myint_m1);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_Labeled_to_int(&labeled), myint_m1);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_ze;
  error = ZP_Labeled_from_int(&labeled, &myint_ze);
  handle_error(error);
  assert(integer_copy, myint_ze);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_Labeled_to_int(&labeled), myint_ze);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_p1;
  error = ZP_Labeled_from_int(&labeled, &myint_p1);
  handle_error(error);
  assert(integer_copy, myint_p1);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_Labeled_to_int(&labeled), myint_p1);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_p2;
  error = ZP_Labeled_from_int(&labeled, &myint_p2);
  handle_error(error);
  assert(integer_copy, myint_p2);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_Labeled_to_int(&labeled), myint_p2);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_high_1;
  error = ZP_Labeled_from_int(&labeled, &myint_high_1);
  handle_error(error);
  assert(integer_copy, myint_high_1);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_Labeled_to_int(&labeled), myint_high_1);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_high_2;
  error = ZP_Labeled_from_int(&labeled, &myint_high_2);
  handle_error(error);
  assert(integer_copy, myint_high_2);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_Labeled_to_int(&labeled), myint_high_2);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_high_3;
  error = ZP_Labeled_from_int(&labeled, &myint_high_3);
  handle_error(error);
  assert(integer_copy, myint_high_3);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_Labeled_to_int(&labeled), myint_high_3);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_high_err;
  error = ZP_Labeled_from_int(&labeled, &myint_high_err);
  handle_not_error(error);
  assert(integer_copy, myint_high_err);  /* didn't change the original */

  integer_copy = myint_low_1;
  error = ZP_Labeled_from_int(&labeled, &myint_low_1);
  handle_error(error);
  assert(integer_copy, myint_low_1);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_Labeled_to_int(&labeled), myint_low_1);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_low_2;
  error = ZP_Labeled_from_int(&labeled, &myint_low_2);
  handle_error(error);
  assert(integer_copy, myint_low_2);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_Labeled_to_int(&labeled), myint_low_2);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_low_3;
  error = ZP_Labeled_from_int(&labeled, &myint_low_3);
  handle_error(error);
  assert(integer_copy, myint_low_3);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_Labeled_to_int(&labeled), myint_low_3);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_low_err;
  error = ZP_Labeled_from_int(&labeled, &myint_low_err);
  handle_not_error(error);
  assert(integer_copy, myint_low_err);  /* didn't change the original */

  printf("END test_int_labeling\n");

  return 0;
}

int test_float_labeling() {
  printf("BEGIN test_float_labeling\n");

  ZP_Float_t myfloat1 = 3.14f;
  ZP_Float_t myfloat2 = 3.14f;
  ZP_Float_t myfloat3 = -3.14f;
  ZP_Float_t myfloat4 = -3.14f;

  ZP_Index_t* tmp;

  tmp = (ZP_Index_t*)(&myfloat2);
  *tmp &= ~(ZP_Index_t)1;
  tmp = (ZP_Index_t*)(&myfloat4);
  *tmp &= ~(ZP_Index_t)1;

  printf("%.30f\n", myfloat1);
  printf("%.30f\n", myfloat2);
  printf("%.30f\n", myfloat3);
  printf("%.30f\n", myfloat4);

  assert(myfloat1 != myfloat2, true);
  assert(myfloat1 != myfloat3, true);
  assert(myfloat1 != myfloat4, true);

  ZP_Labeled_t labeled;
  ZP_Float_t float_copy;
  ZP_Index_t labeled_data_copy;

  float_copy = myfloat1;
  assert(myfloat1 == float_copy, true);
  ZP_Labeled_from_float(&labeled, &myfloat1);
  assert(myfloat1 == float_copy, true);  /* didn't change the original */

  labeled_data_copy = labeled.data;
  assert(fabs(myfloat1 - ZP_Labeled_to_float(&labeled)) < 1e8, true);
  assert(labeled_data_copy, labeled.data);  /* didn't change the original */

  float_copy = myfloat2;
  assert(myfloat2 == float_copy, true);
  ZP_Labeled_from_float(&labeled, &myfloat2);
  assert(myfloat2 == float_copy, true);  /* didn't change the original */

  labeled_data_copy = labeled.data;
  assert(fabs(myfloat2 - ZP_Labeled_to_float(&labeled)) < 1e8, true);
  assert(labeled_data_copy, labeled.data);  /* didn't change the original */

  float_copy = myfloat3;
  assert(myfloat3 == float_copy, true);
  ZP_Labeled_from_float(&labeled, &myfloat3);
  assert(myfloat3 == float_copy, true);  /* didn't change the original */

  labeled_data_copy = labeled.data;
  assert(fabs(myfloat3 - ZP_Labeled_to_float(&labeled)) < 1e8, true);
  assert(labeled_data_copy, labeled.data);  /* didn't change the original */

  float_copy = myfloat4;
  assert(myfloat4 == float_copy, true);
  ZP_Labeled_from_float(&labeled, &myfloat4);
  assert(myfloat4 == float_copy, true);  /* didn't change the original */

  labeled_data_copy = labeled.data;
  assert(fabs(myfloat4 - ZP_Labeled_to_float(&labeled)) < 1e8, true);
  assert(labeled_data_copy, labeled.data);  /* didn't change the original */

  printf("END test_float_labeling\n");

  return 0;
}

int test_buffer() {
  printf("BEGIN test_buffer\n");

  size_t fixed_arena_size = 64;
  void* fixed_arena = malloc(fixed_arena_size);
  init(fixed_arena, fixed_arena_size);

  size_t refcount_size = 8;
  void* refcount = malloc(refcount_size);

  size_t region_arena_size = 64;
  void* region_arena = malloc(region_arena_size);
  init(region_arena, region_arena_size);

  ZP_Error_t error = ZP_NOERROR;

  ZP_FixedAllocator_t fixed_allocator;
  error = ZP_FixedAllocator_new(
    &fixed_allocator,
    fixed_arena,
    refcount,
    fixed_arena_size,
    refcount_size
  );
  handle_error(error);

  ZP_RegionAllocator_t region_allocator;
  error = ZP_RegionAllocator_new(&region_allocator, region_arena, region_arena_size);

  viewhex(fixed_arena, fixed_arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  handle_error(error);
  view(region_arena, region_arena_size);
  printf("\n");

  ZP_Labeled_t buffer1;
  const char* payload1 = "hello there";
  ZP_Index_t payload1_size = 11;
  error = ZP_BufferOwned_new(&fixed_allocator, &region_allocator, &buffer1, payload1_size);

  ZP_BufferOwned_copy_data(&fixed_allocator, &region_allocator, &buffer1, payload1);

  viewhex(fixed_arena, fixed_arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  handle_error(error);
  view(region_arena, region_arena_size);
  printf("\n");




  printf("END test_buffer\n");

  return 0;
}

int test_list() {
  printf("BEGIN test_list\n");

  size_t arena_size = 64;
  void* arena = malloc(arena_size);
  init(arena, arena_size);

  size_t refcount_size = 8;
  void* refcount = malloc(refcount_size);

  ZP_Error_t error = ZP_NOERROR;

  ZP_FixedAllocator_t fixed_allocator;
  error = ZP_FixedAllocator_new(
    &fixed_allocator,
    arena,
    refcount,
    arena_size,
    refcount_size
  );
  handle_error(error);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");

  ZP_Labeled_t list1;
  ZP_List_new_empty(&list1);
  assert(ZP_Labeled_label_of(&list1), ZP_LABEL_LIST);

  ZP_Labeled_t labeled_int1;
  ZP_Int_t int1 = 5;
  error = ZP_Labeled_from_int(&labeled_int1, &int1);
  handle_error(error);

  ZP_Labeled_t list2;
  error = ZP_List_push(&fixed_allocator, &list2, &list1, &labeled_int1);
  handle_error(error);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list2), 1);

  ZP_Labeled_t labeled_int2;
  ZP_Int_t int2 = 6;
  error = ZP_Labeled_from_int(&labeled_int2, &int2);
  handle_error(error);

  ZP_Labeled_t list3;
  error = ZP_List_push(&fixed_allocator, &list3, &list2, &labeled_int2);
  handle_error(error);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list2), 2);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list3), 1);

  ZP_Labeled_t labeled_int3;
  ZP_Int_t int3 = 7;
  error = ZP_Labeled_from_int(&labeled_int3, &int3);
  handle_error(error);

  ZP_Labeled_t list4;
  error = ZP_List_push(&fixed_allocator, &list4, &list3, &labeled_int3);
  handle_error(error);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list2), 2);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list3), 2);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list4), 1);

  ZP_Labeled_t obj1;
  ZP_Labeled_t obj2;
  error = ZP_List_pop(&fixed_allocator, &obj1, &obj2, &list4);
  handle_error(error);
  assert(ZP_Labeled_label_of(&obj1), ZP_LABEL_LIST);
  assert(ZP_Labeled_label_of(&obj2), ZP_LABEL_INT);
  assert(ZP_Labeled_to_int(&obj2), 7);
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list2), 2);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list3), 3);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list4), 1);

  ZP_Labeled_t obj3;
  ZP_Labeled_t obj4;
  error = ZP_List_pop(&fixed_allocator, &obj3, &obj4, &obj1);
  handle_error(error);
  assert(ZP_Labeled_label_of(&obj3), ZP_LABEL_LIST);
  assert(ZP_Labeled_label_of(&obj4), ZP_LABEL_INT);
  assert(ZP_Labeled_to_int(&obj4), 6);
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list2), 3);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list3), 3);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list4), 1);

  ZP_Labeled_t obj5;
  ZP_Labeled_t obj6;
  error = ZP_List_pop(&fixed_allocator, &obj5, &obj6, &obj3);
  handle_error(error);
  assert(ZP_Labeled_label_of(&obj5), ZP_LABEL_LIST);
  assert(ZP_Labeled_label_of(&obj6), ZP_LABEL_INT);
  assert(ZP_Labeled_to_raw(&obj5), 0x00ffffff);  /* list terminator */
  assert(ZP_Labeled_to_int(&obj6), 5);
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list2), 3);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list3), 3);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list4), 1);

  printf("incref list4 many times\n");
  ZP_Index_t i;
  for (i = 0;  i < 254;  i++) {
    error = ZP_FixedAllocator_incref(&fixed_allocator, &list4);
    handle_error(error);
  }
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list4), 255);

  ZP_Labeled_t new_list4;
  ZP_Labeled_copy(&new_list4, &list4);
  error = ZP_FixedAllocator_incref(&fixed_allocator, &new_list4);
  handle_error(error);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list2), 3);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list3), 4);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list4), 255);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &new_list4), 1);

  printf("decref list4 many times\n");
  for (i = 0;  i < 254;  i++) {
    ZP_FixedAllocator_decref(&fixed_allocator, 0, &list4);
  }
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list4), 1);

  ZP_FixedAllocator_decref(&fixed_allocator, 0, &list4);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list2), 3);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list3), 3);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &new_list4), 1);

  ZP_FixedAllocator_decref(&fixed_allocator, 0, &new_list4);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list2), 3);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list3), 2);

  ZP_FixedAllocator_decref(&fixed_allocator, 0, &list3);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list2), 3);
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list3), 1);

  ZP_FixedAllocator_decref(&fixed_allocator, 0, &list3);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list2), 2);

  ZP_FixedAllocator_decref(&fixed_allocator, 0, &list2);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");
  assert(ZP_FixedAllocator_refcnt(&fixed_allocator, &list2), 1);

  ZP_FixedAllocator_decref(&fixed_allocator, 0, &list2);
  viewhex(arena, arena_size, sizeof(ZP_Index_t));
  viewhex(refcount, refcount_size, sizeof(ZP_RefCnt_t));
  printf("\n");

  printf("END test_list\n");

  return 0;
}

/* int test_symbols() { */
/*   printf("BEGIN test_symbols\n\n"); */

/*   size_t arena_size = 64; */
/*   void* arena = malloc(arena_size); */
/*   init(arena, arena_size); */

/*   ZP_Error_t error = ZP_NOERROR; */

/*   ZP_RegionAllocator_t region_allocator; */
/*   error = ZP_RegionAllocator_new(&region_allocator, arena, arena_size); */
/*   handle_error(error); */
/*   view(arena, arena_size); */
/*   printf("\n"); */

/*   ZP_SymbolSet_t symbol_set; */
/*   error = ZP_SymbolSet_new(&symbol_set); */
/*   handle_error(error); */

/*   ZP_Symbol_t symbol_one; */
/*   error = ZP_Symbol_new( */
/*     &symbol_set, */
/*     &region_allocator, */
/*     "symbol22", */
/*     8, */
/*     &symbol_one */
/*   ); */
/*   handle_error(error); */
/*   view(arena, arena_size); */
/*   printf("\n"); */

/*   ZP_Symbol_t symbol_two; */
/*   error = ZP_Symbol_new( */
/*     &symbol_set, */
/*     &region_allocator, */
/*     "symbol33", */
/*     8, */
/*     &symbol_two */
/*   ); */
/*   handle_error(error); */
/*   view(arena, arena_size); */
/*   printf("\n"); */

/*   ZP_Symbol_t symbol_three; */
/*   error = ZP_Symbol_new( */
/*     &symbol_set, */
/*     &region_allocator, */
/*     "symbol11", */
/*     8, */
/*     &symbol_three */
/*   ); */
/*   handle_error(error); */
/*   view(arena, arena_size); */
/*   printf("\n"); */

/*   printf("END test_symbols\n"); */

/*   return 0; */
/* } */

int main(int argc, char** argv) {
  int result;

  /* result = test_RegionAllocator(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  /* result = test_FixedAllocator(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  /* result = test_int_labeling(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  /* result = test_float_labeling(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  /* result = test_list(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  result = test_buffer();
  if (result != 0) {
    return result;
  }

  return 0;
}
