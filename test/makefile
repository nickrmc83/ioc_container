# standard makefile for unit test project
# Usage: g++ users should use the build
# command:
#          make gcc
# while clang users should use:
#          make clang

# Generic includes
INCLUDES=-I../.. \
		 -I../.
		 
# Generic flags
CFLAGS=-std=c++0x -Wall -g -O0
COV_FLAGS=-fprofile-arcs -ftest-coverage

# Source files
CPP=main.cpp

# Output name
OUTPUT=test_app

# files to exclude from instrumentation
EXINST=typeinfo,stdlib.h,string,stl_vector.h,stl_iterator.h

# Linux can use clang++ 3.x or g++ 4.7
all:
	$(CXX) $(INCLUDES) $(CPP) $(CFLAGS) -o $(OUTPUT)

# Code coverage using gcov
cov:
	$(CXX) $(INCLUDES) -g $(CPP) $(CFLAGS) $(COV_FLAGS) -o $(OUTPUT)

# instrementation build
inst:
	$(CXX) $(INCLUDES) $(CPP) instrument.cpp $(CFLAGS) -o $(OUTPUT) -finstrument-functions -finstrument-functions-exclude-file-list=$(EXINST)
clean:
	rm -r -f $(OUTPUT)*
	rm -r -f ../*~
	rm -r -f *~
