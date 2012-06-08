# Compilers
CXX=clang++
GXX=g++

# Generic includes
INCLUDES=-I../..
		 
# Generic flags
CFLAGS=-std=c++0x -Wall -g -O0 

# Source files
CPP=main.cpp

# Output name
OUTPUT=TestApp

# Linux uses clang 3.0
clang:
	$(CXX) $(INCLUDES) $(CPP) $(CFLAGS) -o $(OUTPUT)
# Windows uses g++ 4.7 January 2012
gcc:
	$(GXX) $(INCLUDES) $(CPP) $(CFLAGS) -o $(OUTPUT)
