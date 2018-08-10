#include "patterns/PatternTree.h"
#include "patterns/Entity.h"
#include "patterns/Visualize.h"
#include "patterns/EntityMapping.h"
#include <iostream>


template<typename T>
void name() {
  std::cout << __PRETTY_FUNCTION__ << std::endl;
}

using CPULeaf = PatternLeaf<Configuration<Process>>;
using GPULeaf = PatternLeaf<Configuration<Process>>;
using root   = split<PatternLeaf<Configuration<None, RootProjection>>>::type;
using level1 = split_right<split_left<root>::type>::type;
using level2 = replace_right<level1>::with<level1>::type;
using level3 = replace_left<level1>::with<CPULeaf>::type;
using Node = level3;

int main() {
  std::cout << "graph tree {" << std::endl;
  level3 node;
  visualize(node);
  std::cout << "}" << std::endl;
  Process p;
  RoundRobinEntityMapping<level3> mapping;
  std::cout << "matching nodes: " << mapping.map(p) << std::endl;
}
