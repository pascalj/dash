#ifndef PATTERNS__ENTITY_MAPPING_H
#define PATTERNS__ENTITY_MAPPING_H

#include "Entity.h"
#include "Configuration.h"
#include "PatternTree.h"
#include "ElementMapping.h"
#include <type_traits>

template <typename Node>
struct is_process {
  static constexpr bool value =
      std::is_same<Process, typename get_entity_t<Node>::type>::value;
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
    using Nodes = typename find_node<is_process>::in_tree<PatternTree, 0, 0>::type;
    using MatchingNodes = location_size<Nodes>;

    static_assert(MatchingNodes::value > 0, "No matching nodes in the pattern tree");

    auto level = levels<Nodes>::value;
    auto offset = offsets<Nodes>::value;
    auto idx = p.index % MatchingNodes::value;
    return RuntimeLocation{level[idx], offset[idx]};
  }

  static const int entities_on_node(const Process &p) {
    using Nodes = typename find_node<is_process>::in_tree<PatternTree, 0, 0>::type;
    using MatchingNodes = location_size<Nodes>;

    static_assert(MatchingNodes::value > 0, "No matching nodes in the pattern tree");

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
   * address the node in a tree.
   */
  const RuntimeLocation map(const Process &p)
  {
    using Nodes = typename find_node<is_process>::in_tree<PatternTree, 0, 0>::type;
    using MatchingNodes = location_size<Nodes>;

    static_assert(MatchingNodes::value > 0, "No matching nodes in the pattern tree");

    auto level = levels<Nodes>::value;
    auto offset = offsets<Nodes>::value;
    auto per_node = entities_on_node(p);
    auto idx = p.index / per_node;
    return RuntimeLocation{level[idx], offset[idx]};
  }

  static const int entities_on_node(const Process &p) {
    using Nodes = typename find_node<is_process>::in_tree<PatternTree, 0, 0>::type;
    using MatchingNodes = location_size<Nodes>;

    static_assert(MatchingNodes::value > 0, "No matching nodes in the pattern tree");
    auto per_node = p.total / MatchingNodes::value + (p.total % MatchingNodes::value > 0 ? 1 : 0);
    // TODO: edge cases
    return per_node;
  };

  static const int position_on_node(const Process &p) {
    using Nodes = typename find_node<is_process>::in_tree<PatternTree, 0, 0>::type;
    using MatchingNodes = location_size<Nodes>;

    static_assert(MatchingNodes::value > 0, "No matching nodes in the pattern tree");
    auto per_node = p.total / MatchingNodes::value + (p.total % MatchingNodes::value > 0 ? 1 : 0);
    auto offset = p.index % per_node;
    // TODO: edge cases
    return offset;
  }
};
#endif
