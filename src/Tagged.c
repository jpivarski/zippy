#include <zippy/data.h>

/***** helper (private) functions ********************************************/

/***** public functions ******************************************************/

void ZP_Tagged_copy(
  ZP_Tagged_t* output,
  ZP_Tagged_t* input
) {
  
}

ZP_Int_t ZP_Tagged_to_int(
  ZP_Tagged_t* tagged
) {
  return 0;
}

ZP_Error_t ZP_Tagged_from_int(
  ZP_Tagged_t* tagged,
  ZP_Int_t* data
) {
  return "not implemented Tagged 1";
}

ZP_Float_t ZP_Tagged_to_float(
  ZP_Tagged_t* tagged
) {
  return 3.14;
}

ZP_Error_t ZP_Tagged_from_float(
  ZP_Tagged_t* tagged,
  ZP_Float_t* data
) {
  return "not implemented Tagged 2";
}
