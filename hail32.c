#include <stdio.h>
#include <stdint.h>
#include "hailstone.h"

/*
 * 32-bit wide integer search
 */
#if CHECK_MAXVALUE
void hail32m
#elif CHECK_MAXVALUE_PARENTS
void hail32l
#else
void hail32n
#endif
(uint32_t *n,                                     // input: number to search
 int32_t *steps,                                  // input/output: # steps
 int32_t *maxsteps,                               // input/output: maximum
 uint32_t *maxvalue,int32_t maxvalue_size)        // input/output: maximum
{
  int32_t lsteps = *steps;
  int32_t lmaxsteps = *maxsteps;
  uint64_t num = *n;
  uint64_t num2;
  uint64_t max = (maxvalue_size>1)? 0xfffffffffffffffflu : *maxvalue;
  int count,clz;
#if DEBUG
  if (debug) printf("%s(%lu)\n",__FUNCTION__,num);
#endif
#if defined CHECK_MAXVALUE
  while (num > 1)
#else
 while (num > (1<<LOOKUP_WIDTH))
#endif
   {
#if DEBUG
     if (debug) printf("\t%d: %lu\n",lsteps,num);
#endif
     if (num & 0x1){
       num = num * 3 + 1;
       num2 = num >> 32;
       lsteps++;
#if DEBUG
       if (debug) printf("\t%d: %lu\n",lsteps,num);
#endif
       if (num2){
	 n[0] = num & 0xffffffff;
	 n[1] = num2;
	 *steps = lsteps;
#if CHECK_MAXVALUE || CHECK_MAXVALUE_PARENTS
	 if ((maxvalue_size == 1) ||
	     ((maxvalue_size == 2) && 
	      ((num2 > maxvalue[1]) ||
	       ((num2 == maxvalue[1]) && (num > maxvalue[0]))))){
	   global_maxvalue_found = 1;
	   global_maxvalue[0] = num;
	   global_maxvalue[1] = num2;
	   global_maxvalue_size = 2;	  
	   maxvalue_size = 2;
	 }
	 hail64m(n,steps,maxsteps,maxvalue,maxvalue_size);
#else
	 hail64n(n,steps,maxsteps,maxvalue,maxvalue_size);
#endif
	 lsteps = *steps;
	 lmaxsteps = *maxsteps;
	 num = n[0];
	 max = 0xfffffffffffffffflu;
	 continue;
       }
#if CHECK_MAXVALUE
       if (num > max){
	 global_maxvalue_found = 1;
	 global_maxvalue[0] = max = num;
	 global_maxvalue_size = 1;
       }
#endif
     }
     count = __builtin_ffs(num)-1;
     num = num >> count;
     lsteps += count;
#if !CHECK_MAXVALUE
     // clz check
     clz = __builtin_clz(num);
     if (clz32[clz] + lsteps < lmaxsteps){
       *n = 1;
       *steps = lsteps;
       return;
     }
#endif
   }
#if !CHECK_MAXVALUE
  // look up the last 20 steps
  if (num < (1<<LOOKUP_WIDTH)){
    lsteps += steps20[num];
  }
#endif
  
  *n = num;
  *steps = lsteps;
#if CHECK_MAXSTEPS
  if (lsteps > *maxsteps){
    global_maxsteps_found = 1;
    global_maxsteps = *maxsteps = lsteps;
  }
#endif
  return;
}
