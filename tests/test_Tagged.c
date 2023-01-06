#include <utilities_for_tests.h>

int test_int() {
  ZP_Int_t myint_m2 = -2;
  ZP_Int_t myint_m1 = -1;
  ZP_Int_t myint_ze = 0;
  ZP_Int_t myint_p1 = 1;
  ZP_Int_t myint_p2 = 2;

  ZP_Int_t myint_high_1 = ZP_MAXINT - 2;
  ZP_Int_t myint_high_2 = ZP_MAXINT - 1;
  ZP_Int_t myint_high_3 = ZP_MAXINT;
  ZP_Int_t myint_high_err = ZP_MAXINT + 1;

  ZP_Int_t myint_low_err = ZP_MININT - 1;
  ZP_Int_t myint_low_1 = ZP_MININT;
  ZP_Int_t myint_low_2 = ZP_MININT + 1;
  ZP_Int_t myint_low_3 = ZP_MININT + 2;

  ZP_Tagged_t tagged;
  ZP_Int_t integer_copy;
  ZP_Index_t tagged_data_copy;

  integer_copy = myint_m2;
  ZP_Tagged_from_int(&tagged, &myint_m2);
  assert(integer_copy, myint_m2);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_m2);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_m1;
  ZP_Tagged_from_int(&tagged, &myint_m1);
  assert(integer_copy, myint_m1);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_m1);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_ze;
  ZP_Tagged_from_int(&tagged, &myint_ze);
  assert(integer_copy, myint_ze);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_ze);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_p1;
  ZP_Tagged_from_int(&tagged, &myint_p1);
  assert(integer_copy, myint_p1);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_p1);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_p2;
  ZP_Tagged_from_int(&tagged, &myint_p2);
  assert(integer_copy, myint_p2);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_p2);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_high_1;
  ZP_Tagged_from_int(&tagged, &myint_high_1);
  assert(integer_copy, myint_high_1);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_high_1);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_high_2;
  ZP_Tagged_from_int(&tagged, &myint_high_2);
  assert(integer_copy, myint_high_2);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_high_2);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_high_3;
  ZP_Tagged_from_int(&tagged, &myint_high_3);
  assert(integer_copy, myint_high_3);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_high_3);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_high_err;
  ZP_Tagged_from_int(&tagged, &myint_high_err);
  assert(integer_copy, myint_high_err);  /* didn't change the original */

  integer_copy = myint_low_1;
  ZP_Tagged_from_int(&tagged, &myint_low_1);
  assert(integer_copy, myint_low_1);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_low_1);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_low_2;
  ZP_Tagged_from_int(&tagged, &myint_low_2);
  assert(integer_copy, myint_low_2);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_low_2);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_low_3;
  ZP_Tagged_from_int(&tagged, &myint_low_3);
  assert(integer_copy, myint_low_3);  /* didn't change the original */
  tagged_data_copy = tagged.data;
  assert(ZP_Tagged_to_int(&tagged), myint_low_3);
  assert(tagged.data, tagged_data_copy);  /* didn't change the original */

  integer_copy = myint_low_err;
  ZP_Tagged_from_int(&tagged, &myint_low_err);
  assert(integer_copy, myint_low_err);  /* didn't change the original */

  return 0;
}

int test_float() {
  ZP_Float_t myfloat1 = 3.14;
  ZP_Float_t myfloat2 = 3.14;
  ZP_Float_t myfloat3 = -3.14;
  ZP_Float_t myfloat4 = -3.14;

  ZP_Index_t* tmp;

  tmp = (ZP_Index_t*)(&myfloat2);
  *tmp &= ~(ZP_Index_t)1;
  tmp = (ZP_Index_t*)(&myfloat4);
  *tmp &= ~(ZP_Index_t)1;

  assert(myfloat1 != myfloat2, true);
  assert(myfloat1 != myfloat3, true);
  assert(myfloat1 != myfloat4, true);

  ZP_Tagged_t tagged;
  ZP_Float_t float_copy;
  ZP_Index_t tagged_data_copy;

  float_copy = myfloat1;
  assert(myfloat1 == float_copy, true);
  ZP_Tagged_from_float(&tagged, &myfloat1);
  assert(myfloat1 == float_copy, true);  /* didn't change the original */

  tagged_data_copy = tagged.data;
  assert(fabs(myfloat1 - ZP_Tagged_to_float(&tagged)) < 1e8, true);
  assert(tagged_data_copy, tagged.data);  /* didn't change the original */

  float_copy = myfloat2;
  assert(myfloat2 == float_copy, true);
  ZP_Tagged_from_float(&tagged, &myfloat2);
  assert(myfloat2 == float_copy, true);  /* didn't change the original */

  tagged_data_copy = tagged.data;
  assert(fabs(myfloat2 - ZP_Tagged_to_float(&tagged)) < 1e8, true);
  assert(tagged_data_copy, tagged.data);  /* didn't change the original */

  float_copy = myfloat3;
  assert(myfloat3 == float_copy, true);
  ZP_Tagged_from_float(&tagged, &myfloat3);
  assert(myfloat3 == float_copy, true);  /* didn't change the original */

  tagged_data_copy = tagged.data;
  assert(fabs(myfloat3 - ZP_Tagged_to_float(&tagged)) < 1e8, true);
  assert(tagged_data_copy, tagged.data);  /* didn't change the original */

  float_copy = myfloat4;
  assert(myfloat4 == float_copy, true);
  ZP_Tagged_from_float(&tagged, &myfloat4);
  assert(myfloat4 == float_copy, true);  /* didn't change the original */

  tagged_data_copy = tagged.data;
  assert(fabs(myfloat4 - ZP_Tagged_to_float(&tagged)) < 1e8, true);
  assert(tagged_data_copy, tagged.data);  /* didn't change the original */

  return 0;
}

int test_float_extremes() {
  ZP_Float_t myfloat1 = 1.0/0.0;  /* positive infinity */
  ZP_Float_t myfloat2 = -1.0/0.0;  /* negative infinity */
  ZP_Float_t myfloat3 = 0.0/0.0;  /* not-a-number */

  ZP_Tagged_t tagged;
  ZP_Float_t float_copy;
  ZP_Index_t tagged_data_copy;

  float_copy = myfloat1;
  assert(myfloat1 == float_copy, true);
  ZP_Tagged_from_float(&tagged, &myfloat1);
  assert(myfloat1 == float_copy, true);  /* didn't change the original */

  tagged_data_copy = tagged.data;
  float_copy = ZP_Tagged_to_float(&tagged);
  assert(myfloat1 == float_copy, true);
  assert(tagged_data_copy, tagged.data);  /* didn't change the original */

  float_copy = myfloat2;
  assert(myfloat2 == float_copy, true);
  ZP_Tagged_from_float(&tagged, &myfloat2);
  assert(myfloat2 == float_copy, true);  /* didn't change the original */

  tagged_data_copy = tagged.data;
  float_copy = ZP_Tagged_to_float(&tagged);
  assert(myfloat2 == float_copy, true);
  assert(tagged_data_copy, tagged.data);  /* didn't change the original */

  assert(myfloat3 != myfloat3, true);  /* NaN requires special handling */
  ZP_Tagged_from_float(&tagged, &myfloat3);

  tagged_data_copy = tagged.data;
  float_copy = ZP_Tagged_to_float(&tagged);
  assert(float_copy != float_copy, true);  /* at least, it's still NaN */
  assert(tagged_data_copy, tagged.data);  /* didn't change the original */

  return 0;
}

int main(int argc, char** argv) {
  int result = 0;

  result = test_int();
  if (result != 0) {
    return result;
  }

#if ZIPPY_POINTER_SIZE != 16

  result = test_float();
  if (result != 0) {
    return result;
  }

  result = test_float_extremes();
  if (result != 0) {
    return result;
  }

#endif /* ZIPPY_POINTER_SIZE == 16 */

  return result;
}
