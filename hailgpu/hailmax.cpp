// hailmax.cpp - search for peaks in values
#include <iostream>
#include "config.hpp"
#include "poly10.hpp"

extern int maxcutoff_width;
extern unsigned int maxcutoff_value[];

digit_t global_max[MAX_DIGIT] = { 0 };
int global_max_n = 1;

void report_peak(digit_t *n){
  int i;
  int highdigit = 0;
  std::cout << "Peak in values:";
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
  std::cout << " for <" << n[3] << "," << n[2] << "," << n[1] << "," << n[0] << ">" << std::endl;
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
  do {
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
  do {
    lastbits = n0 & ((1<<1ul << POLY_WIDTH)-1);
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

int main(void){
  search_block0();
}
