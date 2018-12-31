// genpoly.c - generate polynomial table values
#include <iostream>
#include <string>
#include <unistd.h>
#include "config.hpp"
int width = POLY_WIDTH;
int pflag = 0;

int parse_options(int argc,char *const argv[]){
  int opt;
  while ((opt = getopt(argc,argv,"pw:")) != -1){
    switch(opt){
    case 'p':
      pflag = 1;
      break;
    case 'w':
      width = std::stoi(optarg);
      break;
    default:
      std::cerr << "warning: unknown option: " << opt << std::endl;
      return 1;
      break;
    }
  }
  return 0;
}

typedef struct poly {
  int pow2;
  int pow3;
  unsigned long int add;
  int max;
} poly_t;

unsigned long powers_of_2[32];
unsigned long powers_of_3[32];

void compute_poly(unsigned int bits,int width,poly_t *final,poly_t *max){
  int power3 = 0;
  int power2 = 0;
  int steps = 0;
  poly_t partial = { 0 };
  int maxfound = 0;
  double maxratio = 1.0;
  max->pow2 = 0;
  max->pow3 = 0;
  max->add = 0;
  max->max = 0;
  std::cout << "poly(" << bits << "," << width << ")" << std::endl;
  while (width > 0){
    if (bits & 0x1){
      bits = bits * 3 + 1;
      power3++;
      if (powers_of_3[power3] > powers_of_2[power2]){
	if (((double) powers_of_3[power3]/powers_of_2[power2]) > maxratio){
	  maxfound = 1;
	  max->pow3 = power3;
	  max->pow2 = power2;
	  max->add = bits;
	  max->max = 1;
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
  }
  if ((double) powers_of_3[power3]/(double) powers_of_2[power2] > 1.0){
    final->max = 1;
  } else {
    final->max = 0;
  }
  final->pow2 = power2;
  final->pow3 = power3;
  final->add = bits;
  if (powers_of_3 > 0){
    std::cout << "\t" << powers_of_3[power3] << "(X/" << powers_of_2[power2] << ") + "
	      << bits << ((final->max)?" greater ":"") << std::endl;
  } else {
    std::cout << "\t" << "X/" << powers_of_2[power2] << " + "
	      << bits << ((final->max)?" greater ":"") << std::endl;      
  }
  if (maxfound){
    if (max->pow3 > 0){
      std::cout << "\t" << powers_of_3[max->pow3] << "(X/" << powers_of_2[max->pow2] << ") + "
		<< bits << " max" << std::endl;
    } else {
      std::cout << "\t" << "X/" << powers_of_2[max->pow2] << " + "
		<< bits << " max" << std::endl;
    }
  }
}

int main(int argc,char *const argv[]){
  int i;
  poly_t final_poly,max_poly;
  unsigned long pow2 = 1,pow3 = 1;
  if (parse_options(argc,argv)){
    std::cerr << "usage: genpoly -p [-w width]" << std::endl;
    return 1;
  }
  for (i=0;i<32;i++){
    powers_of_2[i] = pow2;
    powers_of_3[i] = pow3;
    pow3 *= 3;
    pow2 *= 2;
  }
  for (i=0;i<(1<<width);i++){
    compute_poly(i,width,&final_poly,&max_poly);
  }
  return 0;
}
