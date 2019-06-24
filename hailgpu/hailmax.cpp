// hailmax.cpp - search for peaks in values
#include <iostream>
#include <omp.h>
#include <string.h>
#include "config.hpp"
#include "poly10.hpp"
#include "poly16.hpp"

extern int maxcutoff_width;
extern int maxcutoff_num;
extern unsigned int maxcutoff_value[];
extern int maxcutoff1_num;
extern unsigned int maxcutoff1_value[];
extern int maxcutoff3_num;
extern unsigned int maxcutoff3_value[];
extern int maxcutoff5_num;
extern unsigned int maxcutoff5_value[];

digit_t global_max[MAX_DIGIT] = { 0 };
wdigit_t global_wmax[MAX_DIGIT] = { 0 };
xdigit_t global_xmax[MAX_DIGIT] = { 0 };
int global_max_n = 1;

void report_peak(digit_t *n){
  int i;
  int highdigit = 0;
  std::cout << std::hex << "Peak in values:";
  for (i=MAX_DIGIT-1;i>=0;i--){
    if (highdigit){
      std::cout << " " << global_max[i];
    } else if (global_max[i] == 0){
      continue;
    } else {
      std::cout << " " << global_max[i];
      highdigit = 1;
    }
  }
  std::cout << " for <" << n[3] << "," << n[2] << "," << n[1] << "," << n[0] << ">" << std::dec << std::endl;
}

void report_wpeak(wdigit_t *n){
  int i;
  int highdigit = 0;
  std::cout << std::hex << "Peak in values:";
  for (i=MAX_DIGIT-1;i>=0;i--){
    if (highdigit){
      std::cout << " " << global_wmax[i];
    } else if (global_wmax[i] == 0){
      continue;
    } else {
      std::cout << " " << global_wmax[i];
      highdigit = 1;
    }
  }
  std::cout << " for <" << n[3] << "," << n[2] << "," << n[1] << "," << n[0] << ">" << std::dec << std::endl;
}

void report_xpeak(xdigit_t *n){
  long i;
  int highdigit = 0;
  std::cout << std::hex << "Peak in values:";
  for (i=MAX_DIGIT-1;i>=0;i--){
    if (highdigit){
      std::cout << " " << global_xmax[i];
    } else if (global_xmax[i] == 0){
      continue;
    } else {
      std::cout << " " << global_xmax[i];
      highdigit = 1;
    }
  }
  std::cout << " for <" << n[3] << "," << n[2] << "," << n[1] << "," << n[0] << ">" << std::dec << std::endl;
}

// search for maximum value up to 2^16
void hailmax16(digit_t *n,int *maxfound){
  unsigned long int maxvalue = global_max[0] + ((unsigned long) global_max[1] << 16)
    + ((unsigned long) global_max[2] << 32) + ((unsigned long) global_max[3] << 48);
  unsigned long int nvalue = n[0];
  unsigned long int start = n[0];
  do {
    if (nvalue & 0x1){
      nvalue = nvalue * 3 + 1;
      if (nvalue > maxvalue){
	*maxfound = 1;
	maxvalue = nvalue;
	global_max[0] = nvalue & 0xffff;
	global_max[1] = (nvalue >> 16) & 0xffff;
	global_max[2] = (nvalue >> 32) & 0xffff;
	global_max[3] = (nvalue >> 48) & 0xffff;	
      }
    } else {
      nvalue = nvalue >> 1;
      if (nvalue <= start) return;
    }
  } while(1);
}

// search for maximum value up to 2^32
void hailmax32(digit_t *n,int *maxfound){
  unsigned int n0 = n[0], n1 = n[1], n2 = 0, n3 = 0;
  unsigned int start0 = n0, start1 = n1;
  //  std::cout << "hailmax32(" << n1 << "," << n0 << ")" << std::endl;
  do {
    // std::cout << "\t" << (n1<<16)+n0 << std::endl;
    if (n0 & 0x1){
      n0 = n0 * 3 + 1;
      n1 = n1 * 3 + (n0 >> 16);
      n2 = n2 * 3 + (n1 >> 16);
      n3 = n3 * 3 + (n2 >> 16);
      n0 &= 0xffff;
      n1 &= 0xffff;
      n2 &= 0xffff;
      n3 &= 0xffff;
      if (n3 > global_max[3]){
	*maxfound = 1;
	global_max[3] = n3;
	global_max[2] = n2;
	global_max[1] = n1;
	global_max[0] = n0;
	continue;
      } else if (n3 < global_max[3]){
	continue;
      }
      // n3 == global_max[3]
      if (n2 > global_max[2]){
	*maxfound = 1;
	global_max[2] = n2;
	global_max[1] = n1;
	global_max[0] = n0;
	continue;
      } else if (n2 < global_max[2]){
	continue;
      }
      // n2 == global_max[2]
      if (n1 > global_max[1]){
	*maxfound = 1;
	global_max[1] = n1;
	global_max[0] = n0;
	continue;
      } else if (n1 < global_max[1]){
	continue;
      }
      // n1 == global_max[1]
      if (n0 > global_max[0]){
	*maxfound = 0;
	global_max[0] = n0;
	continue;
      }
    } else {
      n0 = (n0 >> 1) | ((n1 & 0x1)<<15);
      n1 = (n1 >> 1) | ((n2 & 0x1)<<15);
      n2 = (n2 >> 1) | ((n3 & 0x1)<<15);
      n3 = (n3 >> 1);
      if ((n3 == 0) && (n2 == 0) && ((n1 < start1) || ((n1 == start1) && (n0 <= start0)))) return;
    }
  } while (1);
}

// search for maximum value up to 2^32
void hailmax32p(digit_t *n,int *maxfound){
  unsigned int n0 = n[0], n1 = n[1], n2 = 0, n3 = 0;
  unsigned int start0 = n0, start1 = n1;
  unsigned int lastbits;
  unsigned int shiftcnt;
  unsigned int mul3val;
  unsigned int mask = ((1u << POLY_WIDTH)-1);
  //  std::cout << "hailmax32p(" << n1 << "," << n0 << ")" << std::endl;
  do {
    //    std::cout << "\t" << (n1<<16)+n0 << std::endl;    
    lastbits = n0 & mask;
    n0 = n0 & ~mask;
    // shift by power of 2
    if (shiftcnt = poly_table[lastbits].pow2){
      n0 = (n0 >> shiftcnt) | (n1 << (16-shiftcnt));
      n1 = (n1 >> shiftcnt) | (n2 << (16-shiftcnt));
      n2 = (n2 >> shiftcnt) | (n3 << (16-shiftcnt));
      n3 = (n3 >> shiftcnt);
      n0 &= 0xffff;
      n1 &= 0xffff;
      n2 &= 0xffff;
    }
    // multiply by power of 3 and add
    mul3val = poly_table[lastbits].pow3;
    n0 = n0 * mul3val + poly_table[lastbits].add;
    n1 = n1 * mul3val + (n0 >> 16);
    n2 = n2 * mul3val + (n1 >> 16);
    n3 = n3 * mul3val + (n2 >> 16);
    n0 &= 0xffff;
    n1 &= 0xffff;
    n2 &= 0xffff;
    // check for greater or less
    if (poly_table[lastbits].gtone){
      if (n3 > global_max[3]){
	*maxfound = 1;
	global_max[3] = n3;
	global_max[2] = n2;
	global_max[1] = n1;
	global_max[0] = n0;
	continue;
      } else if (n3 < global_max[3]){
	continue;
      }
      // n3 == global_max[3]
      if (n2 > global_max[2]){
	*maxfound = 1;
	global_max[2] = n2;
	global_max[1] = n1;
	global_max[0] = n0;
	continue;
      } else if (n2 < global_max[2]){
	continue;
      }
      // n2 == global_max[2]
      if (n1 > global_max[1]){
	*maxfound = 1;
	global_max[1] = n1;
	global_max[0] = n0;
	continue;
      } else if (n1 < global_max[1]){
	continue;
      }
      // n1 == global_max[1]
      if (n0 > global_max[0]){
	*maxfound = 0;
	global_max[0] = n0;
	continue;
      }      
    } else {
      if ((n3 == 0) && (n2 == 0) && ((n1 < start1) || ((n1 == start1) && (n0 <= start0)))) return;      
    }
  } while (1);
}

// search for maximum value up to 2^24
void hailwmax24p(wdigit_t *n,int *maxfound){
  unsigned int n0 = n[0], n1 = 0;
  unsigned int lastbits;
  unsigned int mask = ((1u << POLY_WIDTH)-1);
  unsigned int shiftcnt;
  unsigned int mul3val;
  unsigned int start = n0;
  //  std::cout << "hailwmax24p - " << n0 << std::endl;      
  do {
    // std::cout << "val - " << n0 << std::endl;    
    lastbits = n0 & mask;
    n0 = n0 & ~mask;
    // shift by power of 2
    if (shiftcnt = poly_table[lastbits].pow2){
      n0 = (n0 >> shiftcnt) | (n1 << (24-shiftcnt));
      n1 = n1 >> shiftcnt;
      n0 &= 0xffffff;
    }
    // multiply by power of 3 and add
    mul3val = poly_table[lastbits].pow3;
    n0 = n0 * mul3val + poly_table[lastbits].add;
    n1 = n1 * mul3val + (n0 >> 24);
    n0 &= 0xffffff;
    // check for greater or less
    if (poly_table[lastbits].gtone){
      if (n1 > global_wmax[1]){
	*maxfound = 1;
	global_wmax[0] = n0;
	global_wmax[1] = n1;
	continue;
      } else if (n1 < global_wmax[1]){
	continue;
      }
      // n1 == global_max[1]
      if (n0 > global_wmax[0]){
	// std::cout << "max - " << n0 << std::endl;
	*maxfound = 1;
	global_wmax[0] = n0;
	global_wmax[1] = n1;
      }
      continue;
    } else {
      if ((n1 == 0) && (n0 <= start)) return;
    }
  } while (1);
}

// search for maximum value up to 2^36
void hailwmax36p(wdigit_t *n,int *maxfound){
  unsigned int n0 = n[0], n1 = n[1], n2 = 0;
  unsigned int lastbits;
  unsigned int mask = ((1u << POLY_WIDTH)-1);
  unsigned int shiftcnt;
  unsigned int mul3val;
  unsigned int start0 = n0, start1 = n1;
  //  std::cout << "hailwmax24p - " << n0 << std::endl;      
  do {
    // std::cout << "val - " << n0 << std::endl;    
    lastbits = n0 & mask;
    n0 = n0 & ~mask;
    // shift by power of 2
    if (shiftcnt = poly_table[lastbits].pow2){
      n0 = (n0 >> shiftcnt) | (n1 << (24-shiftcnt));
      n1 = (n1 >> shiftcnt) | (n2 << (24-shiftcnt));
      n2 = (n2 >> shiftcnt);
      n0 &= 0xffffff;
      n1 &= 0xffffff;
    }
    // multiply by power of 3 and add
    mul3val = poly_table[lastbits].pow3;
    n0 = n0 * mul3val + poly_table[lastbits].add;
    n1 = n1 * mul3val + (n0 >> 24);
    n2 = n2 * mul3val + (n1 >> 24);
    n0 &= 0xffffff;
    n1 &= 0xffffff;    
    // check for greater or less
    if (poly_table[lastbits].gtone){
      if (n2 > global_wmax[2]){
	*maxfound = 1;
	global_wmax[0] = n0;
	global_wmax[1] = n1;
	global_wmax[2] = n2;	
	continue;
      } else if (n2 < global_wmax[2]){
	continue;
      }
      // at this point n2 == global_wmax[2]
      if (n1 > global_wmax[1]){
	*maxfound = 1;
	global_wmax[0] = n0;
	global_wmax[1] = n1;	
      } else if (n1 < global_wmax[1]){
	continue;
      }
      // at this point n1 == global_wmax[1]
      if (n0 > global_wmax[0]){
	// std::cout << "max - " << n0 << std::endl;
	*maxfound = 1;
	global_wmax[0] = n0;
      }
      continue;
    } else {
      if ((n1 < start1)||((n1 == start1) && (n0 <= start1))) return;
    }
  } while (1);
}

// search for maximum value up to 2^48
int hailwmax48p(wdigit_t start1, wdigit_t start0, int setglobalpeak){
  unsigned int n0 = start0, n1 = start1, n2 = 0, n3 = 0;
  unsigned int lastbits;
  unsigned int mask = ((1u << POLY_WIDTH)-1);
  unsigned int shiftcnt;
  unsigned int mul3val;
  int maxfound = 0;
  do {
#if 0
    std::cout << std::hex << "\t" << n3 << " " << n2 << " " << n1 << " " << n0 << std::dec << std::endl;
#endif

    lastbits = n0 & mask;
    n0 = n0 & ~mask;
    // shift by power of 2
    if (shiftcnt = poly_table[lastbits].pow2){
      n0 = (n0 >> shiftcnt) | (n1 << (24-shiftcnt));
      n1 = (n1 >> shiftcnt) | (n2 << (24-shiftcnt));
      n2 = (n2 >> shiftcnt) | (n3 << (24-shiftcnt));
      n3 = (n3 >> shiftcnt);
      n0 &= 0xffffff;
      n1 &= 0xffffff;
      n2 &= 0xffffff;
    }
    // multiply by power of 3 and add
    mul3val = poly_table[lastbits].pow3;
    n0 = n0 * mul3val + poly_table[lastbits].add;
    n1 = n1 * mul3val + (n0 >> 24);
    n2 = n2 * mul3val + (n1 >> 24);
    n3 = n3 * mul3val + (n2 >> 24);
    n0 &= 0xffffff;
    n1 &= 0xffffff;    
    n2 &= 0xffffff;    
    // check for greater or less
    if (poly_table[lastbits].gtone){
      if (n3 > global_wmax[3]){
	maxfound = 1;
	if (setglobalpeak){
	  global_wmax[0] = n0;
	  global_wmax[1] = n1;
	  global_wmax[2] = n2;	
	  global_wmax[3] = n3;
	} else return maxfound;
	continue;
      } else if (n3 < global_wmax[3]){
	continue;
      }
      // at this point n3 == global_wmax[2]
      if (n2 > global_wmax[2]){
	maxfound = 1;
	if (setglobalpeak){
	  global_wmax[0] = n0;
	  global_wmax[1] = n1;
	  global_wmax[2] = n2;
	} else return maxfound;
	continue;
      } else if (n2 < global_wmax[2]){
	continue;
      }
      // at this point n2 == global_wmax[2]
      if (n1 > global_wmax[1]){
	maxfound = 1;
	if (setglobalpeak){
	  global_wmax[0] = n0;
	  global_wmax[1] = n1;
	} else return maxfound;
      } else if (n1 < global_wmax[1]){
	continue;
      }
      // at this point n1 == global_wmax[1]
      if (n0 > global_wmax[0]){
	// std::cout << "max - " << n0 << std::endl;
	maxfound = 1;
	if (setglobalpeak){
	  global_wmax[0] = n0;
	} else return maxfound;
      }
      continue;
    } else {
      if ((n3 == 0) &&
	  (n2 == 0) &&
	  ((n1 < start1)||
	   ((n1 == start1) && (n0 <= start1)))){
	return maxfound;
      }
    }
  } while (1);
}

// search for maximum value up to 2^48
int hailxmax48p(xdigit_t start0, int setglobalpeak){
  unsigned long n0 = start0, n1 = 0;
  unsigned long lastbits;
  unsigned long mask = ((1u << POLY_XWIDTH)-1);
  unsigned long shiftcnt;
  unsigned long mul3val;
  unsigned long start = n0;
  int maxfound = 0;
  //  std::cout << "hailxmax48p - " << n0 << std::endl;      
  do {
    // std::cout << "val - " << n0 << std::endl;    
    lastbits = n0 & mask;
    n0 = n0 & ~mask;
    // shift by power of 2
    if (shiftcnt = polyx_table[lastbits].pow2){
      n0 = (n0 >> shiftcnt) | (n1 << (48-shiftcnt));
      n1 = n1 >> shiftcnt;
      n0 &= 0xffffffffffff;
    }
    // multiply by power of 3 and add
    mul3val = polyx_table[lastbits].pow3;
    n0 = n0 * mul3val + polyx_table[lastbits].add;
    n1 = n1 * mul3val + (n0 >> 48);
    n0 &= 0xffffffffffff;
    // check for greater or less
    if (polyx_table[lastbits].gtone){
      if (n1 > global_xmax[1]){
	maxfound = 1;
	if (setglobalpeak){
	  global_xmax[0] = n0;
	  global_xmax[1] = n1;
	}
	continue;
      } else if (n1 < global_xmax[1]){
	continue;
      }
      // n1 == global_max[1]
      if (n0 > global_xmax[0]){
	// std::cout << "max - " << n0 << std::endl;
	maxfound = 1;
	if (setglobalpeak){
	  global_xmax[0] = n0;
	  global_xmax[1] = n1;
	}
      }
      continue;
    } else {
      if ((n1 == 0) && (n0 <= start)) return maxfound;
    }
  } while (1);
}

void search_block0(void){
  unsigned int i,j;
  int maxfound = 0;
  digit_t n[MAX_DIGIT] = { 0 };

  for (j=1;j<(1l<<16);j++){
    n[0] = j;
    hailmax16(n,&maxfound);
    if (maxfound){
      report_peak(n);
      maxfound = 0;
    }
  }
  for (i=1;i<(1l<<16);i++){
    n[1] = i;
    for (j=0;j<(1l<<16);j++){
      n[0] = j;
      hailmax32p(n,&maxfound);
      if (maxfound){
	report_peak(n);
	maxfound = 0;
      }
    }
  }  
}

void wsearch_block0(void){
  unsigned int i,j;
  int maxfound = 0;
  wdigit_t n[MAX_DIGIT] = { 0 };

  for (j=1;j<(1l<<24);j++){
    n[0] = j;
    hailwmax24p(n,&maxfound);
    if (maxfound){
      report_wpeak(n);
      maxfound = 0;
    }
  }
}

void xsearch_block0(void){
  unsigned long i,j;
  int maxfound = 0;
  xdigit_t n;

  for (i=0;i<(1l<<24);i++){
    for (j=0;j<maxcutoff_num;j++){
      n = (i<<24)+maxcutoff_value[j];
      if (hailxmax48p(n,1)){
	report_xpeak(global_xmax);
	maxfound = 0;
      }
    }
  }
}

int wsearch_blockn(unsigned int start){
  int num_blocks = omp_get_num_procs();
  int found_peak = 0;
  int i,j;
  unsigned int x0,x1;
  // try searching in parallel
#pragma omp parallel for shared(found_peak) private(j,x0,x1)
  for (i = 0;i < num_blocks;i++){
    x1 = start+i;
    switch(x1%3){
    case 0:
      for (j=0;j<maxcutoff5_num;j++){
	x0 = maxcutoff5_value[j];
	if (hailwmax48p(x1,x0,0)) found_peak = 1;
      }
      break;
    case 1:
      for (j=0;j<maxcutoff1_num;j++){
	x0 = maxcutoff1_value[j];
	if (hailwmax48p(x1,x0,0)) found_peak = 1;
      }
      break;
    case 2:
      for (j=0;j<maxcutoff3_num;j++){
	x0 = maxcutoff3_value[j];
	if (hailwmax48p(x1,x0,0)) found_peak = 1;
      }
      break;      
    }
  }
  if (!found_peak) return num_blocks;
  // A peak was found, do a slow sequential search
  wdigit_t x[MAX_DIGIT] = { 0 };
  x1 = start;
  switch((x1)%3){
  case 0:
    for (j=0;j<maxcutoff5_num;j++){
      x0 = maxcutoff5_value[j];
      if (hailwmax48p(x1,x0,1)){
	x[1] = x1;
	x[0] = x0;
	report_wpeak(x);
      }
    }
    break;
  case 1:
    for (j=0;j<maxcutoff1_num;j++){
      x0 = maxcutoff1_value[j];
      if (hailwmax48p(x1,x0,1)){
	x[1] = x1;
	x[0] = x0;	
	report_wpeak(x);
      }
    }
    break;
  case 2:
    for (j=0;j<maxcutoff3_num;j++){
      x0 = maxcutoff3_value[j];
      if (hailwmax48p(x1,x0,1)){
	x[1] = x1;
	x[0] = x0;	
	report_wpeak(x);
      }
    }
    break;      
  }
  return 1;
}

int xsearch_blockn(unsigned int start){
  int num_blocks = omp_get_num_procs();
  int found_peak = 0;
  int i,j;
  unsigned int x0,x1;
  unsigned long val;
  // try searching in parallel
#pragma omp parallel for shared(found_peak) private(j,x0,x1)
  for (i = 0;i < num_blocks;i++){
    x1 = start+i;
    switch(x1%3){
    case 0:
      for (j=0;j<maxcutoff5_num;j++){
	x0 = maxcutoff5_value[j];
	val = x1<<24+x0;
	if (hailxmax48p(val,0)) found_peak = 1;
      }
      break;
    case 1:
      for (j=0;j<maxcutoff1_num;j++){
	x0 = maxcutoff1_value[j];
	val = x1<<24+x0;	
	if (hailxmax48p(val,0)) found_peak = 1;
      }
      break;
    case 2:
      for (j=0;j<maxcutoff3_num;j++){
	x0 = maxcutoff3_value[j];
	val = x1<<24+x0;	
	if (hailxmax48p(val,0)) found_peak = 1;
      }
      break;      
    }
  }
  if (!found_peak) return num_blocks;
  // A peak was found, do a slow sequential search
  xdigit_t x[MAX_DIGIT] = { 0 };
  x1 = start;
  switch((x1)%3){
  case 0:
    for (j=0;j<maxcutoff5_num;j++){
      x0 = maxcutoff5_value[j];
      val = x1<<24+x0;      
      if (hailxmax48p(val,1)){
	x[0] = val;
	report_xpeak(x);
      }
    }
    break;
  case 1:
    for (j=0;j<maxcutoff1_num;j++){
      x0 = maxcutoff1_value[j];
      val = x1<<24+x0;      
      if (hailxmax48p(val,1)){
	x[0] = val;	
	report_xpeak(x);
      }
    }
    break;
  case 2:
    for (j=0;j<maxcutoff3_num;j++){
      x0 = maxcutoff3_value[j];
      val = x1<<24+x0;      
      if (hailxmax48p(val,1)){
	x[0] = val;	
	report_xpeak(x);
      }
    }
    break;      
  }
  return 1;
}

#if 0
void runtest(){
  wdigit_t n[MAX_DIGIT] = { 0x41f567, 0x36151 };
  int maxfound = 0;
  if (hailwmax48p(n[1],n[0],1)) report_wpeak(n);
  n[1] = 0x83348;
  n[0] = 0x1ACDEF;
  if (hailwmax48p(n[1],n[0],1)) report_wpeak(n);  
}
#endif

int main(int argc,char **argv){
  int maxfound = 0;
  unsigned int block = 1, count;

  if (argc > 1 && !strcmp(argv[1],"x")){
    xsearch_block0();
    while (block < (1u<<24)){
      //    std::cout << std::hex << "search " << block << std::dec << std::endl;
      count = xsearch_blockn(block);
      if (count == 0) break;
      block += count;
      return 0;
    }
  }
  
  wsearch_block0();
  while (block < (1u<<24)){
    //    std::cout << std::hex << "search " << block << std::dec << std::endl;
    count = wsearch_blockn(block);
    if (count == 0) break;
    block += count;
  }
}
