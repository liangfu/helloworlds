#include <stdio.h>
#include <stdlib.h>

struct a_t {
  int32_t v0[2];
  int32_t v1;
};

struct b_t {
  struct a_t v0[2];
  int32_t v1;
};

int main() {
  struct a_t a = {{1, 2}, 3};
  struct b_t b = {{a, {{4, 5}, 6}}, 7};
  printf("b.v0[0]={{%d, %d}, %d}\n", b.v0[0].v0[0], b.v0[0].v0[1], b.v0[0].v1);
  printf("b.v0[1]={{%d, %d}, %d}\n", b.v0[1].v0[0], b.v0[1].v0[1], b.v0[1].v1);
  printf("b.v1=%d\n", b.v1);
  return 0;
}

/**
 We should get the following results:

 $ gcc -Wall -g -O0 hello.c -o hello
 $ ./hello
 b.v0[0]={{1, 2}, 3}
 b.v0[1]={{4, 5}, 6}
 b.v1=7
 
 */
