#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "assert.h"

typedef int FLOAT;

static inline int F2int(FLOAT a) {
  //assert(0);
  //return 0;
  if ((a & 0x80000000) == 0)
    return a >> 16;
  else
    return -((-a) >> 16);
}

static inline FLOAT int2F(int a) {
  //assert(0);
  //return 0;
  if ((a & 0x80000000) == 0)
    return a << 16;
  else
    return -((-a) << 16);
}

static inline FLOAT F_mul_int(FLOAT a, int b) {
  //assert(0);
  //return 0;
  return a*b;
}

static inline FLOAT F_div_int(FLOAT a, int b) {
  //assert(0);
  //return 0;
  return a/b;
}

FLOAT f2F(float);
FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);
FLOAT Fabs(FLOAT);
FLOAT Fsqrt(FLOAT);
FLOAT Fpow(FLOAT, FLOAT);

#endif
