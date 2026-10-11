#include <stdio.h>

int main()
{
  int len;
  scanf("%d", &len);
  int a[len];
  for (int i = 0;  i < len; ++i)
  {
    scanf("%d", &a[i]);
  }

  for (int i = 0; i < len; ++i)
  {
    printf("%d ", a[i]);
  }

  printf("\n");

}