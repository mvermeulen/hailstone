#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if !defined WIDTH
#define WIDTH 8
#endif
#define POLYTABLE_SIZE 1000

int tflag = 0;
int width = WIDTH;
int cflag = 0;
int fflag = 0;
int mflag = 0;
int vflag = 0;

typedef struct poly {
  int pow2;
  int pow3;
  unsigned long int add;
  int max;
} poly_t;
unsigned int powers_of_2[32];
unsigned int powers_of_3[32];

/* simplified hailstone to calculate termination */
int hailsteps(long int n){
  int steps = 0;
  while (n > 1){
    if (n & 0x1){
      n = n * 3 + 1;
      steps++;
    }
    n = n >> 1;
    steps++;
  }
  return steps;
}

void print_poly(poly_t *p){
  if (p->pow3 != 0){
    printf("%u",powers_of_3[p->pow3]);
  }
  if (p->pow2 == 0){
    printf("*X");
  } else {
    printf("[X/%u]",powers_of_2[p->pow2]);
  }
  if (p->add != 0){
    printf(" + %ld",p->add);
  }
}

void compute_poly(int bits,int width,poly_t *final,poly_t *max,int verbose){
  int power3 = 0;
  int power2 = 0;
  int steps = 0;
  poly_t partial = { 0 };
  int maxfound;
  double maxratio = 1.0;
  max->pow2 = 0;
  max->pow3 = 0;
  max->add = 0;
  max->max = 1;
  if (vflag) printf("poly(%d,%d)\n",bits,width);
  while (width > 0){
    if (((double) powers_of_3[power3]/powers_of_2[power2]) < 1.0) max->max = 0;
    maxfound = 0;
    if (bits & 0x1){
      bits = bits * 3 + 1;
      power3++;
      if (powers_of_3[power3] > powers_of_2[power2]){
	if (((double) powers_of_3[power3]/powers_of_2[power2]) > maxratio){
	  maxfound = 1;
	  max->pow3 = power3;
	  max->pow2 = power2;
	  max->add = bits;
	  maxratio = ((double) powers_of_3[power3]/powers_of_2[power2]);
	}
      }
    } else {
      bits = bits >> 1;
      width--;
      power2++;
    }
    steps++;
    partial.pow2 = power2;
    partial.pow3 = power3;
    partial.add = bits;
    if (vflag){
      printf("\t");
      print_poly(&partial);
      if (maxfound){ printf(" <max>"); }
      printf("(%d)\n",max->max);
    }
  }
  if ((double) powers_of_3[power3]/(double) powers_of_2[power2] < 1.0)
    max->max = 0;
  final->pow2 = power2;
  final->pow3 = power3;
  final->add = bits;
}

struct polyinfo {
  unsigned int mod6_1 : 1;
  unsigned int mod6_3 : 1;
  unsigned int mod6_5 : 1;
  poly_t p;
} *polytable = NULL;
int npoly = 0;
int npoly_allocated = 0;
int lookup_poly(poly_t *p,int insert){
  int i;
  if (npoly_allocated == 0){
    npoly_allocated = POLYTABLE_SIZE;
    polytable = malloc(npoly_allocated * sizeof(struct polyinfo));
  } else if (npoly >= npoly_allocated){
    npoly_allocated *= 1.5;
    polytable = realloc(polytable,npoly_allocated*sizeof(struct polyinfo));
  }
  for (i=0;i<npoly;i++){
    if ((p->pow2 == polytable[i].p.pow2) &&
	(p->pow3 == polytable[i].p.pow3) &&
	(p->add == polytable[i].p.add)){
      return i;
    }
  }
  // not found
  if (insert == 0) return -1;
  polytable[npoly].p.pow2 = p->pow2;
  polytable[npoly].p.pow3 = p->pow3;
  polytable[npoly].p.add = p->add;
  polytable[npoly].p.max = p->max;
  polytable[npoly].mod6_1 = 0;
  polytable[npoly].mod6_3 = 0;
  polytable[npoly].mod6_5 = 0;
  return npoly++;
}

int main(int argc,char **argv){
  int i,idx;
  int opt;
  unsigned long pow3 = 1;
  unsigned long pow2 = 1;
  unsigned int mask;
  poly_t final_poly,max_poly,*fpoly,*mpoly;
  int *final_idx = NULL,*max_idx = NULL,*dup_idx = NULL;
  while ((opt = getopt(argc,argv,"+cfmtvw:?")) != -1){
    switch(opt){
    case 'c':
      cflag = 1;
      break;
    case 'f':
      fflag = 1;
      break;
    case 'm':
      mflag = 1;
      break;
    case 't':
      tflag = 1;
      break;
    case 'v':
      vflag = 1;
      break;
    case 'w':
      sscanf(optarg,"%d",&width);
      break;
    case '?':
    default:
      fprintf(stderr,"%s: %s [-t][-w width]\n",argv[0],argv[0]);
      exit(0);
    }
  }
  for (i=0;i<32;i++){
    powers_of_2[i] = pow2;
    powers_of_3[i] = pow3;
    pow3 *= 3;
    pow2 *= 2;
  }

  if (tflag){
    printf("int steps%d[] = {\n",width);
    for (i=0;i<(1<<width);i++){
      printf("%d, /* %d */\n",hailsteps(i),i);
    }
    printf("};\n");
    return 0;
  }
  if (width <= 16){
    final_idx = alloca(sizeof(int)*(1<<width));
    max_idx = alloca(sizeof(int)*(1<<width));
    dup_idx = alloca(sizeof(int)*(1<<width));
  } else {
    final_idx = malloc(sizeof(int)*(1<<width));
    max_idx = malloc(sizeof(int)*(1<<width));
    dup_idx = malloc(sizeof(int)*(1<<width));    
  }
  // compute polynomials
  if (vflag){ printf("/* polynomial table\n"); }
  for (i=0;i<(1<<width);i++){
    compute_poly(i,width,&final_poly,&max_poly,vflag);
    // figure out if it is a duplicate
    idx = lookup_poly(&final_poly,0);
    if (idx == -1){
      dup_idx[i] = 0;
      idx = lookup_poly(&final_poly,1);
    } else {
      dup_idx[i] = 1;
    }
    final_idx[i] = idx;
    // save the mod information
    switch(i%6){
    case 1: polytable[idx].mod6_1 = 1; break;
    case 3: polytable[idx].mod6_3 = 1; break;
    case 5: polytable[idx].mod6_5 = 1; break;
    }
    // record the maximum information
    idx = lookup_poly(&max_poly,1);
    max_idx[i] = idx;
  }
  if (vflag){ printf("*/\n"); }
  if (cflag){ printf("unsigned char cutoff%d[] = {\n",width); }
  else if (fflag){
    printf("struct poly { unsigned int mul3; unsigned short div2; unsigned short steps; unsigned int add; unsigned char smaller; } fpoly%d[] = {\n",width);
  } else if (mflag){
    printf("struct poly { unsigned int mul3; unsigned short div2; unsigned short steps; unsigned int add; unsigned char smaller; } mpoly%d[] = {\n",width);
  }
  // print the appropriate polynomial information
  for (i=0;i<(1<<width);i++){
    fpoly = &polytable[final_idx[i]].p;
    mpoly = &polytable[final_idx[i]].p;
    if (cflag){
      mask = 0;
      if (dup_idx[i]) mask |= 0x80;
      if (!polytable[max_idx[i]].p.max) mask |= 0x40;
      if (polytable[final_idx[i]].mod6_1) mask |= 0x4;
      if (polytable[final_idx[i]].mod6_3) mask |= 0x2;
      if (polytable[final_idx[i]].mod6_5) mask |= 0x1;
      printf("%#02x,\t/* %d%s%s",mask,i,(dup_idx[i]?" dup":""),(polytable[max_idx[i]].p.max?"":" nomax"));
      printf(" ");
      print_poly(&polytable[final_idx[i]].p);
      printf(" */\n");
    } else if (fflag){
      printf("{ %u, %hu, %hu, %lu, %d }, /* %d: ",
	     powers_of_3[polytable[final_idx[i]].p.pow3],
	     polytable[final_idx[i]].p.pow2,
	     polytable[final_idx[i]].p.pow3 + polytable[final_idx[i]].p.pow2,
	     polytable[final_idx[i]].p.add,
	     powers_of_2[polytable[final_idx[i]].p.pow2] > 
	     powers_of_3[polytable[final_idx[i]].p.pow3],
	     i);
      print_poly(&polytable[final_idx[i]].p);
      printf(" */\n");
    } else if (mflag){
      if (polytable[max_idx[i]].p.pow2 == 0){
	printf("{ %u, %hu, %hu, %lu, %d }, /* %d: ",
	       powers_of_3[polytable[final_idx[i]].p.pow3],
	       polytable[final_idx[i]].p.pow2,
	       polytable[final_idx[i]].p.pow3 + polytable[final_idx[i]].p.pow2,
	       polytable[final_idx[i]].p.add,
	       powers_of_2[polytable[final_idx[i]].p.pow2] > 
	       powers_of_3[polytable[final_idx[i]].p.pow3],
	       i);
	print_poly(&polytable[final_idx[i]].p);
      } else {
	printf("{ %u, %hu, %hu, %lu, %d }, /* %d: ",
	       powers_of_3[polytable[max_idx[i]].p.pow3],
	       polytable[max_idx[i]].p.pow2,
	       polytable[max_idx[i]].p.pow3 + polytable[max_idx[i]].p.pow2,
	       polytable[max_idx[i]].p.add,
	       powers_of_2[polytable[max_idx[i]].p.pow2] > 
	       powers_of_3[polytable[max_idx[i]].p.pow3],
	       i);
	print_poly(&polytable[max_idx[i]].p);
      }
      printf(" */\n");
    }
  }
  if (cflag||fflag|mflag){ printf("};\n"); }
  return 0;
}
