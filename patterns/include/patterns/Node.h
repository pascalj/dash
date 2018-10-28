#ifndef PATTERNS__NODE_H
#define PATTERNS__NODE_H

#include <cstdlib>
#include <type_traits>
#include <iostream>
#include <cassert>

using std::size_t;

template<typename Node>
struct entity_type {};

template <typename Entity, typename Left, typename Right> class PatternNode;

template<typename Entity, typename Left, typename Right>
struct entity_type<PatternNode<Entity, Left, Right>> {
  using value = Entity;
};

template<typename Node, typename Entity>
struct entities_in_node {}; //: std::integral_constant<size_t, 0> {};

template<typename NodeType, typename Left, typename Right, typename Entity>
struct entities_in_node<PatternNode<NodeType, Left, Right>, Entity> {
  using node_t = PatternNode<NodeType, Left, Right>;
  using left_count = entities_in_node<Left, Entity>;
  using right_count = entities_in_node<Right, Entity>;
  static const auto entity_count = std::conditional<
      std::is_same<Entity, NodeType>::value,
      std::integral_constant<size_t, 1>,
      std::integral_constant<size_t, 0>>::value;
  static const size_t value =
      entity_count + left_count::value + right_count::value;
};

template<typename Node>
struct get_left {};

template<typename Entity, typename Left, typename Right>
struct get_left<PatternNode<Entity, Left, Right>> {
  using type = Left;
};

template<typename Node>
struct get_right {};

template<typename Entity, typename Left, typename Right>
struct get_right<PatternNode<Entity, Left, Right>> {
  using type = Right;
};

template<typename Node, typename Entity>
bool linear_node_map(const Entity &e) {
  auto left = entities_in_node<typename get_left<Node>::type, Entity>::value;
  auto right = entities_in_node<typename get_right<Node>::type, Entity>::value;
  std::cout << "Left: " << left << "Right: " << right << std::endl;
  return left < e.index;
};

template <typename Entity, typename Left, typename Right> class PatternNode {
public:
  PatternNode(const size_t total, const size_t offset)
      : _total(total), _offset(offset) {}
  PatternNode(const size_t total, const size_t offset, const Left &left,
              const Right &right)
      : _total(total), _offset(offset), left(left), right(right) {}

  template<typename OtherEntity>
  constexpr size_t left_total(const OtherEntity &e) {
    return left.total(e);
  }

  template<typename OtherEntity>
  constexpr size_t right_total(const OtherEntity &e) {
    return right.total(e);
  }

  constexpr size_t begin(const Entity &e) {
    return _offset;
  };

  template<typename OtherEntity>
  constexpr size_t begin(const OtherEntity &e) {
    // either left or right ?
    auto left_is_right = linear_node_map<PatternNode, OtherEntity>(e);
    if (left_is_right) {
      return left.begin(e);
    } else {
      return right.begin(e);
    }
  };

  constexpr size_t total(const Entity &e) { return _total; }
  template <typename OtherEntity> constexpr size_t total(const OtherEntity &e) {
    auto in_left = linear_node_map<PatternNode, OtherEntity>(e);
    if (in_left) {
      return left.total(e);
    } else {
      return right.total(e);
    }
    return 0;
  }

  constexpr size_t offset(const Entity &e) { return _offset; }

  template<typename OtherEntity>
  constexpr size_t offset(const OtherEntity &e) { return _offset; }

protected:
  Left left;
  Right right;
  size_t _total = 0;
  size_t _offset = 0;
};

class NullNode {
  constexpr size_t total() { return 0; }
};

template<typename Entity>
class LeafNode : public PatternNode<Entity, NullNode, NullNode> {
public:
  LeafNode(const size_t total, const size_t offset)
      : PatternNode<Entity, NullNode, NullNode>(total, offset) {}

  const size_t left_total() const {
    assert(0);
    return 0;
  }

  const size_t right_total() const {
    assert(0);
    return 0;
  }
};

template <typename Entity, typename Left, typename Right>
class BalancedNode : public PatternNode<Entity, Left, Right> {
public:
  BalancedNode(const size_t total, const size_t offset)
      : PatternNode<Entity, Left, Right>(total, offset, Left(total / 2, offset),
                                         Right(total / 2, offset + total / 2)) {
  }
};

#endif
