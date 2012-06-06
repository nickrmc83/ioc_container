# Compilers
CXX=clang++
GXX=g++

# Generic includes
INCLUDES=-I../. \
		 -I../..

# Windows/gcc includes
GINCLUDES=-I../.. \
         -IC:\Users\Nick\mingw\x86_64-w64-mingw32\include \
		 -IC:\Users\Nick\mingw\x86_64-w64-mingw32\include\c++\4.7.0 \
		 -IC:\Users\Nick\mingw\x86_64-w64-mingw32\include\c++\4.7.0\x86_64-w64-mingw32
		 
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
