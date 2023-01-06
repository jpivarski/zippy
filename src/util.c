#include <zippy/util.h>

/***** helper (private) functions ********************************************/

/* the sorting algorithm is TimSort: https://en.wikipedia.org/wiki/Timsort */

const ZP_Index_t ZP_UTIL_ARGSORT_RUN = 32;

void ZP_util_argsort_insertion(
  ZP_Index_t* data,
  ZP_Index_t left,
  ZP_Index_t right
) {
  printf("insertion %d %d\n", left, right);

  ZP_Index_t i;
  ZP_Index_t j;
  ZP_Index_t tmp;

  for (i = left + 1;  i <= right;  i++) {
    tmp = data[i];
    j = i;
    while (j > left  &&  tmp < data[j - 1]) {
      data[j] = data[j - 1];
      j--;
    }
    data[j] = tmp;
  }
}

void ZP_util_argsort_merge(
  ZP_Index_t* scratch,
  ZP_Index_t* data,
  ZP_Index_t l,
  ZP_Index_t m,
  ZP_Index_t r
) {
  printf("merge %d %d %d\n", l, m, r);

  ZP_Index_t i;
  ZP_Index_t j;
  ZP_Index_t k;
  ZP_Index_t len1 = m - l + 1;
  ZP_Index_t len2 = r - m;

  ZP_Index_t* left = scratch;
  for (i = 0;  i < len1;  i++) {
    left[i] = data[l + i];
  }

  ZP_Index_t* right = &scratch[len1];
  for (i = 0;  i < len2;  i++) {
    right[i] = data[m + 1 + i];
  }

  i = 0;
  j = 0;
  k = l;

  while (i < len1  &&  j < len2) {
    if (left[i] <= right[j]) {
      data[k] = left[i];
      i++;
    }
    else {
      data[k] = right[j];
      j++;
    }
    k++;
  }

  while (i < len1) {
    data[k] = left[i];
    k++;
    i++;
  }

  while (j < len2) {
    data[k] = right[j];
    k++;
    j++;
  }
}

/***** public functions ******************************************************/

void ZP_util_argsort(
  ZP_Index_t* scratch,
  ZP_Index_t* index,
  ZP_Index_t* data,
  ZP_Index_t num
) {
  printf("BEGIN\n");

  ZP_Index_t i;
  ZP_Index_t min;
  for (i = 0;  i < num;  i += ZP_UTIL_ARGSORT_RUN) {
    min = i + ZP_UTIL_ARGSORT_RUN - 1;
    if (min > num - 1) {
      min = num - 1;
    }
    ZP_util_argsort_insertion(data, i, min);
  }

  ZP_Index_t size;
  ZP_Index_t left;
  ZP_Index_t mid;
  ZP_Index_t right;
  for (size = ZP_UTIL_ARGSORT_RUN;  size < num;  size = 2 * size) {
    for (left = 0;  left < num;  left += 2 * size) {
      mid = left + size - 1;
      right = left + 2*size - 1;
      if (right > num - 1) {
        right = num - 1;
      }
      ZP_util_argsort_merge(scratch, data, left, mid, right);
    }
  }

  printf("END\n");
}
