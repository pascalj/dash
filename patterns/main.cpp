#include "PatternTree.h"
#include "Visualize.h"
#include <iostream>


template<typename T>
void name() {
  std::cout << __PRETTY_FUNCTION__ << std::endl;
}

int main() {
  std::cout << "graph tree {" << std::endl;
  level2 node;
  visualize(node);
  std::cout << "}" << std::endl;
}
