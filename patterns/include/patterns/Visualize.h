#include <string>
#include <cstdio>

#include "Configuration.h"
#include "Entity.h"

template <typename Config, typename Left, typename Right>
class PatternNode;

template <typename Config, typename Left, typename Right>
void visualize(PatternNode<Config, Left, Right> &n, int i = 0)
{
  using Entity = typename get_entity_t<Config>::type;
  printf("Node%d [label=\"Node%d  %s\"]\n", i, i, entity_name<Entity>::value);
  printf("Node%d -- Node%d\n", i, i * 2 + 1);
  printf("Node%d -- Node%d\n", i, i * 2 + 2);
  Left  l;
  Right r;
  visualize(l, i * 2 + 1);
  visualize(r, i * 2 + 2);
}

template <typename Config, typename Right>
void visualize(PatternNode<Config, void, Right> &n, int i = 0)
{
  printf("Noder%d\n", i);
  Right r;
  visualize(r, i + 1);
}

template <typename Config, typename Left>
void visualize(PatternNode<Config, Left, void> &n, int i = 0)
{
  printf("Nodel%d\n", i);
  Left l;
  visualize(l, i + 1);
}

template <typename Config>
void visualize(PatternNode<Config, void, void> &n, int i = 0)
{
  using Entity = typename get_entity_t<Config>::type;
  printf("Node%d [label=\"Node%d  %s\"]\n", i, i, entity_name<Entity>::value);
}
