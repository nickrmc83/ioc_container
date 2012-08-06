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

# Source files
CPP=main.cpp

# Output name
OUTPUT=test_app

# Linux uses clang 3.0
clang:
	$(CXX) $(INCLUDES) $(CPP) $(CFLAGS) -o $(OUTPUT)
# Windows uses g++ 4.7 January 2012
gcc:
	$(GXX) $(INCLUDES) $(CPP) $(CFLAGS) -o $(OUTPUT)
