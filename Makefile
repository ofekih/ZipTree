CXX = g++

CXXFLAGS = -std=c++11 -O3

BINARIES= test

all: ${BINARIES}
	@./test

test:
	@$(CXX) $(CXXFLAGS) $(wildcard src/*.cpp) -o $@

clean:
	@/bin/rm -f ${BINARIES} *.o

