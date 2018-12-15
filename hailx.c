#include <stdio.h>
#include <stdint.h>
#include "hailstone.h"
/*
 * X-bit wide integer search
 */
#if CHECK_MAXVALUE
#if NO_UPDATE
void hailxmf
#else
void hailxm
#endif
#else
#if NO_UPDATE
void hailxnf
#else
void hailxn
#endif
#endif
(uint32_t *n,                                     // input: number to search
 int32_t nsize,                                   //
 int32_t *steps,                                  // input/output: # steps
 int32_t *maxsteps,                               // input/output: maximum
 uint32_t *maxvalue,int32_t maxvalue_size         // input/output: maximum
#if NO_UPDATE
 ,int32_t *peak_found
#endif
 )
{
  int i;
  int32_t lsteps = *steps;
  uint32_t carry,newcarry;
  int32_t maxfound;
  uint64_t num[MAXDIGITS] = { 0 };
  for (i=0;i<nsize;i++){
    num[i] = n[i];
  }
#if DEBUG
  if (debug) printf("%s(%lu %lu)\n",__FUNCTION__,num[1],num[0]);
#endif
  while (nsize > 1){
#if DEBUG
    if (debug){ printf("\t%lu %lu %lu %lu\n",num[3],num[2],num[1],num[0]); }
#endif
    if (num[0] & 0x1){
      carry = 1;
      for (i=0;i<nsize;i++){
	num[i] = num[i] * 3 + carry;
	carry = num[i] >> 32;
	num[i] = num[i] & 0xffffffff;
      }
      if (carry){
	num[nsize] = carry;
	nsize++;
      }
      lsteps++;
#if CHECK_MAXVALUE
      maxfound = 0;
      if (nsize > maxvalue_size){
	maxfound = 1;
      } else if (nsize < maxvalue_size){
	maxfound = 0;
      } else {
	for (i=nsize-1;i>=0;i--){
	  if (num[i] > maxvalue[i]){
	    maxfound = 1;
	    break;
	  } else if (num[i] < maxvalue[i]){
	    maxfound = 0;
	    break;
	  } 
	}
      }
      if (maxfound){
#if NO_UPDATE
	*peak_found = 1;
	for (i=0;i<nsize;i++){
	  n[i] = 0;
	}
	nsize = 1;
	n[0] = 1;
	return;
#else
	global_maxvalue_found = 1;
	global_maxvalue_size = maxvalue_size = nsize;
	for (i=0;i<nsize;i++){
	  global_maxvalue[i] = maxvalue[i] = num[i];
	}
#endif
	maxfound = 0;
      }
#endif
#if DEBUG
      if (debug){ printf("\t%lu %lu %lu %lu\n",num[3],num[2],num[1],num[0]); }
#endif
    }
    carry = 0;
    for (i=nsize-1;i>=0;i--){
      newcarry = num[i] & 0x1;
      num[i] = num[i] >> 1 | (carry << 31);
      carry = newcarry;
    }
    lsteps++;
    if (num[nsize-1] == 0){
      n[nsize-1] = 0;
      nsize--;
    }
  }
  *steps = lsteps;
  for (i=0;i<nsize;i++){
    n[i] = num[i];
  }
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

