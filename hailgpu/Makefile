#
CXX=g++
CXXOPTS=-g -std=c++11
HCC=/opt/rocm/bin/hcc
HCCOPTS=`/opt/rocm/bin/hcc-config --cxxflags`
HCCLDOPTS=`/opt/rocm/bin/hcc-config --ldflags`
WIDTH=32

genpoly:	genpoly.cpp
	$(CXX) -o genpoly $(CXXOPTS) genpoly.cpp

allcutoff:	cutoff32.c cutoff30.c cutoff28.c cutoff26.c cutoff24.c cutoff22.c cutoff20.c cutoff18.c cutoff16.c cutoff14.c cutoff12.c cutoff10.c

cutoff32.c:	genpoly
	./genpoly -m -w 32 > cutoff32.c

cutoff30.c:	genpoly
	./genpoly -m -w 30 > cutoff30.c

cutoff28.c:	genpoly
	./genpoly -m -w 28 > cutoff28.c

cutoff26.c:	genpoly
	./genpoly -m -w 26 > cutoff26.c

cutoff24.c:	genpoly
	./genpoly -m -w 24 > cutoff24.c

cutoff22.c:	genpoly
	./genpoly -m -w 22 > cutoff22.c

cutoff20.c:	genpoly
	./genpoly -m -w 20 > cutoff20.c

cutoff18.c:	genpoly
	./genpoly -m -w 18 > cutoff18.c

cutoff16.c:	genpoly
	./genpoly -m -w 16 > cutoff16.c

cutoff14.c:	genpoly
	./genpoly -m -w 14 > cutoff14.c

cutoff12.c:	genpoly
	./genpoly -m -w 12 > cutoff12.c

cutoff10.c:	genpoly
	./genpoly -m -w 10 > cutoff10.c

clean:
	- rm *~ *.o

clobber:	clean
	- rm genpoly
