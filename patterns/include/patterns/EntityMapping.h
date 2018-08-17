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
  constexpr const int map(Process &p) const
  {
    return p.index % p.total;
  }
};
#endif
