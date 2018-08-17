#ifndef PATTERNS__NODE_MAPPING_H
#define PATTERNS__NODE_MAPPING_H

#include <cstddef>
#include <type_traits>
#include "Configuration.h"
#include "PatternTree.h"
#include "Projection.h"

template<typename Entity>
struct Location {
        Entity &entity;
        size_t offset;
};

struct NodeAddress {
  int Level;
  int Offset;
};

template<typename PatternTree, typename Entity>
constexpr inline Location<Entity> map_element(size_t Index) {
  using Configuration = typename get_config<PatternTree>::type;
  using Projection = typename get_projection<Configuration>::type;


  return Index;
}

template <typename PatternTree>
struct ElementMapping {

template <typename Entity = void>
constexpr inline typename std::enable_if<!std::is_same<PatternTree, void>::value, NodeAddress>::type
    map_to_node(
        size_t total, size_t index, const int Level = 0, const int Offset = 0)
{
  using Configuration = typename get_config<PatternTree>::type;
  using Projection    = typename get_projection<Configuration>::type;
  using Left          = typename get_left<PatternTree>::type;
  using Right         = typename get_right<PatternTree>::type;

  constexpr auto correct_entity =
      std::is_same<Entity, typename get_entity_t<Configuration>::type>::value;

  if(correct_entity) {
    return NodeAddress{Level, Offset};
  } else {
    if(Projection::assignment(total, index) == Assignment::Left) {
      return ElementMapping<Left>::template map_to_node<Entity>(total, index, Level + 1, Offset * 2);
    } else {
      return ElementMapping<Right>::template map_to_node<Entity>(
          total, index, Level + 1, Offset * 2 + 1);
    }
  }
};

template <typename Entity = void>
constexpr inline typename std::enable_if<std::is_same<PatternTree, void>::value, NodeAddress>::type
    map_to_node(size_t total, const size_t, const int, const int)
{
  return NodeAddress{-1, -1};
}
};

#endif

