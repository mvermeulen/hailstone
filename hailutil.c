/*
 * hailutil.c - general utility routines used by both daemon and manager
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <x86intrin.h>
#include "hailstone.h"


// initialize globals
int32_t global_maxvalue_found __attribute__ ((aligned(16))) = 0;
int32_t global_maxsteps_found __attribute__ ((aligned(16))) = 0;
int32_t global_maxvalue_size __attribute__ ((aligned(16)))= 1;
int32_t global_maxsteps __attribute__ ((aligned(16))) = 0;
uint32_t global_maxvalue[MAXDIGITS] = { 0 };

// maximum # of steps for each power of two; (INT32_MAX/2) if unknown
int clz32[32] = {
  1050, 1008, 986, 964,
  956, 950, 949, 705,
  704, 664, 596, 556,
  524, 469, 442, 353,
  339, 307, 275, 261,
  237, 181, 178, 143,
  127, 118, 112, 111,
  19, 16, 7, 0,
};
int clz64[64] = { 
  INT32_MAX/2, INT32_MAX/2, INT32_MAX/2, INT32_MAX/2,
  INT32_MAX/2, INT32_MAX/2, INT32_MAX/2, INT32_MAX/2,
  INT32_MAX/2, INT32_MAX/2, INT32_MAX/2, INT32_MAX/2,
  INT32_MAX/2, INT32_MAX/2, 1862, 1856,
  1847, 1823, 1659, 1601,
  1569, 1563, 1549, 1437,
  1348, 1321, 1307, 1234,
  1220, 1219, 1210, 1131,
  1050, 1008, 986, 964,
  956, 950, 949, 705,
  704, 664, 596, 556,
  524, 469, 442, 353,
  339, 307, 275, 261,
  237, 181, 178, 143,
  127, 118, 112, 111,
  19, 16, 7, 0,  
};

extern int bflag;
extern uint32_t start_block;
extern FILE *logfile;
extern char *checkpoint_file;

struct predict {
  unsigned long int blocknum;
  struct predict *next;
};
struct predict *predicted_blocks = NULL;
void add_predict_block(unsigned long int blocknum){
  struct predict *p;
  for (p = predicted_blocks;p;p = p->next){
    if (p->blocknum == blocknum)
      return; // already predicted
  }
  p = malloc(sizeof(struct predict));
  p->blocknum = blocknum;
  p->next = predicted_blocks;
  predicted_blocks = p;
}

// see if this block was predicted
int check_prediction(unsigned long int blocknum){
  struct predict *p;
  if (blocknum == 0) return 1;
  for (p = predicted_blocks;p;p = p->next){
    if (p->blocknum == blocknum) return 1;
  }
  return 0;
}

// remove a block that was predicted (if needed)
void remove_prediction(unsigned long int blocknum){
  struct predict *p,*newp,*nextp;
  if (blocknum == 0) return;
  p = predicted_blocks;
  newp = NULL;
  while (p){
    nextp = p->next;
    if (p->blocknum != blocknum){
      p->next = newp;
      newp = p;
    }
    p = nextp;
  }
  predicted_blocks = newp;
}

void record_peak(unsigned long int n,unsigned long int block){
  time_t now;
  struct tm tmval;
  char timebuf[120];
  int predict_double = 0,predict_four = 0;
  int i,clz_value;
  unsigned long int predicted;
  time(&now);
  localtime_r(&now,&tmval);
  strftime(timebuf,sizeof(timebuf),"%F %T",&tmval);
  fprintf(logfile,"%s: %lu:",timebuf,n);
  fprintf(logfile," %d",global_maxsteps);
  if (global_maxsteps_found){
    fprintf(logfile,"*");
    predict_double = (n%6 == 3);
    predict_four = (n%6 == 1);
    global_maxsteps_found = 0;
  }
  for (i=0;i<global_maxvalue_size;i++){
    fprintf(logfile," %u",global_maxvalue[global_maxvalue_size-i-1]);
  }
  if (global_maxvalue_found){
    fprintf(logfile,"*");
    global_maxvalue_found = 0;
  }
  fprintf(logfile,"\n");
  if (block == 0){
    clz_value = __builtin_clz(n);
    if (clz32[clz_value] == INT32_MAX/2){
      fprintf(logfile,"%s: %lu new clz found - clz32[%d] = %d\n",
	       timebuf,n,clz_value,global_maxsteps);
    } else if (clz32[clz_value] < global_maxsteps){
      fprintf(logfile,"%s: %lu new clz found ! clz32[%d] = %d\n",
	       timebuf,n,clz_value,global_maxsteps);
    }
  }
  clz_value = __lzcnt64(n);
  if (clz64[clz_value] == INT32_MAX/2){
    fprintf(logfile,"%s: %lu new clz found - clz64[%d] = %d\n",
	     timebuf,n,clz_value,global_maxsteps);
  }
  if (predict_double){
    predicted = n*2;
    fprintf(logfile,"%s:\t%lu: %d predicted (%ld)\n",timebuf,predicted,global_maxsteps+1,predicted >> 32);
    if ((predicted>>32) != block){
      add_predict_block(predicted>>32);
    }
  } else if (predict_four){
    predicted = (n*4-1)/3;
    fprintf(logfile,"%s:\t%lu: %d predicted (%ld)\n",timebuf,predicted,global_maxsteps+3,predicted>>32);
    if ((predicted>>32) != block){
      add_predict_block(predicted>>32);
    }
  }
  fflush(logfile);
}

void recover_checkpoint_file(void){
  unsigned long int block;
  FILE *checkfile;
  int i,status;
  char predictstring[1024];
  if (access(checkpoint_file,F_OK) == -1){
    fprintf(stderr,"note: no checkpoint file found: %s\n",checkpoint_file);
    return;
  }
  if ((checkfile = fopen(checkpoint_file,"r")) == NULL){
    fprintf(stderr,"unable to open checkpoint file: %s\n",checkpoint_file);
    exit(0);
  }
  // read in the block
  if (fscanf(checkfile,"Block: %lu\n",&block) != 1){
    fprintf(stderr,"unable to read checkpoint block\n");
    exit(0);
  } else {
    if (bflag == 0) start_block = block;
  }
  status = fscanf(checkfile,"Maxsteps: %d\n",&global_maxsteps);
  status = fscanf(checkfile,"Maxvalue: %d:",&global_maxvalue_size);
  global_maxvalue128 = 0;
  for (i=0;i<global_maxvalue_size;i++){
    status = fscanf(checkfile,"%u",&global_maxvalue[i]);
    if (i!=0) global_maxvalue128 <<= 32;
    global_maxvalue128 |= global_maxvalue[i];
  }
  half_global_maxvalue128 = global_maxvalue128 >> 1;
  status = fscanf(checkfile,"%s",predictstring);
  while (fscanf(checkfile,"%lu",&block) == 1){
    add_predict_block(block);
  }
}

void save_checkpoint_file(uint64_t blocknum){
  FILE *checkfile;
  char *backup;
  int i;
  struct predict *p;
  if (access(checkpoint_file, F_OK) != -1){
    backup = malloc(strlen(checkpoint_file)+6);
    strcpy(backup,checkpoint_file);
    strcat(backup,".back");
    if (rename(checkpoint_file,backup) != 0){
      fprintf(stderr,"checkpoint not saved: unable to rename %s to %s\n",
	      checkpoint_file,backup);
      return;
    }
  }
  fprintf(logfile,"Saving checkpoint at block %lu\n",blocknum);
  if ((checkfile = fopen(checkpoint_file,"w")) == NULL){
    fprintf(stderr,"checkpoint not saved: unable to open file %s\n",
	    checkpoint_file);
    return;
  }
  fprintf(checkfile,"Block: %lu\n",blocknum);
  fprintf(checkfile,"Maxsteps: %d\n",global_maxsteps);
  fprintf(checkfile,"Maxvalue: %d:",global_maxvalue_size);
  for (i=0;i<global_maxvalue_size;i++){
    fprintf(checkfile," %u",global_maxvalue[i]);
  }
  fprintf(checkfile,"\n");
  fprintf(checkfile,"Predict:");
  for (p = predicted_blocks;p;p = p->next){
    fprintf(checkfile," %lu",p->blocknum);
  }
  fprintf(checkfile,"\n");
  fclose(checkfile);
}

void search_block0(void){
  unsigned long int start,stop,i;
  uint32_t n[MAXDIGITS] = { 0 };
  int32_t steps;

  for (i=0;i<(1l<<CUTOFF_WIDTH);i++){
    n[0] = i;
    steps = 0;
    hail32m(n,&steps,&global_maxsteps,global_maxvalue,global_maxvalue_size);
    if (global_maxvalue_found || global_maxsteps_found){
      record_peak(i,0);
    }
  }
  start = (1l << CUTOFF_WIDTH);
  stop = (1l<<32);
  for (i=start;i<stop;i++){
    // duplicate polynomial
    if (cutoff16[i&0xffff] & 0x80) continue;
    // 5 mod 6 && 2,4,8 mod 9
    switch(i%18){
    case 2: // 2 mod 9
    case 4: // 4 mod 9
    case 5: // 5 mod 6
    case 8: // 8 mod 9
    case 11: // 2 mod 9, 5 mod 6
    case 13: // 4 mod 9
    case 17: // 8 mod 9, 5 mod 6
      continue;
    }
    n[0] = i;
    steps = 0;
    if (cutoff16[i&0xffff] & 0x40){
      hail32pn(n,&steps,&global_maxsteps,global_maxvalue,global_maxvalue_size);
    } else {
      hail32pm(n,&steps,&global_maxsteps,global_maxvalue,global_maxvalue_size);
    }
    if (global_maxvalue_found || global_maxsteps_found){
      record_peak(i,0);
    }
  }
  return;
}

void search_block(uint64_t blocknum){
  unsigned long int start,stop,i;
  uint32_t n[MAXDIGITS] = { 0 };
  int32_t steps;
  if (blocknum == 0){
    search_block0();
  } else {
    for (i=0;i<(1l<<32);i++){
      // duplicate polynomial
      if (cutoff16[i&0xffff] & 0x80) continue;
      // 5 mod 6 && 2,4,8 mod 9
      switch(((blocknum<<32)+i)%18){
      case 2: // 2 mod 9
      case 4: // 4 mod 9
      case 5: // 5 mod 6
      case 8: // 8 mod 9
      case 11: // 2 mod 9, 5 mod 6
      case 13: // 4 mod 9
      case 17: // 8 mod 9, 5 mod 6
	 continue;
      }
      n[0] = i;
      n[1] = blocknum;
      steps = 0;
      if (cutoff16[i&0xffff] & 0x40){
	 hail64pn(n,&steps,&global_maxsteps,global_maxvalue,global_maxvalue_size);
	 hail32pn(n,&steps,&global_maxsteps,global_maxvalue,global_maxvalue_size);
      } else {
	 hail64pm(n,&steps,&global_maxsteps,global_maxvalue,global_maxvalue_size);
	 hail32pn(n,&steps,&global_maxsteps,global_maxvalue,global_maxvalue_size);
      }
      if (global_maxvalue_found || global_maxsteps_found){
	 record_peak((blocknum<<32)+i,blocknum);
      }
    }
  }
  return;
}

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


// compute a slow hailstone and update the peaks
unsigned __int128 half_global_maxvalue128 = 0;
unsigned __int128 global_maxvalue128 = 0;
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

