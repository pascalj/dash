#ifndef PATTERNS__TYPE_TRAITS_H
#define PATTERNS__TYPE_TRAITS_H

#include <cstdlib>
#include <type_traits>

using std::size_t;


// Nodes
template <typename Node>
struct entity_type {
};


template <typename Node>
struct get_left {
};

template <typename Node>
struct get_right {
};

template <typename Node, typename Entity, typename SFINEAE = void>
struct entities_in_node : std::integral_constant<size_t, 0> {
};

template <
    template <class, class, class> class Node,
    typename Entity,
    typename Left,
    typename Right>
struct entity_type<Node<Entity, Left, Right>> {
  using type = Entity;
};

template <
    template <class> class Node,
    typename Entity>
struct entity_type<Node<Entity>> {
  using type = Entity;
};

template <
    template <class, class, class> class Node,
    class Entity,
    class Left,
    class Right>
struct get_left<Node<Entity, Left, Right>> {
  using type = Left;
};

template <
    template <class, class, class> class Node,
    class Entity,
    class Left,
    class Right>
struct get_right<Node<Entity, Left, Right>> {
  using type = Right;
};

template <typename Node, typename Entity>
struct entities_in_node<Node, Entity> {
  using left                     = typename get_left<Node>::type;
  using right                    = typename get_right<Node>::type;
  using left_count               = entities_in_node<left, Entity>;
  using right_count              = entities_in_node<right, Entity>;
  using node_entity              = typename entity_type<Node>::type;
  using entity_count             = typename std::conditional<
      std::is_same<Entity, node_entity>::value,
      std::integral_constant<size_t, 1>,
      std::integral_constant<size_t, 0>>;
  static const size_t value =
      entity_count::value + left_count::value + right_count::value;
};

template <template <class> class Node, typename NodeEntity, typename Entity>
struct entities_in_node<Node<NodeEntity>, Entity> {
  using entity_count = typename std::conditional<
      std::is_same<Entity, NodeEntity>::value,
      std::integral_constant<size_t, 1>,
      std::integral_constant<size_t, 0>>;
  static const size_t value = entity_count::value;
};

#endif

