#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

void t() {
  int i;
  float f;

  i = rand();
  /* Ensure i > 0. */
  if (i == 0) {
     i = 1;
  }
  /* Pretend to be calculating a score with weight 10. */
  f = - log(((float) i) / RAND_MAX) / 10.0;
}

main() {
  int limit = 100000000;
  int j;
  time_t t1, t2;

  t1 = time(NULL);
  for (j = 0; j < limit; j++) {
    t();
  }
  t2 = time(NULL);
  printf("%d %d\n", (int) t1, (int) t2);
  printf("%e\n", ((float) (t2 - t1)) / limit);
}
