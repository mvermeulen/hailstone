#
CXX=g++
CXXOPTS=-O3 -std=c++11
HCC=/opt/rocm/bin/hcc
HCCOPTS=`/opt/rocm/bin/hcc-config --cxxflags`
HCCLDOPTS=`/opt/rocm/bin/hcc-config --ldflags`
WIDTH=32

hailmax:	hailmax.o cutoff24.o poly10.o
	$(CXX) -o hailmax hailmax.o cutoff24.o poly10.o

hailmax.o:	hailmax.cpp config.hpp poly10.hpp
	$(CXX) $(CXXOPTS) -c hailmax.cpp 

genpoly:	genpoly.cpp config.hpp

	$(CXX) -o genpoly $(CXXOPTS) genpoly.cpp

allcutoff:	cutoff32.c cutoff30.c cutoff28.c cutoff26.c cutoff24.c cutoff22.c cutoff20.c cutoff18.c cutoff16.c cutoff14.c cutoff12.c cutoff10.c

allpolyhpp:	poly32.hpp poly30.hpp poly28.hpp poly26.hpp poly24.hpp poly22.hpp poly20.hpp poly18.hpp poly16.hpp poly14.hpp poly12.hpp poly10.hpp poly8.hpp poly6.hpp

allpolycpp:	poly32.cpp poly30.cpp poly28.cpp poly26.cpp poly24.cpp poly22.cpp poly20.cpp poly18.cpp poly16.cpp poly14.cpp poly12.cpp poly10.cpp poly8.cpp poly6.cpp

poly32.hpp:	genpoly
	./genpoly -P -w 32 > poly32.hpp
poly30.hpp:	genpoly
	./genpoly -P -w 30 > poly30.hpp
poly28.hpp:	genpoly
	./genpoly -P -w 28 > poly28.hpp
poly26.hpp:	genpoly
	./genpoly -P -w 26 > poly26.hpp
poly24.hpp:	genpoly
	./genpoly -P -w 24 > poly24.hpp
poly22.hpp:	genpoly
	./genpoly -P -w 22 > poly22.hpp
poly20.hpp:	genpoly
	./genpoly -P -w 20 > poly20.hpp
poly18.hpp:	genpoly
	./genpoly -P -w 18 > poly18.hpp
poly16.hpp:	genpoly
	./genpoly -P -w 16 > poly16.hpp
poly14.hpp:	genpoly
	./genpoly -P -w 14 > poly14.hpp
poly12.hpp:	genpoly
	./genpoly -P -w 12 > poly12.hpp
poly10.hpp:	genpoly
	./genpoly -P -w 10 -t 5 > poly10.hpp
poly8.hpp:	genpoly
	./genpoly -P -w 8 > poly8.hpp
poly6.hpp:	genpoly
	./genpoly -P -w 6 > poly6.hpp

poly32.cpp:	genpoly poly32.hpp
	./genpoly -p -w 32 > poly32.cpp
poly30.cpp:	genpoly poly30.hpp
	./genpoly -p -w 30 > poly30.cpp
poly28.cpp:	genpoly poly28.hpp
	./genpoly -p -w 28 > poly28.cpp
poly26.cpp:	genpoly poly26.hpp
	./genpoly -p -w 26 > poly26.cpp
poly24.cpp:	genpoly poly24.hpp
	./genpoly -p -w 24 > poly24.cpp
poly22.cpp:	genpoly poly22.hpp
	./genpoly -p -w 22 > poly22.cpp
poly20.cpp:	genpoly poly20.hpp
	./genpoly -p -w 20 > poly20.cpp
poly18.cpp:	genpoly poly18.hpp
	./genpoly -p -w 18 > poly18.cpp
poly16.cpp:	genpoly poly16.hpp
	./genpoly -p -w 16 > poly16.cpp
poly14.cpp:	genpoly poly14.hpp
	./genpoly -p -w 14 > poly14.cpp
poly12.cpp:	genpoly poly12.hpp
	./genpoly -p -w 12 > poly12.cpp
poly10.cpp:	genpoly poly10.hpp
	./genpoly -p -w 10 -t 5 > poly10.cpp
poly8.cpp:	genpoly poly8.hpp
	./genpoly -p -w 8 > poly8.cpp
poly6.cpp:	genpoly poly6.hpp
	./genpoly -p -w 6 > poly6.cpp

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
	- rm genpoly hailmax
