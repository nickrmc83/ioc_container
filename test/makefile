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

.PHONY:all run_cov

all : $(OUTPUT) run_cov

# Linux can use clang++ 3.x or g++ 4.7
$(OUTPUT):
	$(CXX) $(INCLUDES) $(CPP) $(CFLAGS) -o $(OUTPUT)

# Code coverage using gcov
$(OUTPUT).cov:
	$(CXX) $(INCLUDES) -g $(CPP) $(CFLAGS) $(COV_FLAGS) -o $@

run_cov : $(OUTPUT).cov
	./$<
	gcov -r $(CPP)

clean:
	rm -r -f $(OUTPUT)*
	rm -r -f ../*~
	rm -r -f *~
	rm -r -f *.gcov
	rm -r -f *.gcno
	rm -r -f *.gcda
