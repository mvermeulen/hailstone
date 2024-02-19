#
# Makefile for hailstone program
#
DEBUG=0
LOOKUP_WIDTH=20
CUTOFF_WIDTH=16
POLY_WIDTH=10
CFLAGS=-no-pie -O3 -march=native -fopenmp
CC=gcc -fcommon -DDEBUG=$(DEBUG)

all:	hailstone.daemon hailstone.daemonf hailmgr

hailstone.daemon:	hailstone.o hailutil.o hail64am.o hail64an.o hail32m.o hail32n.o hail32l.o hail32pm.o hail32pn.o hail32pl.o hail32pmf.o hail32pnf.o hail64m.o hail64n.o hail64l.o hail64pm.o hail64pn.o hail64pl.o hail64pmf.o hail64pnf.o hailxm.o hailxn.o hailxmf.o hailxnf.o steps$(LOOKUP_WIDTH).o cutoff$(CUTOFF_WIDTH).o mpoly$(POLY_WIDTH).o fpoly$(POLY_WIDTH).o fpoly16.o mpoly16.o hailym.o hailyn.o
	$(CC) $(CFLAGS) -o hailstone.daemon hailstone.o hailutil.o hail64am.o hail64an.o hailym.o hailyn.o hail32m.o hail32n.o hail32l.o hail32pm.o hail32pn.o hail32pl.o hail32pmf.o hail32pnf.o hail64m.o hail64n.o hail64l.o hail64pm.o hail64pn.o hail64pl.o hail64pmf.o hail64pnf.o hailxm.o hailxn.o hailxmf.o hailxnf.o steps$(LOOKUP_WIDTH).o cutoff$(CUTOFF_WIDTH).o mpoly$(POLY_WIDTH).o fpoly$(POLY_WIDTH).o fpoly16.o mpoly16.o

hailstone.daemonf:	hailstonef.o hailutil.o hail64am.o hail64an.o hail64pm.o hail64pn.o hail32m.o hail32pm.o hail64m.o hail64n.o hail32pn.o hailxmf.o hailxnf.o hailxm.o hailxn.o steps$(LOOKUP_WIDTH).o cutoff$(CUTOFF_WIDTH).o fpoly$(POLY_WIDTH).o mpoly$(POLY_WIDTH).o fpoly16.o mpoly16.o hailym.o hailyn.o
	$(CC) $(CFLAGS) -o hailstone.daemonf hailutil.o hailstonef.o hail64am.o hail64an.o hailym.o hailyn.o hail64pm.o hail64pn.o hail64m.o hail64n.o hail32m.o hail32pm.o hail32pn.o steps$(LOOKUP_WIDTH).o cutoff$(CUTOFF_WIDTH).o fpoly$(POLY_WIDTH).o mpoly$(POLY_WIDTH).o hailxmf.o hailxnf.o hailxm.o hailxn.o fpoly16.o mpoly16.o

hailmgr:	hailmgr.o hailutil.o hail64pm.o hail64pn.o hail64m.o hail64n.o hail32pm.o hail32pn.o hail32m.o hail32n.o hailxm.o hailxn.o steps$(LOOKUP_WIDTH).o cutoff$(CUTOFF_WIDTH).o mpoly$(POLY_WIDTH).o fpoly$(POLY_WIDTH).o
	$(CC) -o hailmgr $(CFLAGS) hailmgr.o hailutil.o hail64pm.o hail64pn.o hail64m.o hail64n.o hail32pm.o hail32pn.o hail32m.o hail32n.o hailxm.o hailxn.o steps$(LOOKUP_WIDTH).o cutoff$(CUTOFF_WIDTH).o mpoly$(POLY_WIDTH).o fpoly$(POLY_WIDTH).o hailym.o hailyn.o

hailstone.o:	hailstone.c hailstone.h steps$(LOOKUP_WIDTH).c cutoff$(CUTOFF_WIDTH).c fpoly$(POLY_WIDTH).c mpoly$(POLY_WIDTH).c
	$(CC) -c -o hailstone.o $(CFLAGS) -DPOLYNOMIAL=1 -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DCUTOFF_WIDTH=$(CUTOFF_WIDTH) -DPOLY_WIDTH=$(POLY_WIDTH) hailstone.c

hailstonef.o:	hailstone.c hailstone.h steps$(LOOKUP_WIDTH).c cutoff$(CUTOFF_WIDTH).c fpoly$(POLY_WIDTH).c mpoly$(POLY_WIDTH).c
	$(CC) -c -o hailstonef.o $(CFLAGS) -DEXPERIMENTAL=1 -DPOLYNOMIAL=1 -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DCUTOFF_WIDTH=$(CUTOFF_WIDTH) -DPOLY_WIDTH=$(POLY_WIDTH) hailstone.c

hailmgr.o:	hailmgr.c
	$(CC) -c $(CFLAGS) hailmgr.c

hailutil.o:	hailutil.c
	$(CC) -c $(CFLAGS) -DCUTOFF_WIDTH=$(CUTOFF_WIDTH) hailutil.c

genpoly:	genpoly.o
	$(CC) -o genpoly $(CFLAGS) genpoly.o

genpoly.o:	genpoly.c
	$(CC) -c $(CFLAGS) genpoly.c

steps$(LOOKUP_WIDTH).o: steps$(LOOKUP_WIDTH).c
	$(CC) -c -O0 steps$(LOOKUP_WIDTH).c

steps$(LOOKUP_WIDTH).c:	genpoly
	./genpoly -t -w $(LOOKUP_WIDTH) > steps$(LOOKUP_WIDTH).c

cutoff$(CUTOFF_WIDTH).o: cutoff$(CUTOFF_WIDTH).c
	$(CC) -c -O0 cutoff$(CUTOFF_WIDTH).c

cutoff$(CUTOFF_WIDTH).c: genpoly
	./genpoly -c -w $(CUTOFF_WIDTH) > cutoff$(CUTOFF_WIDTH).c

fpoly$(POLY_WIDTH).o: fpoly$(POLY_WIDTH).c
	$(CC) -c -O0 fpoly$(POLY_WIDTH).c

fpoly$(POLY_WIDTH).c: genpoly
	./genpoly -f -w $(POLY_WIDTH) > fpoly$(POLY_WIDTH).c

fpoly16.o:	fpoly16.c
	$(CC) -c -O0 fpoly16.c

fpoly16.c:	genpoly
	./genpoly -f -w 16 > fpoly16.c

mpoly16.o:	mpoly16.c
	$(CC) -c -O0 mpoly16.c

mpoly16.c:	genpoly
	./genpoly -m -w 16 > mpoly16.c

mpoly$(POLY_WIDTH).o: mpoly$(POLY_WIDTH).c
	$(CC) -c -O0 mpoly$(POLY_WIDTH).c

mpoly$(POLY_WIDTH).c: genpoly
	./genpoly -m -w $(POLY_WIDTH) > mpoly$(POLY_WIDTH).c

hail64am.o:	hail64am.s
	$(CC) -fPIC -c $(CFLAGS) hail64am.s

hail64an.o:	hail64an.s
	$(CC) -fPIC -c $(CFLAGS) hail64an.s

hail64am.s:	hail64asm.s
	-unifdef -DCHECK_MAXVALUE hail64asm.s > hail64am.s

hail64an.s:	hail64asm.s
	-unifdef -UCHECK_MAXVALUE hail64asm.s > hail64an.s

hail32m.o:	hail32.c hailstone.h steps$(LOOKUP_WIDTH).c 
	$(CC) -c -o hail32m.o $(CFLAGS) -DCHECK_MAXVALUE=1 -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DCHECK_MAXSTEPS=1 hail32.c

hail32l.o:	hail32.c hailstone.h steps$(LOOKUP_WIDTH).c
	$(CC) -c -o hail32l.o $(CFLAGS) -DCHECK_MAXVALUE_PARENTS=1 -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DCHECK_MAXSTEPS=1 hail32.c

hail32n.o:	hail32.c hailstone.h steps$(LOOKUP_WIDTH).c
	$(CC) -c -o hail32n.o $(CFLAGS) -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DCHECK_MAXSTEPS=1 hail32.c

hail32pm.o:	hail32p.c hailstone.h steps$(LOOKUP_WIDTH).c fpoly$(POLY_WIDTH).c mpoly$(POLY_WIDTH).c
	$(CC) -c -o hail32pm.o $(CFLAGS) -DCHECK_MAXVALUE=1 -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DPOLY_WIDTH=$(POLY_WIDTH) -DCHECK_MAXSTEPS=1 hail32p.c

hail32pn.o:	hail32p.c hailstone.h steps$(LOOKUP_WIDTH).c fpoly$(POLY_WIDTH).c mpoly$(POLY_WIDTH).c
	$(CC) -c -o hail32pn.o $(CFLAGS) -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DPOLY_WIDTH=$(POLY_WIDTH) -DCHECK_MAXSTEPS=1 hail32p.c

hail32pl.o:	hail32p.c hailstone.h steps$(LOOKUP_WIDTH).c fpoly$(POLY_WIDTH).c mpoly$(POLY_WIDTH).c
	$(CC) -c -o hail32pl.o $(CFLAGS) -DCHECK_MAXVALUE_PARENTS=1 -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DPOLY_WIDTH=$(POLY_WIDTH) -DCHECK_MAXSTEPS=1 hail32p.c

hail32pmf.o:	hail32p.c hailstone.h steps$(LOOKUP_WIDTH).c fpoly$(POLY_WIDTH).c mpoly$(POLY_WIDTH).c
	$(CC) -c -o hail32pmf.o $(CFLAGS) -DNO_UPDATE=1 -DCHECK_MAXVALUE=1 -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DPOLY_WIDTH=$(POLY_WIDTH) -DCHECK_MAXSTEPS=1 hail32p.c

hail32pnf.o:	hail32p.c hailstone.h steps$(LOOKUP_WIDTH).c fpoly$(POLY_WIDTH).c mpoly$(POLY_WIDTH).c
	$(CC) -c -o hail32pnf.o $(CFLAGS) -DNO_UPDATE=1 -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DPOLY_WIDTH=$(POLY_WIDTH) -DCHECK_MAXSTEPS=1 hail32p.c

hail64m.o:	hail64.c hailstone.h
	$(CC) -c -o hail64m.o $(CFLAGS) -DCHECK_MAXVALUE=1 -DCHECK_MAXSTEPS=1 hail64.c

hail64l.o:	hail64.c hailstone.h
	$(CC) -c -o hail64l.o $(CFLAGS) -DCHECK_MAXVALUE_PARENTS=1 -DCHECK_MAXSTEPS=1 hail64.c

hail64n.o:	hail64.c hailstone.h
	$(CC) -c -o hail64n.o $(CFLAGS) -DCHECK_MAXSTEPS=1 hail64.c

hail64pm.o:	hail64p.c hailstone.h steps$(LOOKUP_WIDTH).c fpoly$(POLY_WIDTH).c mpoly$(POLY_WIDTH).c
	$(CC) -c -o hail64pm.o $(CFLAGS) -DCHECK_MAXVALUE=1 -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DPOLY_WIDTH=$(POLY_WIDTH) -DCHECK_MAXSTEPS=1 hail64p.c

hail64pn.o:	hail64p.c hailstone.h steps$(LOOKUP_WIDTH).c fpoly$(POLY_WIDTH).c mpoly$(POLY_WIDTH).c
	$(CC) -c -o hail64pn.o $(CFLAGS) -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DPOLY_WIDTH=$(POLY_WIDTH) -DCHECK_MAXSTEPS=1 hail64p.c

hail64pl.o:	hail64p.c hailstone.h steps$(LOOKUP_WIDTH).c fpoly$(POLY_WIDTH).c mpoly$(POLY_WIDTH).c
	$(CC) -c -o hail64pl.o $(CFLAGS) -DCHECK_MAXVALUE_PARENTS=1 -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DPOLY_WIDTH=$(POLY_WIDTH) -DCHECK_MAXSTEPS=1 hail64p.c

hail64pmf.o:	hail64p.c hailstone.h steps$(LOOKUP_WIDTH).c fpoly$(POLY_WIDTH).c mpoly$(POLY_WIDTH).c
	$(CC) -c -o hail64pmf.o $(CFLAGS) -DCHECK_MAXVALUE=1 -DNO_UPDATE=1 -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DPOLY_WIDTH=$(POLY_WIDTH) -DCHECK_MAXSTEPS=1 hail64p.c

hail64pnf.o:	hail64p.c hailstone.h steps$(LOOKUP_WIDTH).c fpoly$(POLY_WIDTH).c mpoly$(POLY_WIDTH).c
	$(CC) -c -o hail64pnf.o $(CFLAGS) -DNO_UPDATE=1 -DLOOKUP_WIDTH=$(LOOKUP_WIDTH) -DPOLY_WIDTH=$(POLY_WIDTH) -DCHECK_MAXSTEPS=1 hail64p.c

hailxm.o:	hailx.c hailstone.h
	$(CC) -c -o hailxm.o $(CFLAGS) -DCHECK_MAXVALUE=1 -DCHECK_MAXSTEPS=1 hailx.c

hailxn.o:	hailx.c hailstone.h
	$(CC) -c -o hailxn.o $(CFLAGS) -DCHECK_MAXSTEPS=1 hailx.c

hailym.o:	haily.c hailstone.h
	$(CC) -c -o hailym.o $(CFLAGS) -DCHECK_MAXVALUE=1 -DCHECK_MAXSTEPS=1 haily.c

hailyn.o:	haily.c hailstone.h
	$(CC) -c -o hailyn.o $(CFLAGS) -DCHECK_MAXSTEPS=1 haily.c

hailxmf.o:	hailx.c hailstone.h
	$(CC) -c -o hailxmf.o $(CFLAGS) -DNO_UPDATE=1 -DCHECK_MAXVALUE=1 -DCHECK_MAXSTEPS=1 hailx.c

hailxnf.o:	hailx.c hailstone.h
	$(CC) -c -o hailxnf.o $(CFLAGS) -DNO_UPDATE=1 -DCHECK_MAXSTEPS=1 hailx.c

clean:
	-rm *.o *~

clobber:	clean
	-rm hailstone.daemon steps*.c cutoff*.c fpoly*.c mpoly*.c genpoly hail64am.s hail64an.s hailstone.daemonf hailmgr
