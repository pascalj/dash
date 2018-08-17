
CXXFLAGS=-I./include -std=c++14

includes = $(wildcard include/*.h)

graph: graph.cpp ${includes}
	$(CXX) $(CXXFLAGS) -o graph graph.cpp

clean:
	rm -f graph
