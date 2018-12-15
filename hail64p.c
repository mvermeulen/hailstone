#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>
#include "hailstone.h"
/*
 * 64-bit wide integer search using polynomials
 */
#if CHECK_MAXVALUE
#if NO_UPDATE
void hail64pmf
#else
void hail64pm
#endif
#elif CHECK_MAXVALUE_PARENTS
#if NO_UPDATE
void hail64plf
#else
void hail64pl
#endif
#else
#if NO_UPDATE
void hail64pnf
#else
void hail64pn
#endif
#endif
(uint32_t *n,                                     // input: number to search
 int32_t *steps,                                  // input/output: # steps
 int32_t *maxsteps,                               // input/output: maximum
 uint32_t *maxvalue,int32_t maxvalue_size         // input/output: maximum
#if NO_UPDATE
 ,int32_t *peak_found
#endif
 )
{
  int32_t lsteps = *steps;
  int32_t lmaxsteps = *maxsteps;
  uint64_t num = n[0];
  uint64_t num2 = n[1];
  uint64_t num3;
  uint64_t max = maxvalue[0];
  uint64_t max2 = (maxvalue_size > 2)?0xfffffffffffffffflu : maxvalue[1];
  int clz;
  int nsize;
  struct poly *ptab;
#if DEBUG
  if (debug){
    printf("%s(%lu %lu)\n",__FUNCTION__,num2,num);
  }
#endif
  while (num2 > 0){
#if DEBUG
    if (debug){ printf("\t%d: %lu %lu\n",lsteps,num2,num); }
#endif
#if CHECK_MAXVALUE
    ptab = &mpoly10[num & ((1<<POLY_WIDTH)-1)];
#else
    ptab = &fpoly10[num & ((1<<POLY_WIDTH)-1)];
#endif
    if (num2 < 65536){
      num = num & ~((1<<POLY_WIDTH)-1);
      num = (num2 << 32)|num;
      num = num >> ptab->div2;
      num = num * ptab->mul3 + ptab->add;
      num2 = num >> 32;
      num = num & 0xffffffff;
      num3 = 0;
      lsteps += ptab->steps;
    } else {
      num = num & ~((1<<POLY_WIDTH)-1); // get rid of bottom bits
      // divide by power of 2
      num = (num >> ptab->div2)|((num2 & ((1<<ptab->div2)-1))<<(32 - ptab->div2));
      num2 = num2 >> ptab->div2;
      // multiply by power of 3
      num = num * ptab->mul3 + ptab->add;
      num2 = num2 * ptab->mul3 + (num >> 32);
      num = num & 0xffffffff;
      num3 = num2 >> 32;
      lsteps += ptab->steps;
      if (num3){
	num2 = num2 & 0xffffffff;
	n[0] = num;
	n[1] = num2;
	n[2] = num3;
	*steps = lsteps;
	nsize = 3;
#if CHECK_MAXVALUE
	if ((maxvalue_size == 2)||
	    ((maxvalue_size == 3) &&
	     ((num3 > maxvalue[2])||
	      ((num3 == maxvalue[2])&&
	       ((num2 > maxvalue[1])||
		((num2 == maxvalue[1])&&
		 (num > maxvalue[0]))))))){
#if NO_UPDATE
	  *peak_found = 1;
	  n[0] = 1;
	  n[1] = 0;
	  n[2] = 0;
	  return;
#else
	  global_maxvalue_found = 1;
	  global_maxvalue[0] = maxvalue[0] = num;
	  global_maxvalue[1] = maxvalue[1] = num2;
	  global_maxvalue[2] = maxvalue[2] = num3;
	  global_maxvalue_size = maxvalue_size = 3;
#endif
	}
#if NO_UPDATE
	hailxmf(n,nsize,steps,maxsteps,maxvalue,maxvalue_size,peak_found);
#else
	hailxm(n,nsize,steps,maxsteps,maxvalue,maxvalue_size);
#endif
	maxvalue[0] = global_maxvalue[0];
	maxvalue[1] = global_maxvalue[1];
	maxvalue[2] = global_maxvalue[2];
	maxvalue_size = global_maxvalue_size;
#else
#if NO_UPDATE
	hailxnf(n,nsize,steps,maxsteps,maxvalue,maxvalue_size,peak_found);
#else
	hailxn(n,nsize,steps,maxsteps,maxvalue,maxvalue_size);
#endif
#endif
	lsteps = *steps;
	lmaxsteps = *maxsteps;
	num = n[0];
	num2 = n[1];
	max2 = 0xfffffffffffffffflu;
	nsize = 2;
	continue;
      }
    }
#if CHECK_MAXVALUE
    if ((num2 > max2)||
	((num2 == max2) && (num > max))){
#if NO_UPDATE
      *peak_found = 1;
      // do nothing
#else
      global_maxvalue_found = 1;
      global_maxvalue[0] = max = num;
      global_maxvalue[1] = max2 = num2;
      global_maxvalue_size = 2;
#endif
    }
#endif
#if !CHECK_MAXVALUE
    // clz check
    clz = __lzcnt64((num2<<32)|num);
    if (clz64[clz] + lsteps < lmaxsteps){
      n[0] = 1;
      n[1] = 0;
      *steps = lsteps;
      return;
    }
#endif
  }
  n[0] = num;
  n[1] = num2;
  *steps = lsteps;
#if CHECK_MAXSTEPS
  if (lsteps > *maxsteps){
#if NO_UPDATE
    *peak_found = 1;
#else
    global_maxsteps_found = 1;
    global_maxsteps = *maxsteps = lsteps;
#endif
  }
#endif
  return;
}
