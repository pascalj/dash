CXXFLAGS=-I. -I../include -std=c++14 -lgtest -lpthread

includes = $(wildcard ../include/patterns/*.h *.hpp)

all: main

main: main.cpp ${includes}
	$(CXX) -lgtest_main $(CXXFLAGS) -o main main.cpp

clean:
	rm -f main
