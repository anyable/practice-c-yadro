#include <stdio.h>
#define time2min(h, m) ((((h) * 60) + m))
#define min2time(mm, h, m) (h = ((mm) / 60), m = ((mm) % 60))

int main()
{
/*   int x;
  scanf("%d", &x);
  printf("%d\n", x);
  printf("%s", __FILE__); */

/*   int h, m, mm;
  scanf("%d:%d", &h, &m);

  mm = time2min(h, m);
  printf("%d\n", mm);

  mm = time2min(h, m)*2;
  printf("%d\n", mm);

  mm = time2min(h+1, m+5);
  printf("%d\n", mm); */

  int h, m, mm;
  scanf("%d", &mm);

  min2time(mm, h, m);
  printf("%02d:%02d\n", h, m);

  min2time(mm+65, h, m);
  printf("%02d:%02d\n", h, m);
}