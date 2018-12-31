// config.hpp - configuration parameters for hailstone GPU computations
#define PARALLEL_COMPUTATIONS 2048
#define NUMBER_ITERATIONS       10
#define POLY_WIDTH              10 // 3^10 == 59049 fits in 16 bits

typedef unsigned short digit_t;    // 16-bit
#define MAX_DIGIT                8 // 128-bit

