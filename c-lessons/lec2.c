//сортировка слиянием
#include "stdio.h"
typedef bool (*icomp_f)(int a, int b);

void merge(size_t len, int a[len], int b[len], size_t mid, icomp_f comp)
{
  size_t i = 0, j = mid;
  for (size_t k = 0; k < len; ++k)
  {
    if (i < mid && (j >= len || comp(a[i], a[j])))
    {
      b[k] = a[i++];
    }
    else
    {
      b[k] = a[j++];
    }
  }
}

void split_merge_sort(size_t len, int a[len], int b[len], icomp_f compare)
{
  if (len == 1)
  {
    return;
  }

  int middle = len / 2;
  int half_len = len - middle;
  split_merge_sort(middle, b, a, compare);
  split_merge_sort(half_len, b + middle, a + middle, compare);

  merge(len, a, b, middle, compare);
}

void msort(size_t len, int arr[len], int tmp[len], icomp_f compare)
{
  memcpy(tmp, arr, sizeof(*arr) * len);
  split_merge_sort(len, tmp, arr, compare);
}

//бинарный поиск, в уже отсортированном массиве
ssize_t binsearch(size_t len, char arr[len], char c)
{
  size_t l = 0, r = len - 1;
  do
  {
    size_t i = l + (r - l) / 2;
    if (c == arr[i])
    {
      return i;
    }
    if (c < arr[i])
    {
      r = i - 1;
    }
    else
    {
      l = i + 1;
    }
  }
  while (l <= r);
  return -1;
}