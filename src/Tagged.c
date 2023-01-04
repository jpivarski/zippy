#include <zippy/data.h>

/***** helper (private) functions ********************************************/

#if ZIPPY_POINTER_SIZE != 16
  const ZP_Index_t ZP_FLOAT_SIGN_0 = ZP_TAG_FLOAT;
  const ZP_Index_t ZP_FLOAT_SIGN_1 = ZP_FLOAT_SIGN_0 | 1;
  const ZP_Index_t ZP_FLOAT_MASK_DATA = (ZP_Index_t)(~ZP_FLOAT_SIGN_0);
  const ZP_Index_t ZP_FLOAT_MASK_SIGNIFICANT = ~(ZP_Index_t)1;
#endif  /* ZIPPY_POINTER_SIZE != 16 */

#if ZIPPY_POINTER_SIZE == 16
  #define ZP_INT_MASK_SIGN       0x8000              /* 10000000,0x00 */
  #define ZP_INT_MASK_DATA       0x7fff              /* 01111111,0xff */
  #define ZP_INT_SIGN_TEST       0x4000              /* 01000000,0x00 */
  #define ZP_INT_OVERFLOW_TEST   0xc000              /* 11000000,0x00 */
#elif ZIPPY_POINTER_SIZE == 32
  #define ZP_INT_MASK_SIGN       0xc0000000          /* 11000000,0x000000 */
  #define ZP_INT_MASK_DATA       0x3fffffff          /* 00111111,0xffffff */
  #define ZP_INT_SIGN_TEST       0x20000000          /* 00100000,0x000000 */
  #define ZP_INT_OVERFLOW_TEST   0xe0000000          /* 11100000,0x000000 */
#else  /* ZIPPY_POINTER_SIZE == 64 */
  #define ZP_INT_MASK_SIGN       0xc000000000000000  /* 11000000,0x00000000000000 */
  #define ZP_INT_MASK_DATA       0x3fffffffffffffff  /* 00111111,0xffffffffffffff */
  #define ZP_INT_SIGN_TEST       0x2000000000000000  /* 00100000,0x00000000000000 */
  #define ZP_INT_OVERFLOW_TEST   0xe000000000000000  /* 11100000,0x00000000000000 */
#endif

/***** public functions ******************************************************/

void ZP_Tagged_copy(
  ZP_Tagged_t* output,
  ZP_Tagged_t* input
) {
  output->data = input->data;
}

bool ZP_Tagged_is_float(
  ZP_Tagged_t* tagged
) {
#if ZIPPY_POINTER_SIZE == 16
  return false;
#else
  return tagged->data & ZP_TAG_FLOAT;
#endif
}

bool ZP_Tagged_is_int(
  ZP_Tagged_t* tagged
) {
#if ZIPPY_POINTER_SIZE == 16
  return tagged->data & ZP_TAG_INT;
#else
  return !(tagged->data & ZP_TAG_FLOAT)  &&  (tagged->data & ZP_TAG_INT);
#endif
}

bool ZP_Tagged_is_refcntable(
  ZP_Tagged_t* tagged
) {
  return !(tagged->data & ZP_TAG_OBJECT_ANTIMASK) && (tagged->data & ZP_TAG_OBJECT_MASK);
}

ZP_Index_t ZP_Tagged_enum(
  ZP_Tagged_t* tagged
) {
  /* ZP_TERMINATOR means "not an enum" */
  return (tagged->data & ZP_TAG_OBJECT_MASK) ? ZP_TERMINATOR : tagged->data & (~ZP_TAG_OBJECT_MASK);
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
  ZP_Int_t hold = (ZP_Int_t)(*data);
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

bool ZP_Tagged_int_is_overflow(
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
  tagged->data |= ZP_TAG_INT;
}
