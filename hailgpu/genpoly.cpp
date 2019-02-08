// genpoly.c - generate polynomial table values
#include <iostream>
#include <string>
#include <unistd.h>
#include "config.hpp"
int width = POLY_WIDTH;
int verbose = 0;
int maxcutoff = 0;
int phflag = 0;
int pflag = 0;
int maxpow3 = 0;

int parse_options(int argc,char *const argv[]){
  int opt;
  while ((opt = getopt(argc,argv,"mpPt:vw:")) != -1){
    switch(opt){
    case 'm':
      maxcutoff = 1;
      break;
    case 'P':
      phflag = 1;
      break;
    case 'p':
      pflag = 1;
      break;
    case 't':
      maxpow3 = std::stoi(optarg);
      break;
    case 'v':
      verbose = 1;
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
  int lessthanoneany;  // during computation of the polynomial did it go below starting point?
  int lessthanone; // is the polynomial lower than the start?
  int duplicate;  // was there another polynomial alreay computed?
} poly_t;

unsigned long powers_of_2[32];
unsigned long powers_of_3[32];

void compute_poly(unsigned long bits,int width,poly_t *final,poly_t *max){
  static unsigned long last_poly_add[32] = { 0 }; // look for duplicates
  unsigned long savebits = bits;
  int power3 = 0;
  int power2 = 0;
  int steps = 0;
  poly_t partial = { 0 };
  int maxfound = 0;
  int lessthanoneany = 0;
  double maxratio = 1.0;
  max->pow2 = 0;
  max->pow3 = 0;
  max->add = 0;
  if (verbose)
    std::cout << "/*\npoly(" << bits << "," << width << ")" << std::endl;
  while (width > 0){
    if (bits & 0x1){
      bits = bits * 3 + 1;
      power3++;
      if (maxpow3 && (power3 > maxpow3)){
	// do nothing, already saved by following lines
      } else if (maxpow3 && (power3 == maxpow3)){
	if ((maxfound == 0) || (((double) powers_of_3[power3]/powers_of_2[power2]) > maxratio)){
	  maxfound = 1;
	  max->pow3 = power3;
	  max->pow2 = power2;
	  max->add = bits;
	  maxratio = ((double) powers_of_3[power3]/powers_of_2[power2]);	  
	}
      } else if (powers_of_3[power3] > powers_of_2[power2]){
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
      if (powers_of_3[power3] < powers_of_2[power2]){
	lessthanoneany = 1;
      }
    }
    steps++;
    partial.pow2 = power2;
    partial.pow3 = power3;
    partial.add = bits;
  }
  final->pow2 = power2;
  final->pow3 = power3;
  final->add = bits;
  final->lessthanoneany = lessthanoneany;
  final->lessthanone = (powers_of_3[power3] < powers_of_2[power2]);
  if (last_poly_add[power3] == bits && power3 != 0){
    final->duplicate = 1;
  } else {
    final->duplicate = 0;
    last_poly_add[power3] = bits;
  }
  if (verbose){
    if (powers_of_3 > 0){
      std::cout << "\t" << powers_of_3[power3] << "(X/" << powers_of_2[power2] << ") + "
		<< bits << ((final->lessthanoneany)?" ! lessthan1 ":"")
		<< ((final->duplicate)?" dup":"")
		<< std::endl;
    } else {
      std::cout << "\t" << "X/" << powers_of_2[power2] << " + "
		<< bits << ((final->lessthanoneany)?" ! lessthan1 ":"")
		<< ((final->duplicate)?" dup":"")
		<< std::endl;
    }
  }
  if (maxfound){
    if (verbose){
      if (max->pow3 > 0){
	std::cout << "\t" << powers_of_3[max->pow3] << "(X/" << powers_of_2[max->pow2] << ") + "
		  << bits << " max" << std::endl;
      } else {
	std::cout << "\t" << "X/" << powers_of_2[max->pow2] << " + "
		  << bits << " max" << std::endl;
      }
    }
  } else {
    max->pow2 = power2;
    max->pow3 = power3;
    max->add = bits;
    max->lessthanoneany = lessthanoneany;
    max->lessthanone = final->lessthanone;
    max->duplicate = final->duplicate;
  }
  if (verbose) std::cout << "*/" << std::endl;
  if (maxcutoff){
    if (!final->lessthanoneany && !final->duplicate){
      std::cout << "\t" << savebits << "," << std::endl;
    }
  }
  if (pflag){
    std::cout << "\t{ " << (powers_of_3[max->pow3] > powers_of_2[max->pow2]) << ", "
	      << powers_of_3[max->pow3] << ", "
	      << max->pow2 << ", "
	      << max->add << "}, // " << savebits << std::endl;
  }
}

void print_maxcutoff_start(int width){
  std::cout << "// maxcutoff " << std::endl;
  std::cout << "int maxcutoff_width = " << width << ";" << std::endl;
  std::cout << "unsigned int maxcutoff_value[] = {" << std::endl;
}

void print_maxcutoff_finish(int width){
  std::cout << "};" << std::endl;
  std::cout << "int maxcutoff_num = sizeof(maxcutoff_value)/sizeof(maxcutoff_value[0]);" << std::endl;
}

void print_polyheader(int width){
  if (width > 32){
    std::cerr << "max width is 32" << std::endl;
    return;
  } else if (width <= 0){
    std::cerr << "min width is 1" << std::endl;
    return;    
  }
  std::cout << "#define POLY_WIDTH " << width << std::endl;
  std::cout << "struct poly {" << std::endl;
  std::cout << "\tbool gtone;" << std::endl;
  if (powers_of_3[width-1] < (1l << 16)){
    std::cout << "\tunsigned short pow3;" << std::endl;
    std::cout << "\tunsigned short pow2;" << std::endl;
    std::cout << "\tunsigned int add;" << std::endl;
  } else if (powers_of_3[width-1] < (1UL << 32)){
    std::cout << "\tunsigned int pow3;" << std::endl;
    std::cout << "\tunsigned int pow2;" << std::endl;
    std::cout << "\tunsigned long add;" << std::endl;    
  } else {
    std::cout << "\tunsigned long pow3;" << std::endl;
    std::cout << "\tunsigned long pow2;" << std::endl;
    std::cout << "\tunsigned long add;" << std::endl;
  }
  std::cout << "};" << std::endl;
  std::cout << "extern struct poly poly_table[];" << std::endl;
}

void print_poly_start(int width){
  std::cout << "#include \"poly" << width << ".hpp\"" << std::endl;
  std::cout << "struct poly poly_table[] = {" << std::endl;
}

void print_poly_finish(int width){
  std::cout << "};" << std::endl;
}

int main(int argc,char *const argv[]){
  long i;
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
  if (maxcutoff) print_maxcutoff_start(width);
  if (phflag) print_polyheader(width);
  if (pflag) print_poly_start(width);
  for (i=0;i<(1UL<<width);i++){
    compute_poly(i,width,&final_poly,&max_poly);
  }
  if (maxcutoff) print_maxcutoff_finish(width);
  if (pflag) print_poly_finish(width);  
  return 0;
}
