#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "hailstone.h"

#define CUTOFF_CLZ
#define ODD_ONLY

#ifdef LOOKUP20
extern int steps20[];
#endif

#define MAXALPHA	20

#ifdef STANDALONE_TEST
int global_maxsteps;
int clz64[64] = { 0 };
#endif

void print128(unsigned __int128);
#ifdef CHECK_MAXVALUE
void print128(unsigned __int128 n){
  int i, idx;
  char digits[40];
  for (i=0;i<40;i++) digits[40] = ' ';
  idx = 0;
  while (n >= 10){
    digits[idx] = (n % 10) + '0';
    n = n / 10;
    idx++;
  }
  digits[idx] = n + '0';  
  for (i=idx;i>=0;i--){
    printf("%c",digits[i]);
  }
}
#endif

// compute a fast hailstone and set a flag if a peak is found
#ifdef CHECK_MAXVALUE
void hail64ym
#else
void hail64yn
#endif
(
	      uint64_t num,
	      int32_t steps,
#ifdef CHECK_MAXSTEPS
	      int32_t global_maxsteps,
#endif
#ifdef CHECK_MAXVALUE
	      unsigned __int128 global_maxvalue128,
#endif
	      int32_t *peak_found
){
#ifdef ODD_ONLY
  assert(num&0x1 != 0);
#endif
  
  unsigned __int128 n = num;
  int alpha, beta,gamma;
  while(n >
#if LOOKUP_WIDTH == 8
	(1<<8)
#elif LOOKUP_WIDTH == 10
	(1<<10)
#elif LOOKUP_WIDTH == 16
	(1<<16)
#elif LOOKUP_WIDTH == 20
	(1<<20)
#elif LOOKUP_WIDTH == 24
	(1<<24)
#else
	1
#endif
	){	
    n = n + 1;
    alpha = __builtin_ctzl(n);
    if (alpha > MAXALPHA) alpha = MAXALPHA;
    n = n >> alpha;
    n = n * pow3[alpha];
    n = n - 1;
#ifdef CHECK_MAXVALUE
    if (n > global_maxvalue128){
      *peak_found = 1;
      return;
    }
#endif
    beta = __builtin_ctzl(n);
    n = n >> beta;
    steps = steps + alpha + alpha + beta;
#ifdef CUTOFF_CLZ
    // cutoff if #steps + maximum for this level < global_maxsteps
    gamma = __builtin_clzl(n);
    if ((steps + clz64[gamma]) < global_maxsteps) return;
#endif
  }
#if LOOKUP_WIDTH == 8
  steps += steps8[n];
#elif LOOKUP_WIDTH == 10
  steps += steps10[n];
#elif LOOKUP_WIDTH == 16
  steps += steps16[n];
#elif LOOKUP_WIDTH == 20
  steps += steps20[n];
#elif LOOKUP_WIDTH == 24
  steps += steps24[n];
#endif
#ifdef CHECK_MAXSTEPS
  if (steps > global_maxsteps){
    *peak_found = 1;
  }
#endif
  return;
}

#ifdef STANDALONE_TEST
unsigned __int128 half_global_maxvalue128 = 0;
unsigned __int128 global_maxvalue128 = 0;
// compute a slow hailstone and update the peaks
void update_peak128(uint64_t num){
  unsigned __int128 maxvalue = 0;
  unsigned __int128 n = num;
  int steps = 0;
  while (n > 1){
    if (n & 0x1){
      n = n * 3 + 1;
      if (n > maxvalue) maxvalue = n;
    } else {
      n = n >> 1;
    }
    steps++;
  }
  if ((steps > global_maxsteps) || (maxvalue > global_maxvalue128)){
    // convert to 4x32 for printing comparison
    uint32_t maxval32[4];
    unsigned __int128 value = maxvalue;
    int i;
    int gamma;
    maxval32[0] = value & 0xffffffff;
    value = value >> 32;
    maxval32[1] = value & 0xffffffff;
    value = value >> 32;    
    maxval32[2] = value & 0xffffffff;
    maxval32[3] = value >> 32;
    printf("%lu: %d",num,steps);
    if (steps > global_maxsteps){
      global_maxsteps = steps;
      gamma = __builtin_clzl(num);
      clz64[gamma] = steps;
      printf("*");
    }
    if (maxval32[3] != 0) printf(" %u",maxval32[3]);
    if (maxval32[2] != 0) printf(" %u",maxval32[2]);
    if (maxval32[1] != 0) printf(" %u",maxval32[1]);
    printf(" %u",maxval32[0]);
    if (maxvalue > global_maxvalue128){
      half_global_maxvalue128 = maxvalue >> 1;
      global_maxvalue128 = maxvalue;
      printf("*");
    }
    printf("\n");
  }
}

int main(void){
  int peak_found;
  unsigned long n;
  int i;
  for (i=0;i<sizeof(clz64)/sizeof(clz64[0]);i++) clz64[i] = INT32_MAX/2;
  
  if (sizeof(unsigned __int128) != 16){
    fprintf(stderr,"__int128 != 16 bytes");
    return 0;
  }
#ifdef ODD_ONLY
  for (n=1;n < (1ul<<32);n += 2){  
#else
  for (n=1;n < (1ul<<32);n++){
#endif
    peak_found = 0;
#ifdef CHECK_MAXVALUE
    hail64ym(n,0,global_maxsteps,half_global_maxvalue128,&peak_found);
#else
    hail64yn(n,0,global_maxsteps,half_global_maxvalue128,&peak_found);
#endif
    if (peak_found){
      update_peak(n);
    }
  }
}
#endif
