#include "find_min_max.h"
#include <stdio.h>
#include <limits.h>

struct MinMax GetMinMax(int *array, unsigned int begin, unsigned int end) {
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  // your code here
  if (end <= begin){
    printf("Error: end:%u <= begin:%u; The borders of the array are specified incorrectly", end, begin);
    min_max.min = 0;
    min_max.max = 0;
    return min_max;
  }

  for (unsigned int i = begin; i < end; i++) {
    if (array[i] < min_max.min) {
      min_max.min = array[i];
    }
    if (array[i] > min_max.max) {
      min_max.max = array[i];
    }
  }

  return min_max;
}
