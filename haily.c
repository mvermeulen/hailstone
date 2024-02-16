#include <stdio.h>
#include <stdint.h>
#include "hailstone.h"

unsigned long pow3[] = {
  1,
  3,
  3*3,
  3*3*3,
  3*3*3*3,
  3*3*3*3*3,
  3*3*3*3*3*3,
  3*3*3*3*3*3*3,
  3*3*3*3*3*3*3*3,
  3*3*3*3*3*3*3*3*3,
  3*3*3*3*3*3*3*3*3*3,  // 10
  3*3*3*3*3*3*3*3*3*3*3,  
  3*3*3*3*3*3*3*3*3*3*3*3,
  3*3*3*3*3*3*3*3*3*3*3*3*3,
  3*3*3*3*3*3*3*3*3*3*3*3*3*3,
  3*3*3*3*3*3*3*3*3*3*3*3*3*3*3,
  3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3,
  3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3,
  3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3,
  3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3,
  3ul*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3, // 20
};
  

#define MAXALPHA	4

void haily(uint64_t n,             // %rdi
	   uint64_t steps,         // %rsi
	   uint64_t maxsteps,      // %rdx
	   uint64_t maxvalue_size, // %rcx
	   uint32_t *maxvalue,     // %r8
	   int32_t *peak_found)    // %r9
{
  int alpha;
  int beta;
  printf("haily(%lu)\n",n);
  while(n > 1){
    printf("%lu: %lu\n",steps,n);
    n = n + 1;                                   // overflow
    alpha = __builtin_ctz(n);
    if (alpha > MAXALPHA) alpha = MAXALPHA;
    n = n >> alpha;                              // shift two...
    n = n * pow3[alpha];                         // multiply 64x64-->128
    n = n - 1;
    printf("  max=%lu, alpha=%d\n",n<<1,alpha);  // check max
    beta = __builtin_ctz(n);
    n = n >> beta;                               // shift two
    steps = steps + alpha + alpha + beta;
  }
  printf("%lu: %lu\n",steps,n);                  // check steps
}

int main(void){
  uint32_t maxvalue[MAXDIGITS];
  int peak_found;
  haily(27,0,23,1,maxvalue,&peak_found);
}
