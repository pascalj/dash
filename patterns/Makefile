CXXFLAGS=-I./include -std=c++14 -g

includes = $(wildcard include/patterns/*.h)

all: graph examples

graph: graph.cpp ${includes}
	$(CXX) $(CXXFLAGS) -o graph graph.cpp

examples: examples.cpp ${includes}
	$(CXX) $(CXXFLAGS) -o examples examples.cpp

clean:
	rm -f graph examples
