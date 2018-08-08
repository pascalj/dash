#include "PatternTree.h"
#include "Visualize.h"
#include <iostream>


template<typename T>
void name() {
  std::cout << __PRETTY_FUNCTION__ << std::endl;
}


using root   = split<PatternLeaf<Configuration<void, RootProjection>>>::type;
using level1 = split_right<split_left<root>::type>::type;
using level2 = replace_right<level1>::with<level1>::type;
using Node = level2;

int main() {
  std::cout << "graph tree {" << std::endl;
  level2 node;
  visualize(node);
  std::cout << "}" << std::endl;
}
