#include <zippy/data.h>

/***** helper (private) functions ********************************************/

const ZP_Index_t ZP_TAG_SHIFT = 8 * (sizeof(ZP_Index_t) - 1);

const ZP_Index_t ZP_INT_TAG = ZP_TAG_INT;
const ZP_Index_t ZP_INT_MASK_SIGN = 0xc0 << ZP_TAG_SHIFT;
const ZP_Index_t ZP_INT_MASK_DATA = ~ZP_INT_MASK_SIGN;
const ZP_Index_t ZP_INT_SIGN_TEST = 0x20 << ZP_TAG_SHIFT;
const ZP_Index_t ZP_INT_OVERFLOW_TEST = 0xe0 << ZP_TAG_SHIFT;

const ZP_Index_t ZP_FLOAT_SIGN_0 = ZP_TAG_FLOAT;
const ZP_Index_t ZP_FLOAT_SIGN_1 = ZP_FLOAT_SIGN_0 | 1;
const ZP_Index_t ZP_FLOAT_MASK_DATA = ~ZP_FLOAT_SIGN_0;
const ZP_Index_t ZP_FLOAT_MASK_SIGNIFICANT = ~(ZP_Index_t)1;

/***** public functions ******************************************************/

void ZP_Tagged_copy(
  ZP_Tagged_t* output,
  ZP_Tagged_t* input
) {
  output->data = input->data;
}

bool ZP_Tagged_int_overflow(
  ZP_Int_t data
) {
  ZP_Int_t test = data & ZP_INT_OVERFLOW_TEST;
  return test != 0  &&  test != ZP_INT_OVERFLOW_TEST;
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

void ZP_Tagged_from_int(
  ZP_Tagged_t* tagged,
  ZP_Int_t* data
) {
  /* copy the integer data into tagged->data */
  tagged->data = *((ZP_Index_t*)data);

  /* wipe out any overflow bits (check with ZP_Tagged_int_overflow!) and add the tag */
  tagged->data &= ZP_INT_MASK_DATA;
  tagged->data |= ZP_INT_TAG;
}

ZP_Float_t ZP_Tagged_to_float(
  ZP_Tagged_t* tagged
) {
#if ZIPPY_POINTER_SIZE == 16

  /* no floating-point in 16-bit mode; included only for C's type-check */
  return (ZP_Float_t)ZP_Tagged_to_int(tagged);

#else

  /* remove the (one) tag bit in a copy of the tagged->data */
  ZP_Index_t hold = (tagged->data & ZP_FLOAT_MASK_DATA);

  /* the tag bit overlaps IEEE floating point sign data */
  /* so restore the missing sign from the least significant bit */
  hold |= (hold & 1) ? ZP_FLOAT_SIGN_0 : 0;

  /* zeros out the insignificant bit; this is necessary for minus infinity */
  hold &= ZP_FLOAT_MASK_SIGNIFICANT;

  ZP_Float_t* out = (ZP_Float_t*)(&hold);
  return *out;

#endif /* ZIPPY_POINTER_SIZE == 16 */
}

void ZP_Tagged_from_float(
  ZP_Tagged_t* tagged,
  ZP_Float_t* data
) {
#if ZIPPY_POINTER_SIZE == 16

  /* no floating-point in 16-bit mode; included only for C's type-check */
  ZP_Int_t hold = (ZP_Int_t)data;
  ZP_Tagged_from_int(tagged, &hold);

#else

  /* copy the floating point data into tagged->data */
  tagged->data = *((ZP_Index_t*)data);

  /* the tag bit will overlap IEEE floating point sign data */
  /* so store the sign in the least significant bit */
  tagged->data &= ZP_FLOAT_MASK_SIGNIFICANT;
  tagged->data |= (tagged->data & ZP_FLOAT_SIGN_0) ? ZP_FLOAT_SIGN_1 : ZP_FLOAT_SIGN_0;

#endif /* ZIPPY_POINTER_SIZE == 16 */
}
