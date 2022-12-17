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

void viewhex(void* arena, size_t arena_size) {
  ZP_Index_t i;
  for (i = 0;  i < arena_size;  i++) {
    printf("%02x ", ((unsigned char*)arena)[i]);
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

int test_PairAllocator() {
  printf("BEGIN test_PairAllocator\n\n");

  size_t arena_size = 64;
  void* arena = malloc(arena_size);
  init(arena, arena_size);

  size_t refcount_size = 8;
  void* refcount = malloc(refcount_size);
  init(refcount, refcount_size);

  ZP_Error_t error = ZP_NOERROR;

  ZP_PairAllocator_t pair_allocator;
  error = ZP_PairAllocator_new(
    &pair_allocator,
    arena,
    refcount,
    arena_size,
    refcount_size
  );
  handle_error(error);
  viewhex(arena, arena_size);
  printf("\n");

  ZP_Pair_t pair1;
  error = ZP_PairAllocator_alloc(&pair_allocator, &pair1);
  handle_error(error);
  ZP_PairAllocator_set(&pair_allocator, &pair1, 5, 6);
  viewhex(arena, arena_size);
  printf("\n");

  ZP_Index_t one;
  ZP_Index_t two;
  ZP_PairAllocator_get(&pair_allocator, &pair1, &one, &two);
  assert(one, 5);
  assert(two, 6);

  ZP_Pair_t pair2;
  error = ZP_PairAllocator_alloc(&pair_allocator, &pair2);
  handle_error(error);
  ZP_PairAllocator_set(&pair_allocator, &pair2, 0xeeeeee01, 0xeeeeee02);
  viewhex(arena, arena_size);
  printf("\n");

  ZP_Pair_t pair3;
  error = ZP_PairAllocator_alloc(&pair_allocator, &pair3);
  handle_error(error);
  ZP_PairAllocator_set(&pair_allocator, &pair3, 0xdddddd01, 0xdddddd02);
  viewhex(arena, arena_size);
  printf("\n");

  ZP_Pair_t pair4;
  error = ZP_PairAllocator_alloc(&pair_allocator, &pair4);
  handle_error(error);
  ZP_PairAllocator_set(&pair_allocator, &pair4, 0xaaaaaa01, 0xaaaaaa02);
  viewhex(arena, arena_size);
  printf("\n");

  ZP_PairAllocator_get(&pair_allocator, &pair3, &one, &two);
  assert(one, 0xdddddd01);
  assert(two, 0xdddddd02);

  ZP_PairAllocator_free(&pair_allocator, &pair3);
  viewhex(arena, arena_size);
  printf("first_pointer %d\n", pair_allocator.first_pointer);
  printf("\n");

  ZP_Pair_t pair5;
  error = ZP_PairAllocator_alloc(&pair_allocator, &pair5);
  handle_error(error);
  ZP_PairAllocator_set(&pair_allocator, &pair5, 0xbbbbbb01, 0xbbbbbb02);
  viewhex(arena, arena_size);
  printf("first_pointer %d\n", pair_allocator.first_pointer);
  printf("\n");

  ZP_PairAllocator_free(&pair_allocator, &pair5);
  viewhex(arena, arena_size);
  printf("first_pointer %d\n", pair_allocator.first_pointer);
  printf("\n");

  ZP_PairAllocator_free(&pair_allocator, &pair2);
  viewhex(arena, arena_size);
  printf("first_pointer %d\n", pair_allocator.first_pointer);
  printf("\n");

  ZP_PairAllocator_free(&pair_allocator, &pair4);
  viewhex(arena, arena_size);
  printf("first_pointer %d\n", pair_allocator.first_pointer);
  printf("\n");

  ZP_Pair_t pair6;
  error = ZP_PairAllocator_alloc(&pair_allocator, &pair6);
  handle_error(error);
  ZP_PairAllocator_set(&pair_allocator, &pair6, 0xcccccc08, 0xcccccc09);
  viewhex(arena, arena_size);
  printf("first_pointer %d\n", pair_allocator.first_pointer);
  printf("\n");

  ZP_Pair_t pair7;
  error = ZP_PairAllocator_alloc(&pair_allocator, &pair7);
  handle_error(error);
  ZP_PairAllocator_set(&pair_allocator, &pair7, 0xcccccc06, 0xcccccc07);
  viewhex(arena, arena_size);
  printf("first_pointer %d\n", pair_allocator.first_pointer);
  printf("\n");

  ZP_Pair_t pair8;
  error = ZP_PairAllocator_alloc(&pair_allocator, &pair8);
  handle_error(error);
  ZP_PairAllocator_set(&pair_allocator, &pair8, 0xcccccc04, 0xcccccc05);
  viewhex(arena, arena_size);
  printf("first_pointer %d\n", pair_allocator.first_pointer);
  printf("\n");

  ZP_Pair_t pair9;
  error = ZP_PairAllocator_alloc(&pair_allocator, &pair9);
  handle_error(error);
  ZP_PairAllocator_set(&pair_allocator, &pair9, 0xcccccc02, 0xcccccc03);
  viewhex(arena, arena_size);
  printf("first_pointer %d\n", pair_allocator.first_pointer);
  printf("\n");

  printf("END test_PairAllocator\n");

  return 0;
}

int test_integer_labeling() {
  printf("BEGIN test_integer_labeling\n");

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
  error = ZP_from_integer(&myint_m2, &labeled);
  handle_error(error);
  assert(integer_copy, myint_m2);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_to_integer(&labeled), myint_m2);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_m1;
  error = ZP_from_integer(&myint_m1, &labeled);
  handle_error(error);
  assert(integer_copy, myint_m1);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_to_integer(&labeled), myint_m1);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_ze;
  error = ZP_from_integer(&myint_ze, &labeled);
  handle_error(error);
  assert(integer_copy, myint_ze);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_to_integer(&labeled), myint_ze);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_p1;
  error = ZP_from_integer(&myint_p1, &labeled);
  handle_error(error);
  assert(integer_copy, myint_p1);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_to_integer(&labeled), myint_p1);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_p2;
  error = ZP_from_integer(&myint_p2, &labeled);
  handle_error(error);
  assert(integer_copy, myint_p2);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_to_integer(&labeled), myint_p2);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_high_1;
  error = ZP_from_integer(&myint_high_1, &labeled);
  handle_error(error);
  assert(integer_copy, myint_high_1);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_to_integer(&labeled), myint_high_1);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_high_2;
  error = ZP_from_integer(&myint_high_2, &labeled);
  handle_error(error);
  assert(integer_copy, myint_high_2);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_to_integer(&labeled), myint_high_2);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_high_3;
  error = ZP_from_integer(&myint_high_3, &labeled);
  handle_error(error);
  assert(integer_copy, myint_high_3);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_to_integer(&labeled), myint_high_3);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_high_err;
  error = ZP_from_integer(&myint_high_err, &labeled);
  handle_not_error(error);
  assert(integer_copy, myint_high_err);  /* didn't change the original */

  integer_copy = myint_low_1;
  error = ZP_from_integer(&myint_low_1, &labeled);
  handle_error(error);
  assert(integer_copy, myint_low_1);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_to_integer(&labeled), myint_low_1);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_low_2;
  error = ZP_from_integer(&myint_low_2, &labeled);
  handle_error(error);
  assert(integer_copy, myint_low_2);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_to_integer(&labeled), myint_low_2);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_low_3;
  error = ZP_from_integer(&myint_low_3, &labeled);
  handle_error(error);
  assert(integer_copy, myint_low_3);  /* didn't change the original */
  labeled_data_copy = labeled.data;
  assert(ZP_to_integer(&labeled), myint_low_3);
  assert(labeled.data, labeled_data_copy);  /* didn't change the original */

  integer_copy = myint_low_err;
  error = ZP_from_integer(&myint_low_err, &labeled);
  handle_not_error(error);
  assert(integer_copy, myint_low_err);  /* didn't change the original */

  printf("END test_integer_labeling\n");

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
  ZP_from_float(&myfloat1, &labeled);
  assert(myfloat1 == float_copy, true);  /* didn't change the original */

  labeled_data_copy = labeled.data;
  assert(fabs(myfloat1 - ZP_to_float(&labeled)) < 1e8, true);
  assert(labeled_data_copy, labeled.data);  /* didn't change the original */

  float_copy = myfloat2;
  assert(myfloat2 == float_copy, true);
  ZP_from_float(&myfloat2, &labeled);
  assert(myfloat2 == float_copy, true);  /* didn't change the original */

  labeled_data_copy = labeled.data;
  assert(fabs(myfloat2 - ZP_to_float(&labeled)) < 1e8, true);
  assert(labeled_data_copy, labeled.data);  /* didn't change the original */

  float_copy = myfloat3;
  assert(myfloat3 == float_copy, true);
  ZP_from_float(&myfloat3, &labeled);
  assert(myfloat3 == float_copy, true);  /* didn't change the original */

  labeled_data_copy = labeled.data;
  assert(fabs(myfloat3 - ZP_to_float(&labeled)) < 1e8, true);
  assert(labeled_data_copy, labeled.data);  /* didn't change the original */

  float_copy = myfloat4;
  assert(myfloat4 == float_copy, true);
  ZP_from_float(&myfloat4, &labeled);
  assert(myfloat4 == float_copy, true);  /* didn't change the original */

  labeled_data_copy = labeled.data;
  assert(fabs(myfloat4 - ZP_to_float(&labeled)) < 1e8, true);
  assert(labeled_data_copy, labeled.data);  /* didn't change the original */

  printf("END test_float_labeling\n");

  return 0;
}

int test_symbols() {
  printf("BEGIN test_symbols\n\n");

  size_t arena_size = 64;
  void* arena = malloc(arena_size);
  init(arena, arena_size);

  ZP_Error_t error = ZP_NOERROR;

  ZP_RegionAllocator_t region_allocator;
  error = ZP_RegionAllocator_new(&region_allocator, arena, arena_size);
  handle_error(error);
  view(arena, arena_size);
  printf("\n");

  ZP_SymbolSet_t symbol_set;
  error = ZP_SymbolSet_new(&symbol_set);
  handle_error(error);

  ZP_Symbol_t symbol_one;
  error = ZP_Symbol_new(
    &symbol_set,
    &region_allocator,
    "symbol22",
    8,
    &symbol_one
  );
  handle_error(error);
  view(arena, arena_size);
  printf("\n");

  ZP_Symbol_t symbol_two;
  error = ZP_Symbol_new(
    &symbol_set,
    &region_allocator,
    "symbol33",
    8,
    &symbol_two
  );
  handle_error(error);
  view(arena, arena_size);
  printf("\n");

  ZP_Symbol_t symbol_three;
  error = ZP_Symbol_new(
    &symbol_set,
    &region_allocator,
    "symbol11",
    8,
    &symbol_three
  );
  handle_error(error);
  view(arena, arena_size);
  printf("\n");

  printf("END test_symbols\n");

  return 0;
}

int main(int argc, char** argv) {
  int result;

  /* result = test_RegionAllocator(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  result = test_PairAllocator();
  if (result != 0) {
    return result;
  }

  /* result = test_integer_labeling(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  /* result = test_float_labeling(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  /* result = test_symbols(); */
  /* if (result != 0) { */
  /*   return result; */
  /* } */

  return 0;
}
