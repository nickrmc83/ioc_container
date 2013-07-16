# standard makefile for unit test project
# Usage: g++ users should use the build
# command:
#          make gcc
# while clang users should use:
#          make clang

# Compilers
CXX=clang++
GXX=g++ # g++-4.7 # name of installed 4.7 version on Ubuntu

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
clang:
	$(CXX) $(INCLUDES) $(CPP) $(CFLAGS) -o $(OUTPUT)
# Windows uses g++ 4.7 January 2012
gcc:
	$(GXX) $(INCLUDES) $(CPP) $(CFLAGS) -fstack-protector-all -o $(OUTPUT)
# Code coverage using gcov
cov:
	$(GXX) $(INCLUDES) -g $(CPP) $(CFLAGS) $(COV_FLAGS) -o $(OUTPUT)
# instrementation build
inst:
	$(GXX) $(INCLUDES) $(CPP) instrument.cpp $(CFLAGS) -o $(OUTPUT) -finstrument-functions -finstrument-functions-exclude-file-list=$(EXINST)
clean:
	rm -r -f $(OUTPUT)*
	rm -r -f ../*~
	rm -r -f *~
