CXX = g++

CXXFLAGS = -std=c++2a -O3

BINARIES=test

all: ${BINARIES} src/*.cpp
	@./test

test:
	@$(CXX) $(CXXFLAGS) $(wildcard src/*.cpp) -o $@

clean:
	@/bin/rm -f ${BINARIES} *.o

