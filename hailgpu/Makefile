#
CXX=g++
CXXOPTS=-g -std=c++11
HCC=/opt/rocm/bin/hcc
HCCOPTS=`/opt/rocm/bin/hcc-config --cxxflags`
HCCLDOPTS=`/opt/rocm/bin/hcc-config --ldflags`

genpoly:	genpoly.cpp
	$(CXX) -o genpoly $(CXXOPTS) genpoly.cpp
