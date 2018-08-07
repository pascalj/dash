#include "PatternTree.h"
#include <iostream>


template <typename T> std::string type_name();

template<typename T>
void name() {
  std::cout << __PRETTY_FUNCTION__ << std::endl;
}

int main() {
  name<level1>();
  name<get_node<level1>::at<0, 0>::type>();
  name<get_node<level1>::at<0, 1>::type>();
  name<level2>();
  name<get_node<level2>::at<2, 1>::type>();
}
