#include <stdio.h>
#include <stdint.h>
#include "hailstone.h"

/*
 * 32-bit wide integer search
 */
#if CHECK_MAXVALUE
#if NO_UPDATE
void hail32pmf
#else
void hail32pm
#endif
#elif CHECK_MAXVALUE_PARENTS
#if NO_UPDATE
void hail32plf
#else
void hail32pl
#endif
#else
#if NO_UPDATE
void hail32pnf
#else
void hail32pn
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
  uint64_t num = *n;
  uint64_t num2;
  uint64_t max = (maxvalue_size>1)? 0xfffffffffffffffflu : *maxvalue;
  int clz;
  struct poly *ptab;
#if DEBUG
  if (debug){
    printf("%s(%lu)\n",__FUNCTION__,num);
  }
#endif
#if defined CHECK_MAXVALUE
  while (num > 1)
#else
 while (num > (1<<LOOKUP_WIDTH))
#endif
   {
#if DEBUG
     if (debug){
       printf("\t%d: %lu\n",lsteps,num);
     }
#endif     
#if CHECK_MAXVALUE
     ptab = &mpoly10[num & ((1<<POLY_WIDTH)-1)];
#else
     ptab = &fpoly10[num & ((1<<POLY_WIDTH)-1)];
#endif
     num = num & ~((1<<POLY_WIDTH)-1); // get rid of bottom bits
     num = num >> ptab->div2;
     num = num * ptab->mul3 + ptab->add;
     num2 = num >> 32;
     lsteps += ptab->steps;
     if (num2){
       n[0] = num = num & 0xffffffff;
       n[1] = num2;
       *steps = lsteps;
#if CHECK_MAXVALUE || CHECK_MAXVALUE_PARENTS
       if ((maxvalue_size == 1)||
	   ((maxvalue_size == 2) &&
	    ((num2 > maxvalue[1])||
	     ((num2 == maxvalue[1]) && (num > maxvalue[0]))))){
#if NO_UPDATE
	 n[0] = 1;
	 n[1] = 0;
	 *peak_found = 1;
	 return;
#else
	 global_maxvalue_found = 1;
	 global_maxvalue[0] = num;
	 global_maxvalue[1] = num2;
	 global_maxvalue_size = 2;	  
	 maxvalue_size = 2;
#endif
       }
       hail64m(n,steps,maxsteps,maxvalue,maxvalue_size);
       max = 0xfffffffffffffffflu;
#else
       hail64n(n,steps,maxsteps,maxvalue,maxvalue_size);
#endif
       lsteps = *steps;
       lmaxsteps = *maxsteps;
       num = n[0];
#if CHECK_MAXVALUE
     } else if (num < (1<<POLY_WIDTH)){
       n[0] = num;
       *steps = lsteps;
       hail32m(n,steps,maxsteps,maxvalue,maxvalue_size);
       num = n[0];
       lsteps = *steps;
       lmaxsteps = *maxsteps;
     } else if (num > max){
#if NO_UPDATE
       // indicate a new peak
       *peak_found = 1;
#else
       global_maxvalue_found = 1;
       global_maxvalue[0] = num;
       max = num;
#endif
#endif
     }
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
