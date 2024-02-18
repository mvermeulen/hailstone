#include <stdint.h>
#if DEBUG
int32_t debug;
#endif
#if !NO_UPDATE
int32_t global_maxvalue_found;
int32_t global_maxsteps_found;
#endif
#define MAXDIGITS 100
int32_t global_maxvalue_size;
uint32_t global_maxvalue[MAXDIGITS];
int32_t global_maxsteps;
extern unsigned __int128 half_global_maxvalue128;
extern unsigned __int128 global_maxvalue128;
extern unsigned long pow3[];

int clz32[32];
int clz64[64];
void hail32m(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hail32n(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hail32l(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hail32pm(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hail32pn(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hail32pl(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hail32pmf(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size,int32_t *peak_found);
void hail32pnf(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size,int32_t *peak_found);
void hail32plf(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size,int32_t *peak_found);
void hail64am(uint64_t n,uint64_t steps,uint64_t maxsteps,uint64_t maxvalue_size,uint32_t *maxvalue,int32_t *peak_found);
void hail64an(uint64_t n,uint64_t steps,uint64_t maxsteps,uint64_t maxvalue_size,uint32_t *maxvalue,int32_t *peak_found);
void hail64m(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hail64n(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hail64l(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hail64pm(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hail64pn(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hail64pl(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hail64pmf(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size,int32_t *peak_found);
void hail64pnf(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size,int32_t *peak_found);
void hail64plf(uint32_t *n,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hailxm(uint32_t *n,int32_t nsize,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hailxn(uint32_t *n,int32_t nsize,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size);
void hailxmf(uint32_t *n,int32_t nsize,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size,int32_t *peak_found);
void hailxnf(uint32_t *n,int32_t nsize,int32_t *steps,int32_t *maxsteps,uint32_t *maxvalue,int32_t maxvalue_size,int32_t *peak_found);
void hail64ym(uint64_t num,int32_t steps,int32_t global_maxsteps,unsigned __int128 global_maxvalue128,int32_t *peak_found);
void hail64yn(uint64_t num,int32_t steps,int32_t global_maxsteps,int32_t *peak_found);

#if defined LOOKUP_WIDTH
#if LOOKUP_WIDTH == 8
extern int steps8[];
#elif LOOKUP_WIDTH == 10
extern int steps10[];
#elif LOOKUP_WIDTH == 16
extern int steps16[];
#elif LOOKUP_WIDTH == 20
extern int steps20[];
#elif LOOKUP_WIDTH == 24
extern int steps24[];
#else
#error Unknown LOOKUP_WIDTH
#endif
#endif

#if defined CUTOFF_WIDTH
#if CUTOFF_WIDTH == 8
extern unsigned char cutoff8[];
#elif CUTOFF_WIDTH == 10
extern unsigned char cutoff10[];
#elif CUTOFF_WIDTH == 16
extern unsigned char cutoff16[];
#elif CUTOFF_WIDTH == 20
extern unsigned char cutoff20[];
#else
#error Unknown CUTOFF_WIDTH
#endif
#endif

#if defined POLY_WIDTH
struct poly {
  unsigned int mul3;
  unsigned short div2;
  unsigned short steps;
  unsigned int add;
  unsigned char smaller;
};
#if POLY_WIDTH == 8
extern struct poly fpoly8[],mpoly8[];
#elif POLY_WIDTH == 10
extern struct poly fpoly10[],mpoly10[];
#elif POLY_WIDTH == 16
extern struct poly fpoly16[],mpoly16[];
#elif POLY_WIDTH == 20
extern struct poly fpoly20[],mpoly20[];
#else
#error Unknown CUTOFF_WIDTH
#endif
#endif

enum hail_message {
  HAIL_ERROR = -1,
  HAIL_FASTSEARCH = 1,
  HAIL_NOPEAK = 2,
  HAIL_PEAKFOUND = 3,
};

struct hail_packet {
  enum hail_message message;
  uint64_t start_block;
  uint64_t num_blocks;
  uint32_t maxsteps;
  uint32_t maxvalue_size;
  uint32_t maxvalue[MAXDIGITS];
};

void recover_checkpoint_file(void);
int check_prediction(unsigned long int blocknum);
void search_block(uint64_t blocknum);
void remove_prediction(unsigned long int blocknum);
void save_checkpoint_file(uint64_t blocknum);
