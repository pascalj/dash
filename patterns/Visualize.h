#include <string>
#include <cstdio>

template<typename Config, typename Left, typename Right>
class PatternNode;


template<typename Config, typename Left, typename Right>
std::enable_if<std::is_same<Left, void>::value> visualize(PatternNode<Config, Left, Right> &n, int i = 0, int o = 0) {
  printf("Node%d_%d -- Node%d_%d\n", i, o, i + 1, o * 2);
  printf("Node%d_%d -- Node%d_%d\n", i, o, i + 1, o * 2 + 1);
  Left l;
  Right r;
  visualize(l, i + 1, o);
  visualize(r, i + 1, o  * 2 + 1);
}

template<typename Config, typename Right>
void visualize(PatternNode<Config, void, Right> &n, int i = 0, int o = 0) {
  printf("Node%d\n", i);
  Right r;
  visualize(r, i + 1);
}

template<typename Config, typename Left>
void visualize(PatternNode<Config, Left, void> &n, int i = 0, int o = 0) {
  printf("Node%d\n", i);
  Left l;
  visualize(l, i + 1);
}

template<typename Config>
void visualize(PatternNode<Config, void, void> &n, int i = 0, int o = 0) {
}
