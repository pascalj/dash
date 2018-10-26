#ifndef PATTERNS__ENTITY_MAPPING_H
#define PATTERNS__ENTITY_MAPPING_H

#include "Entity.h"
#include "Configuration.h"
#include "PatternTree.h"
#include "ElementMapping.h"
#include <type_traits>

template <typename EntityType>
struct with_entity_type {
  template <typename Node>
  struct type {
    static const bool value =
        std::is_same<EntityType, typename get_entity_t<Node>::type>::value;
    };
};

template<class Node>
using is_process = with_entity_type<Process>::type<Node>;

template <typename EntityType, typename Tree>
struct nodes_with_entity_type {
  using entities = with_entity_type<EntityType>;
  using nodes   = typename find_node<
      entities::template type>::template in_tree<Tree>::type;
  using node_count       = location_size<nodes>;
  const static int count = node_count::value;
};

/**
 * The RoundRobinEntityMapping maps entities (e.g. processes) round robin to
 * nodes. No weights or distances are considered. However, the assigned entity
 * type is of course respected.
 *
 */
template <typename PatternTree>
class RoundRobinEntityMapping {
  using tree_t = PatternTree;

public:
  /**
   * Map a process to a node. It returns a NodeAddr that can be used to
   * address the node in a tree.
   */
  const RuntimeLocation map(const Process &p)
  {
    using MatchingNodes = nodes_with_entity_type<Process, PatternTree>;
    using Nodes = typename MatchingNodes::nodes;

    static_assert(MatchingNodes::count > 0, "No matching nodes in the pattern tree");

    auto level = levels<Nodes>::value;
    auto offset = offsets<Nodes>::value;
    auto idx = p.index % MatchingNodes::count;
    return RuntimeLocation{level[idx], offset[idx]};
  }

  template<typename EntityType>
  static const int entities_on_node(const EntityType &p) {
    using MatchingNodes = nodes_with_entity_type<EntityType, PatternTree>;

    static_assert(MatchingNodes::count > 0, "No matching nodes in the pattern tree");

    // TODO: edge cases
    return p.total / MatchingNodes::value;
  };
};

template <typename PatternTree>
class LinearEntityMapping {
  using tree_t = PatternTree;

public:
  /** 
   * Map a process to a node. It returns a NodeAddr that can be used to
   * address the node in a tree.*/
  template<typename EntityType>
  const RuntimeLocation map(const EntityType &p)
  {
    using MatchingNodes = nodes_with_entity_type<EntityType, PatternTree>;
    using Nodes = typename MatchingNodes::nodes;

    static_assert(MatchingNodes::count > 0, "No matching nodes in the pattern tree");

    auto level = levels<Nodes>::value;
    auto offset = offsets<Nodes>::value;
    auto per_node = entities_on_node(p);
    auto idx = p.index / per_node;
    return RuntimeLocation{level[idx], offset[idx]};
  }

  template<class EntityType>
  static const int entities_on_node(const EntityType &p) {
    using MatchingNodes = nodes_with_entity_type<EntityType, PatternTree>;
    using Nodes = typename MatchingNodes::nodes;
    /* static_assert(entity_nodes::count > 0, "No matching nodes in the pattern tree"); */
    auto per_node = p.total / MatchingNodes::count + (p.total % MatchingNodes::count > 0 ? 1 : 0);
    // TODO: edge cases
    return per_node;
  };

  template<class EntityType>
  static const int position_on_node(const EntityType &p) {
    using MatchingNodes = nodes_with_entity_type<EntityType, PatternTree>;
    using Nodes = typename MatchingNodes::nodes;

    static_assert(MatchingNodes::count > 0, "No matching nodes in the pattern tree");
    auto per_node = p.total / MatchingNodes::count + (p.total % MatchingNodes::count > 0 ? 1 : 0);
    auto offset = p.index % per_node;
    // TODO: edge cases
    return offset;
  }
};
#endif
