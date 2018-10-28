CXXFLAGS=-I./include -std=c++14 -g

includes = $(wildcard include/patterns/*.h)

all: examples


examples: examples.cpp ${includes}
	$(CXX) $(CXXFLAGS) -o examples examples.cpp

clean:
	rm -f graph examples
