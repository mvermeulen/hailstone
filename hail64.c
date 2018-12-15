#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>
#include "hailstone.h"
/*
 * 64-bit wide integer search
 */
#if CHECK_MAXVALUE
void hail64m
#elif CHECK_MAXVALUE_PARENTS
void hail64l
#else
void hail64n
#endif
(uint32_t *n,                                     // input: number to search
 int32_t *steps,                                  // input/output: # steps
 int32_t *maxsteps,                               // input/output: maximum
 uint32_t *maxvalue,int32_t maxvalue_size)        // input/output: maximum
{
  int32_t lsteps = *steps;
  int32_t lmaxsteps = *maxsteps;
  uint64_t num = n[0];
  uint64_t num2 = n[1];
  uint64_t num3;
  uint64_t max = maxvalue[0];
  uint64_t max2 = (maxvalue_size > 2)?0xfffffffffffffffflu : maxvalue[1];
  int nsize;
  uint64_t numx;
  int count,clz;
#if DEBUG
  if (debug){
    printf("%s(%lu %lu)\n",__FUNCTION__,num2,num);
    printf("\tmaxvalue_size = %d\n",maxvalue_size);
    printf("\tmaxvalue = %u %u %u\n",maxvalue[2],maxvalue[1],maxvalue[0]);
  }
#endif
  while (num2 > 0){
#if DEBUG
    if (debug){ printf("\t%d: %lu %lu\n",lsteps,num2,num); }
#endif
    if (num & 0x1){
      num = num * 3 + 1;
      num2 = num2 * 3 + (num >> 32);
      num = num & 0xffffffff;
      num3 = num2 >> 32;
      lsteps++;
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
	     ((num3 > maxvalue[2]) ||
	      (num3 == maxvalue[2] &&
	       ((num2 > maxvalue[1])||
		((num2 == maxvalue[1]) && (num > maxvalue[0]))))))){
#if DEBUG
	  if (debug){ printf("maxvalue %lu %lu %lu\n",num3,num2,num); }
#endif
	  global_maxvalue_found = 1;
	  global_maxvalue[0] = maxvalue[0] = num;
	  global_maxvalue[1] = maxvalue[1] = num2;
	  global_maxvalue[2] = maxvalue[2] = num3;
	  global_maxvalue_size = maxvalue_size = 3;	  	  
	}
	hailxm(n,nsize,steps,maxsteps,maxvalue,maxvalue_size);
	maxvalue[0] = global_maxvalue[0];
	maxvalue[1] = global_maxvalue[1];
	maxvalue[2] = global_maxvalue[2];
	maxvalue_size = global_maxvalue_size;
#else
	hailxn(n,nsize,steps,maxsteps,maxvalue,maxvalue_size);
#endif
	lsteps = *steps;
	lmaxsteps = *maxsteps;
	num = n[0];
	num2 = n[1];
	max2 = 0xfffffffffffffffflu;
	nsize = 2;
	continue;
      }
#if CHECK_MAXVALUE
      if ((num2 > max2)||
	  ((num2 == max2) && (num > max))){
	global_maxvalue_found = 1;
	global_maxvalue[0] = max = num;
	global_maxvalue[1] = max2 = num2;
	global_maxvalue_size = 2;
      }
#endif
#if DEBUG
      if (debug){ printf("\t%d: %lu %lu\n",lsteps,num2,num); }
#endif
    }
#if 0
    num = (num >> 1)|((num2 & 0x1)<<31);
    num2 = num2 >> 1;
    lsteps++;
#endif
    //#if 0
    numx = (num2 << 32)| num;
    count = __builtin_ffsl(numx)-1;
    numx = numx >> count;
    num2 = numx >> 32;
    num = numx & 0xffffffff;
    lsteps += count;
    //#endif
#if !CHECK_MAXVALUE
    //#if 0
    // clz check
    clz = __lzcnt64(numx);
    if (clz64[clz] + lsteps < lmaxsteps){
      n[0] = 1;
      n[1] = 0;
      *steps = lsteps;
      return;
    }
    //#endif
#endif
  }
  n[0] = num;
  n[1] = num2;
  *steps = lsteps;
#if CHECK_MAXSTEPS
  if (lsteps > *maxsteps){
    global_maxsteps_found = 1;
    global_maxsteps = *maxsteps = lsteps;
  }
#endif
  return;
}
