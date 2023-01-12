#ifndef ZIPPY_UTIL_H
#define ZIPPY_UTIL_H

#include <zippy/defs.h>

ZP_Index_t ZP_util_next_power_2(
  ZP_Index_t x
);

void ZP_util_argsort(
  ZP_Index_t* scratch,
  ZP_Index_t* index,
  ZP_Index_t* data,
  ZP_Index_t num
);


#endif /* ZIPPY_UTIL_H */
