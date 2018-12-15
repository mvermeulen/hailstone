#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <x86intrin.h>
#include "hailstone.h"

int bflag = 0;
uint32_t start_block = 0;
int cflag = 0;
#define CHECKPOINT_NAME "checkpt.txt"
char *checkpoint_file = CHECKPOINT_NAME;
int lflag = 0;
FILE *logfile = NULL;
int nflag = 0;
uint32_t num_blocks = 1;

char *usage[] = {
  "hailmgr [-b <n>][-c <file>][-l <f>][-n <n>]",
  "\t-b n, start at block n, where each block is 2^32 numbers",
  "\t-c file, set name of checkpoint file (default checkpt.txt)",
  "\t-l file, use file l as log file",
  "\t-n n, search n blocks",
};

/* parse_options - read command line options and set flags */
int parse_options(int argc,char **argv){
  int opt;
  int i;
  while ((opt = getopt(argc,argv,"b:c:l:n:?")) != -1){
    switch(opt){
    case 'b':
      bflag = 1;
      sscanf(optarg,"%u",&start_block);
      break;
    case 'c':
      cflag = 1;
      checkpoint_file = strdup(optarg);
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
  if (lflag == 0) logfile = stdout;
  return optind;
}

#define NUM_HOSTS 10
int nhosts = 0;
struct hostinfo {
  char *name;
  uint64_t chunksize;
  struct sockaddr_in address;
} hosttable[NUM_HOSTS];

void read_host_info(char *filename){
  FILE *fp;
  char buffer[1024];
  char hostname[128];
  uint64_t chunksize;
  struct hostent *host;
  struct servent *service;
  if ((fp = fopen(filename,"r")) == NULL){
    fprintf(stderr,"can't open host table: %s\n",filename);
    exit(0);
  }
  if ((service = getservbyname("haild","udp")) == NULL){
    fprintf(stderr,"can't find haild/udp in /etc/services\n");
    exit(0);
  }
  while (fgets(buffer,sizeof(buffer),fp) != NULL){
    if (buffer[0] == '#') continue;
    if (sscanf(buffer,"%s %lu",hostname,&chunksize) != 2){
      fprintf(stderr,"unknown line in %s: %s\n",filename,buffer);
      continue;
    }
    if ((host = gethostbyname(hostname)) == NULL){
      fprintf(stderr,"can't find host %s\n",hostname);
      continue;
    }
    if (nhosts >= NUM_HOSTS){
      fprintf(stderr,"too many hosts, increase NUM_HOSTS definition\n");
      continue;
    }
    hosttable[nhosts].name = strdup(hostname);
    hosttable[nhosts].chunksize = chunksize;
    memset(&hosttable[nhosts].address,0,sizeof(struct sockaddr_in));
    hosttable[nhosts].address.sin_family = AF_INET;
    memcpy(&hosttable[nhosts].address.sin_addr.s_addr,
	   host->h_addr,
	   host->h_length);
    hosttable[nhosts].address.sin_port = htons(service->s_port);
    nhosts++;
  }
}

int main(int argc,char **argv){
  int sockfd,n;
  unsigned long int i,j;
  time_t start_time,start_time2,now;
  struct tm tmval;
  char timebuf[120];
  int search_parallel;
  struct sockaddr_in my_addr;
  struct servent *service;
  struct hail_packet packet;
  parse_options(argc,argv);
  recover_checkpoint_file();
  sockfd = socket(AF_INET,SOCK_DGRAM,0);
  memset(&my_addr,0,sizeof(my_addr));
  my_addr.sin_family = AF_INET;
  my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  if ((service = getservbyname("haild","udp")) == NULL){
    fprintf(stderr,"can't find haild/udp in /etc/services\n");
    exit(0);
  }
  my_addr.sin_port = htons(service->s_port+1);
  bind(sockfd,(struct sockaddr *) &my_addr, sizeof(my_addr));
  read_host_info("hosts.txt");
  for (i=0;i<num_blocks;){
    time(&start_time);
    localtime_r(&start_time,&tmval);
    strftime(timebuf,sizeof(timebuf),"%F %T",&tmval);
    // see if we can do a fast search
    search_parallel = 1;
    for (j=0;j<hosttable[0].chunksize;j++){
      if (check_prediction(start_block+i+j) != 0){
	search_parallel = 0;
	break;
      }
    }
    if (search_parallel == 0){
      // predicted, so search only the next block
      fprintf(logfile,"%s: Search of block: %lu\n",timebuf,start_block+i);
      search_block(start_block+i);
      remove_prediction(start_block+i);
      time(&now);
      localtime_r(&now,&tmval);
      strftime(timebuf,sizeof(timebuf),"%F %T",&tmval);
      fprintf(logfile,"%s: Search complete, %ld seconds\n",
	      timebuf,now-start_time);
      i++;
    } else {
      // no predictions, so try search in parallel
      fprintf(logfile,"%s: Fast parallel search of blocks: %lu-%lu on %s\n",
	      timebuf,
	      start_block+i,start_block+i+hosttable[0].chunksize-1,
	      hosttable[0].name);
      packet.message = HAIL_FASTSEARCH;
      packet.start_block = start_block + i;
      packet.num_blocks = hosttable[0].chunksize;
      packet.maxsteps = global_maxsteps;
      packet.maxvalue_size = global_maxvalue_size;
      for (j=0;j<global_maxvalue_size;j++){
	packet.maxvalue[j] = global_maxvalue[j];
      }
      sendto(sockfd,&packet,sizeof(packet),0,
	     (struct sockaddr *) &hosttable[0].address,
	     sizeof(hosttable[0].address));
      // now wait for the search response
      n = recvfrom(sockfd,&packet,sizeof(packet),0,NULL,NULL);
      time(&now);
      localtime_r(&now,&tmval);
      strftime(timebuf,sizeof(timebuf),"%F %T",&tmval);
      switch(packet.message){
      case HAIL_PEAKFOUND:
	fprintf(logfile,"%s: Found a peak, %ld seconds\n",
		timebuf,now-start_time);
	start_time = now;
	search_block(start_block+i);
	time(&now);
	localtime_r(&now,&tmval);
	strftime(timebuf,sizeof(timebuf),"%F %T",&tmval);
	fprintf(logfile,"%s: Search complete, %ld seconds\n",
		timebuf,now-start_time);
	i++;
	break;
      case HAIL_NOPEAK:
	fprintf(logfile,"%s: Search complete, %ld seconds\n",
		timebuf,now-start_time);
	i = i + hosttable[0].chunksize;
	break;
      default:
      case HAIL_ERROR:
	fprintf(logfile,"%s: Unknown hail packet type: %d\n",
		timebuf,packet.message);
	exit(0);
      }
    }
  }
  save_checkpoint_file(start_block+i);
  return 0;
}
