#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <x86intrin.h>
#include "hailstone.h"

extern struct poly fpoly16[];

// flags
int bflag = 0;
uint32_t start_block = 0;
int cflag = 0;
#define CHECKPOINT_NAME "checkpoint.txt";
char *checkpoint_file = CHECKPOINT_NAME;
int dflag = 0;
int fflag = 0;
int f2flag = 0;
int lflag = 0;
FILE *logfile = NULL;
int nflag = 0;
uint32_t num_blocks = 1;
int wflag = 0;
uint32_t wvalue = 1;

// usage
char *usage[] = {
  "hailstone [-b <n>][-c <file>][-f][-l <f>][-n <n>][-w <n>]",
  "\t-b n, start at block n, where each block is 2^32 numbers",
  "\t-c file, set name of checkpoint file (default checkpoint.txt)",
  "\t-d, start in daemon mode",
  "\t-n n, search n blocks,  where each block is 2^32 numbers",
  "\t-f  , fast search - look for peak only and backtrack if found",
  "\t-l f, use file l as a log file",
  "\t-w n, search n blocks in parallel",
};
/* parse_options - read command line options and set flags */
int parse_options(int argc,char **argv){
  int opt;
  int i;
  while ((opt = getopt(argc,argv,"b:c:dfFl:n:w:?")) != -1){
    switch(opt){
    case 'b':
      bflag = 1;
      sscanf(optarg,"%u",&start_block);
      break;
    case 'c':
      cflag = 1;
      checkpoint_file = strdup(optarg);
      break;
    case 'd':
      dflag = 1;
      break;
    case 'f':
      fflag = 1;
      break;
    case 'F':
      f2flag = 1;
      break;      
    case 'l':
      lflag = 1;
      if (!strcmp(optarg,"-")) logfile = stdout;
      else if ((logfile = fopen(optarg,"a+")) == 0){
	 fprintf(stderr,"unable to open logfile: %s\n",optarg);
	 goto usage;
      }
      break;
    case 'n':
      nflag = 1;
      sscanf(optarg,"%u",&num_blocks);
      break;
    case 'w':
      wflag = 1;
      sscanf(optarg,"%u",&wvalue);
      break;
    case '?':
    default:
    usage:
      fprintf(stderr,"usage: ");
      for (i=0;i<(sizeof(usage)/sizeof(usage[0]));i++){
	 fputs(usage[i],stderr);
	 fputc('\n',stderr);
      }
      exit(0);
      break;
    }
  }
  if (bflag == 1 && fflag == 1){
    fprintf(stderr,"The -f flag can not be given when start block is 0\n");
    goto usage;
  }
  if (fflag == 1 && f2flag == 1){
    fprintf(stderr,"Both -f and -F given\n");
    goto usage;
  }
  if (lflag == 0) logfile = stdout;
  return optind;
}

// conduct a fast search - return 1 if peaks are found, 0 otherwise
int fast_search(uint64_t blocknum){
  unsigned int i,j;
  uint32_t n[MAXDIGITS] = { 0 };
  int32_t steps;
  int32_t peak_found;
  int clz_value;
  struct poly *fp;
  unsigned long int num;
  for (i=0;i<(1<<CUTOFF_WIDTH);i++){
    if (cutoff16[i] & 0x80) continue; // duplicate polynomial
    if (cutoff16[i] & 0x40){
      // no max in values
      // see if we can cut off all the polynomials at once
      fp = &fpoly16[i&0xffff];
      num = (blocknum<<32)|((1l<<32)-1);
      num = num >> fp->div2;
      num = num * fp->mul3;
      num = num + fp->add;
      clz_value = __lzcnt64(num);
      if (clz64[clz_value] + fp->steps < global_maxsteps) continue;
      // no cutoff so calculate each entry
      for (j=0;j<(1<<(32-CUTOFF_WIDTH));j++){
	 num = (blocknum<<32)|(j<<CUTOFF_WIDTH)|i;
	 // ignore evens + 5 mod 6 + 2,4,8 mod 9
	 switch(num%18){
	 case 0: // even
	 case 2: // even, 2 mod 9
	 case 4: // even, 4 mod 9
	 case 5: // 5 mod 6
	 case 6: // even
	 case 8: // even, 8 mod 9
	 case 10: // even
	 case 11: // 2 mod 9, 5 mod 6
	 case 12: // even
	 case 13: // 4 mod 9
	 case 14: // even
	 case 16: // even
	 case 17: // 8 mod 9, 5 mod 6
	   continue;
	 }
	 num = num >> fp->div2;
	 num = num * fp->mul3 + fp->add;
	 steps = fp->steps;	

	 peak_found = 0;
#if EXPERIMENTAL
	 hail64an(num,steps,global_maxsteps,global_maxvalue_size,global_maxvalue,&peak_found);
#else
	 n[0] = num & 0xffffffff;
	 n[1] = num >> 32;
	 hail64pnf(n,&steps,&global_maxsteps,global_maxvalue,global_maxvalue_size,&peak_found);
	 hail32pnf(n,&steps,&global_maxsteps,global_maxvalue,global_maxvalue_size,&peak_found);
#endif
	 if (peak_found){
	   // oops we found one, so exit for more complete search
	   if (!dflag)
	     fprintf(logfile,"peak found for %lu\n",(blocknum<<32)|(j<<CUTOFF_WIDTH)|i);
	   return 1;
	 }
      }
    } else {
      // possible max in values
      for (j=0;j<(1<<(32-CUTOFF_WIDTH));j++){
	 num = (blocknum<<32)|(j<<CUTOFF_WIDTH)|i;
	 // ignore evens + 5 mod 6 + 2,4,8 mod 9
	 switch(num%18){
	 case 0: // even
	 case 2: // even, 2 mod 9
	 case 4: // even, 4 mod 9
	 case 5: // 5 mod 6
	 case 6: // even
	 case 8: // even, 8 mod 9
	 case 10: // even
	 case 11: // 2 mod 9, 5 mod 6
	 case 12: // even
	 case 13: // 4 mod 9
	 case 14: // even
	 case 16: // even
	 case 17: // 8 mod 9, 5 mod 6
	   continue;
	 }
	 steps = 0;
	 peak_found = 0;
#if EXPERIMENTAL
	 hail64am(num,steps,global_maxsteps,global_maxvalue_size,global_maxvalue,&peak_found);
#else
	 n[0] = (j<<CUTOFF_WIDTH)|i;
	 n[1] = blocknum;
	 hail64pmf(n,&steps,&global_maxsteps,global_maxvalue,global_maxvalue_size,&peak_found);
	 hail32pnf(n,&steps,&global_maxsteps,global_maxvalue,global_maxvalue_size,&peak_found);
#endif
	 if (peak_found){
	   if (!dflag)
	     fprintf(logfile,"peak found for %lu\n",(blocknum<<32)|(j<<CUTOFF_WIDTH)|i);
	   // oops we found one, so exit for more complete search
	   return 1;
	 }
      }
    }
  }
  return 0;
}

// conduct a fast search using the power of 3 algorithm - return 1 if peaks are found, 0 otherwise
int fast_search2(uint64_t blocknum){
  unsigned int i,j;
  uint64_t num;
  int no_max;
  int32_t peak_found;
  int32_t steps;
  for (i=0;i<(1<<CUTOFF_WIDTH);i++){
    if (cutoff16[i] & 0x80) continue; // duplicate polynomial
    if (cutoff16[i] & 0x40){
      no_max = 1;
    } else {
      no_max = 0;
    }
    for (j=0;j<(1<<(32-CUTOFF_WIDTH));j++){
      num = (blocknum << 32)|(j<<CUTOFF_WIDTH)|i;
      // ignore evens + 5 mod 6 + 2,4,8 mod 9
      switch(num%18){
      case 0: // even
      case 2: // even, 2 mod 9
      case 4: // even, 4 mod 9
      case 5: // 5 mod 6
      case 6: // even
      case 8: // even, 8 mod 9
      case 10: // even
      case 11: // 2 mod 9, 5 mod 6
      case 12: // even
      case 13: // 4 mod 9
      case 14: // even
      case 16: // even
      case 17: // 8 mod 9, 5 mod 6
	continue;
      }
      peak_found = 0;
      steps = 0;
      if (no_max){
	hail64yn(num,steps,global_maxsteps,&peak_found);
      } else {
	hail64ym(num,steps,global_maxsteps,half_global_maxvalue128,&peak_found);
      }
      if (peak_found){
	if (!dflag)
	  fprintf(logfile,"peak found for %lu\n",(blocknum<<32)|(j<<CUTOFF_WIDTH)|i);
	// oops we found one, so exit for more complete search
	return 1;
      }	
    }
  }
  return 0;
}
  
int main(int argc,char **argv){
  uint64_t i,j;
  time_t start_time,start_time2,now;
  struct tm tmval;
  char timebuf[120];
  int search_parallel,found_peak;
  int sockfd,n,len;
  struct servent *service;
  struct sockaddr_in server_addr,client_addr;
  struct hail_packet packet;
  parse_options(argc,argv);
  if (dflag){
    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    if ((service = getservbyname("haild","udp")) == NULL){
      fprintf(stderr,"can't find haild/udp in /etc/services\n");
      exit(0);
    }
    server_addr.sin_port = htons(service->s_port);
    bind(sockfd,(struct sockaddr *) &server_addr,sizeof(server_addr));
    while (1){
      len = sizeof(client_addr);
      n = recvfrom(sockfd,&packet,sizeof(packet),0,
		   (struct sockaddr *) &client_addr,&len);
      if (packet.message == HAIL_FASTSEARCH){
	start_block = packet.start_block;
	num_blocks = packet.num_blocks;
	global_maxsteps = packet.maxsteps;
	global_maxvalue_size = packet.maxvalue_size;
	for (j=0;j<packet.maxvalue_size;j++){
	  global_maxvalue[j] = packet.maxvalue[j];
	}
	found_peak = 0;
#pragma omp parallel for shared(found_peak)
	for (j=0;j<num_blocks;j++){
	  if ((f2flag?fast_search2:fast_search)(start_block+j))
	    found_peak = 1;
	}
	packet.message = (found_peak)? HAIL_PEAKFOUND : HAIL_NOPEAK;
      } else {
	packet.message = HAIL_ERROR;
      }
      client_addr.sin_port = htons(service->s_port+1);
      // send back a response
      sendto(sockfd,&packet,
	     sizeof(packet),0,
	     (struct sockaddr *) &client_addr,sizeof(client_addr));
    }
  } else {
    recover_checkpoint_file();
  }
  for (i=0;i<num_blocks;){
    time(&start_time);
    localtime_r(&start_time,&tmval);
    strftime(timebuf,sizeof(timebuf),"%F %T",&tmval);
    if (wflag && wvalue > 1){
      // try fast search in parallel...
      search_parallel = 1;
      for (j=0;j<wvalue;j++){
	if (check_prediction(start_block+i+j) != 0){
	  search_parallel = 0;
	  break;
	}
      }
      if (search_parallel == 0){
	// predictions, so search next block only
	fprintf(logfile,"%s: Search of block: %lu\n",timebuf,start_block+i);
	search_block(start_block+i);
	remove_prediction(start_block+i);
	i++;	
      } else {
	// no predictions, so try search in parallel
	fprintf(logfile,"%s: Fast parallel search of blocks: %lu-%lu\n",timebuf,start_block+i,start_block+i+wvalue-1);	
	found_peak = 0;
#pragma omp parallel for shared(found_peak)
	for (j=0;j<wvalue;j++){
	  if ((f2flag?fast_search2:fast_search)(start_block+i+j)){
	    found_peak = 1;
	  }
	}
	if (found_peak == 1){
	  // failure, one of the parallel searches found a peak
	  time(&start_time2);
	  localtime_r(&start_time2,&tmval);
	  strftime(timebuf,sizeof(timebuf),"%F %T",&tmval);
	  fprintf(logfile,"%s: Peak found, parallel search of blocks: %lu-%lu, %ld seconds\n",
		  timebuf,start_block+i,start_block+i+wvalue-1,start_time2-start_time);	  
	  start_time = start_time2;
	  search_block(start_block+i);
	  i++;
	} else {
	  // success! no peaks found in parallel search
	  i += wvalue;
	}
      }
    } else if (check_prediction(start_block+i) == 0){
      // no parallel search but try a fast search first
      fprintf(logfile,"%s: Fast search of block: %lu\n",timebuf,start_block+i);
      if ((f2flag?fast_search2:fast_search)(start_block+i)){
	time(&start_time2);
	localtime_r(&start_time2,&tmval);
	strftime(timebuf,sizeof(timebuf),"%F %T",&tmval);
	fprintf(logfile,"%s: Peak found, search of block: %lu, %ld seconds\n",timebuf,start_block+i,start_time2-start_time);
	start_time = start_time2;
	search_block(start_block+i);
      }
      i++;
    } else {
      // prediction, so start with slow search
      fprintf(logfile,"%s: Search of block: %lu\n",timebuf,start_block+i);
      search_block(start_block+i);
      remove_prediction(start_block+i);
      i++;
    }
    time(&now);
    localtime_r(&now,&tmval);
    strftime(timebuf,sizeof(timebuf),"%F %T",&tmval);
    fprintf(logfile,"%s: Search complete, %ld seconds\n",timebuf,now-start_time);
  }
  save_checkpoint_file(start_block+i);
  return 0;
}
