Computations of hailstone numbers (Collatz conjecture).  In particular:

T(n) = n*3+1 if n is odd
       n/2   if n is even

where: steps is number of iterations of T before it reaches 1 and maxvalue
is the maximum value reached.  See also:
https://en.wikipedia.org/wiki/Collatz_conjecture

Started playing with this in 1986 and was coauthor on a paper in 1990 -
https://www.sciencedirect.com/science/article/pii/089812219290034F

Since then an off/on hobby including these codes redone in 2014.

This search program can be configured with several variables in the Makefile:
   POLY_WIDTH is number of bits to compute polynomials. Currently 10 since
      3^10 is 59049 and still fits in 16-bit width
   LOOKUP_WIDTH is the last number of steps to be looked up, currently 20
      since this table adds to executable size.
   CUTOFF_WIDTH is the polynomial size used as a sieve for no computation and
      also to know no maximum is possible.
These have not been changed for a while, so likely we can increase the
sizes for more recent processors.

The hailstone program searches blocks of 2^32 numbers either in parallel using
OpenMP or individually.  It takes the following options:
   -b is the starting block, e.g. -b 2099 starts at 2099 * 2^32
   -c file is the name of the checkpoint file
   -f uses "fast search", this searches blocks in parallel and either
      completes without a peak or stops to determine a peak was found.
      If a peak is found, it backtracks and searches only the first block.
   -l is the name of the logfile
   -w is the width of the number of blocks search in parallel

The search happens with various search functions named for whether we need
to find a max in values and the width of the search, e.g.

   Used by the fast parallel search (hailstone.c)
      hail64am - searches 64-bit width including max in values (assembly code)
      hail64an - searches 64-bit width without max in values (assembly code)
      hailxmf - searches >64-bit width including max in values (C code)
      hailxnf - searches >64-bit width without max in values (C code)
   Used by the normal search (hailutil.c)
      hail64pm - searches 64-bit width, saves max (C code) 
      hail64pn - searches 64-bit width, no maximum (C code)
      hailxm - searches >64-bit width saving the max steps/value
      hailxn - searches >64-bit width saving the max steps
      hail32pm - searches 32-bit width with max
      hail32pn - searches 32-bit witdth without max
      hail32m - searches the first 2^32 block0

Following are some profile times reported from a search around block 4000:

  41.50%  hailstone.daemo  hailstone.daemonf  [.] fast_search
  32.92%  hailstone.daemo  hailstone.daemonf  [.] hail64an
  25.24%  hailstone.daemo  hailstone.daemonf  [.] hail64am
   0.02%  hailstone.daemo  hailstone.daemonf  [.] hailxmf
   0.00%  hailstone.daemo  hailstone.daemonf  [.] hailxnf

This suggest most time spent in the fast_search which filters and organizes
the search blocks and then the two assembly routines for fast searching.  I
expect as blocks get larger, we will spend more time in the "x" routines
for more than 64-bit widths.

David Barina creates an interesting new algorithm that avoids the need for
these larger polynomial lookups: https://rdcu.be/b5nn1 that looks like an
interesting approach to try (particularly on GPU code where we use clz and
avoid the need for branching).
