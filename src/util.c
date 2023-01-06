#include <zippy/util.h>

/***** helper (private) functions ********************************************/

const ZP_Index_t timsort_run = 32;

/***** public functions ******************************************************/

void ZP_util_argsort(
  ZP_Index_t* index,
  ZP_Index_t* data
) {

}

/* ********************************************************************** */

/* const int RUN = 32; */

/* // This function sorts array from left index to */
/* // to right index which is of size atmost RUN */
/* void insertionSort(int arr[], int left, int right) */
/* { */
/*     for (int i = left + 1; i <= right; i++) */
/*     { */
/*         int temp = arr[i]; */
/*         int j = i - 1; */
/*         while (j >= left && arr[j] > temp) */
/*         { */
/*             arr[j+1] = arr[j]; */
/*             j--; */
/*         } */
/*         arr[j+1] = temp; */
/*     } */
/* } */

/* // Merge function merges the sorted runs */
/* void merge(int arr[], int l, int m, int r) */
/* { */

/*     // Original array is broken in two parts */
/*     // left and right array */
/*     int len1 = m - l + 1, len2 = r - m; */
/*     int left[len1], right[len2]; */
/*     for (int i = 0; i < len1; i++) */
/*         left[i] = arr[l + i]; */
/*     for (int i = 0; i < len2; i++) */
/*         right[i] = arr[m + 1 + i]; */

/*     int i = 0; */
/*     int j = 0; */
/*     int k = l; */

/*     // After comparing, we */
/*     // merge those two array */
/*     // in larger sub array */
/*     while (i < len1 && j < len2) */
/*     { */
/*         if (left[i] <= right[j]) */
/*         { */
/*             arr[k] = left[i]; */
/*             i++; */
/*         } */
/*         else */
/*         { */
/*             arr[k] = right[j]; */
/*             j++; */
/*         } */
/*         k++; */
/*     } */

/*     // Copy remaining elements of left, if any */
/*     while (i < len1) */
/*     { */
/*         arr[k] = left[i]; */
/*         k++; */
/*         i++; */
/*     } */

/*     // Copy remaining element of right, if any */
/*     while (j < len2) */
/*     { */
/*         arr[k] = right[j]; */
/*         k++; */
/*         j++; */
/*     } */
/* } */

/* // Iterative Timsort function to sort the */
/* // array[0...n-1] (similar to merge sort) */
/* void timSort(int arr[], int n) */
/* { */

/*     // Sort individual subarrays of size RUN */
/*     for (int i = 0; i < n; i+=RUN) */
/*         insertionSort(arr, i, min((i+RUN-1), */
/*                                     (n-1))); */

/*     // Start merging from size RUN (or 32). */
/*     // It will merge */
/*     // to form size 64, then 128, 256 */
/*     // and so on .... */
/*     for (int size = RUN; size < n; */
/*                              size = 2*size) */
/*     { */

/*         // pick starting point of */
/*         // left sub array. We */
/*         // are going to merge */
/*         // arr[left..left+size-1] */
/*         // and arr[left+size, left+2*size-1] */
/*         // After every merge, we */
/*         // increase left by 2*size */
/*         for (int left = 0; left < n; */
/*                              left += 2*size) */
/*         { */

/*             // find ending point of */
/*             // left sub array */
/*             // mid+1 is starting point */
/*             // of right sub array */
/*             int mid = left + size - 1; */
/*             int right = min((left + 2*size - 1), */
/*                                             (n-1)); */

/*             // merge sub array arr[left.....mid] & */
/*             // arr[mid+1....right] */
/*               if(mid < right) */
/*                 merge(arr, left, mid, right); */
/*         } */
/*     } */
/* } */

/* ********************************************************************** */

/* const int RUN = 32; // Initialising the RUN to get chunks */
/* void insertionSort(int arr[], int left, int right) // Implementing insertion */
/* sort for RUN size chunks{ */
/*    for (int i = left + 1; i <= right; i++){ */
/*       int t = arr[i]; */
/*       int j = i - 1; */
/*       while (j >= left && t < arr[j]){ */
/*          arr[j+1] = arr[j--]; */
/*       } */
/*       arr[j+1] = t; */
/*    } */
/* } */
/* void merge(int arr[], int l, int m, int r) // using the merge function the */
/* sorted chunks of size 32 are merged into one{ */
/*    int len1 = m - l + 1, len2 = r - m; */
/*    int left[len1], right[len2]; */
/*    for (int i = 0; i < len1; i++) */
/*       left[i] = arr[l + i]; // Filling left array */
/*    for (int i = 0; i < len2; i++) */
/*       right[i] = arr[m + 1 + i]; // Filling right array */
/*    int i = 0; */
/*    int j = 0; */
/*    int k = l; */
/*    while (i < len1 && j < len2) // Iterate into both arrays left and right{ */
/*       if (left[i] <= right[j]) // IF element in left is less then increment i by pushing into larger array{ */
/*          arr[k] = left[i]; */
/*          i++; */
/*       } else { */
/*          arr[k] = right[j]; // Element in right array is greater */
/*          increment j */
/*          j++; */
/*       } */
/*       k++; */
/*    } */
/*    while (i < len1) // This loop copies remaining element in left array{ */
/*       arr[k] = left[i]; */
/*       k++; */
/*       i++; */
/*    } */
/*    while (j < len2) // This loop copies remaining element in right array{ */
/*       arr[k] = right[j]; */
/*       k++; */
/*       j++; */
/*    } */
/* } */
/* void timSortAlgo(int arr[], int n){ */
/*    for (int i = 0; i < n; i+=RUN) insertionSort(arr, i, min((i+31), (n-1))); //Call insertionSort() */
/*    for (int s = RUN; s < n; s = 2*s) // Start merging from size RUN (or 32). It will continue upto 2*RUN{ */
/*       // pick starting point of left sub array. We are going to merge */
/*       arr[left..left+size-1] */
/*       // and arr[left+size, left+2*size-1] */
/*       // After every merge, we */
/*       // increase left by 2*size */
/*       for (int left = 0; left < n;left += 2*s){ */
/*          int mid = left + s - 1; // find ending point of left sub */
/*          array mid+1 is starting point of right sub array */
/*          int right = min((left + 2*s - 1), (n-1)); */
/*          merge(arr, left, mid, right); // merge sub array */
/*          arr[left.....mid] & arr[mid+1....right] */
/*       } */
/*    } */
/* } */

/* ********************************************************************** */

/* const int RUN = 32; */
/* int min(int a, int b) */
/* { */
/*     if(a<b) */
/*     return a; */
/*     else */
/*     return b; */
/* } */
/* /\* This function sorts array from starting index to ending index which is of size atmost RUN *\/ */
/* void insertionSort(int a[], int beg, int end) /\* function to sort an array with insertion sort *\/ */
/* { */
/*     int i, j, temp; */
/*     for (i = beg + 1; i <= end; i++) { */
/*         temp = a[i]; */
/*         j = i - 1; */

/*         while(j>=0 && temp <= a[j]) */
/*         { */
/*             a[j+1] = a[j]; */
/*             jj = j-1; */
/*         } */
/*         a[j+1] = temp; */
/*     } */
/* } */
/* /\* Function to merge the sorted runs *\/ */
/* void merge(int a[], int beg, int mid, int end) */
/* { */
/*     int i, j, k; */
/*     int n1 = mid - beg + 1; */
/*     int n2 = end - mid; */
/*     int LeftArray[n1], RightArray[n2]; //temporary arrays */
/*     /\* copy data to temp arrays *\/ */
/*     for (int i = 0; i < n1; i++) */
/*     LeftArray[i] = a[beg + i]; */
/*     for (int j = 0; j < n2; j++) */
/*     RightArray[j] = a[mid + 1 + j]; */
/*     i = 0; */
/*     j = 0; */
/*     k = beg; */
/*     while (i < n1 && j < n2) */
/*     { */
/*         if(LeftArray[i] <= RightArray[j]) */
/*         { */
/*             a[k] = LeftArray[i]; */
/*             i++; */
/*         } */
/*         else */
/*         { */
/*             a[k] = RightArray[j]; */
/*             j++; */
/*         } */
/*         k++; */
/*     } */
/*     while (i<n1) */
/*     { */
/*         a[k] = LeftArray[i]; */
/*         i++; */
/*         k++; */
/*     } */
/*     while (j<n2) */
/*     { */
/*         a[k] = RightArray[j]; */
/*         j++; */
/*         k++; */
/*     } */
/* } */
/* /\* function to implement tim sort *\/ */
/* void timSort(int a[], int n) */
/* { */

/*     /\* Sort individual subarrays of size RUN *\/ */
/*     for (int i = 0; i < n; i+=RUN) */
/*         insertionSort(a, i, min((i+RUN-1), (n-1))); */
/*     // Start merging from size RUN (or 32). */
/*     for (int size = RUN; size < n; size = 2*size) */
/*     { */
/*         for (int beg = 0; beg < n; beg += 2*size) */
/*         { */
/*             /\* find ending point of left sub array. The starting point of right sub array is mid + 1 *\/ */
/*             int mid = beg + size - 1; */
/*             int end = min((beg + 2*size - 1),(n-1)); */

/*             /\* Merge subarray a[beg...mid] and a[mid+1...end] *\/ */
/*             if(mid < end) */
/*                 merge(a, beg, mid, end); */
/*         } */
/*     } */
/* } */

/* ********************************************************************** */

/* const int RUN = 32; */

/* // this function sorts array from left index to to right index which is of size atmost RUN */
/* void insertionSort(int arr[], int left, int right) */
/* { */
/*     for (int i = left + 1; i <= right; i++) */
/*     { */
/*         int temp = arr[i]; */
/*         int j = i - 1; */
/*         while (arr[j] > temp && j >= left) */
/*         { */
/*             arr[j+1] = arr[j]; */
/*             j--; */
/*         } */
/*         arr[j+1] = temp; */
/*     } */
/* } */

/* // merge function merges the sorted runs */
/* void merge(int arr[], int l, int m, int r) */
/* { */
/*     // original array is broken in two parts, left and right array */
/*     int len1 = m - l + 1, len2 = r - m; */
/*     int left[len1], right[len2]; */
/*     for (int i = 0; i < len1; i++) */
/*         left[i] = arr[l + i]; */
/*     for (int i = 0; i < len2; i++) */
/*         right[i] = arr[m + 1 + i]; */

/*     int i = 0; */
/*     int j = 0; */
/*     int k = l; */

/*     // after comparing, we merge those two array in larger sub array */
/*     while (i < len1 && j < len2) */
/*     { */
/*         if (left[i] <= right[j]) */
/*         { */
/*             arr[k] = left[i]; */
/*             i++; */
/*         } */
/*         else */
/*         { */
/*             arr[k] = right[j]; */
/*             j++; */
/*         } */
/*         k++; */
/*     } */

/*     // copy remaining elements of left, if any */
/*     while (i < len1) */
/*     { */
/*         arr[k] = left[i]; */
/*         k++; */
/*         i++; */
/*     } */

/*     // copy remaining element of right, if any */
/*     while (j < len2) */
/*     { */
/*         arr[k] = right[j]; */
/*         k++; */
/*         j++; */
/*     } */
/* } */

/* // iterative Timsort function to sort the array[0...n-1] (similar to merge sort) */
/* void timSort(int arr[], int n) */
/* { */
/*     // Sort individual subarrays of size RUN */
/*     for (int i = 0; i < n; i+=RUN) */
/*         insertionSort(arr, i, min((i+31), (n-1))); */

/*     // start merging from size RUN (or 32). It will merge to form size 64, then 128, 256 and so on .... */
/*     for (int size = RUN; size < n; size = 2*size) */
/*     { */
/*         // pick starting point of left sub array. We are going to merge arr[left..left+size-1] and arr[left+size, left+2*size-1] */
/*         // After every merge, we increase left by 2*size */
/*         for (int left = 0; left < n; left += 2*size) */
/*         { */
/*             // find ending point of left sub array */
/*             // mid+1 is starting point of right sub array */
/*             int mid = left + size - 1; */
/*             int right = min((left + 2*size - 1), (n-1)); */

/*             // merge sub array arr[left.....mid] & arr[mid+1....right] */
/*             merge(arr, left, mid, right); */
/*         } */
/*     } */
/* } */
