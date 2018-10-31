#ifndef PATTERNS__NODE_H
#define PATTERNS__NODE_H

#include <cassert>
#include <cstdlib>
#include <iostream>
#include "TypeTraits.h"

using std::size_t;

template <typename Entity, typename Left, typename Right>
class PatternNode;
template <typename Entity>
class LeafNode;

template <typename Entity>
struct entities_in_node<LeafNode<Entity>, Entity>
  : public std::integral_constant<size_t, 1> {
};

template <typename Node, typename Entity>
bool linear_node_map(const Entity &e)
{
  auto left = entities_in_node<typename get_left<Node>::type, Entity>::value;
  return e.index < left;
};

template <typename Entity, typename Left, typename Right>
class PatternNode {
public:
  PatternNode(const size_t total, const size_t offset)
    : _total(total)
    , _offset(offset)
  {
  }
  PatternNode(
      const size_t total,
      const size_t offset,
      const Left & left,
      const Right &right)
    : _total(total)
    , _offset(offset)
    , left(left)
    , right(right)
  {
  }

  template <typename OtherEntity>
  constexpr size_t left_total(const OtherEntity &e)
  {
    return left.total(e);
  }

  template <typename OtherEntity>
  constexpr size_t right_total(const OtherEntity &e)
  {
    return right.total(e);
  }

  constexpr size_t begin(const Entity &e)
  {
    return _offset;
  };

  template <typename OtherEntity>
  constexpr size_t begin(const OtherEntity &e)
  {
    auto use_left = linear_node_map<PatternNode, OtherEntity>(e);
    if (use_left) {
      return left.begin(OtherEntity(e.index / 2, e.total));
    }
    else {
      return right.begin(OtherEntity(e.index / 2, e.total));
    }
  };

  constexpr size_t total(const Entity &e)
  {
    return _total;
  }
  template <typename OtherEntity>
  constexpr size_t total(const OtherEntity &e)
  {
    auto use_left = linear_node_map<PatternNode, OtherEntity>(e);
    if (use_left) {
      return left.total(e);
    }
    else {
      return right.total(e);
    }
    return 0;
  }

  constexpr size_t offset(const Entity &e)
  {
    return _offset;
  }

  template <typename OtherEntity>
  constexpr size_t offset(const OtherEntity &e)
  {
    return _offset;
  }

protected:
  size_t _total  = 0;
  size_t _offset = 0;
  Left   left;
  Right  right;
};


class NullNode {
};


template <typename Entity>
class LeafNode : public PatternNode<Entity, NullNode, NullNode> {
public:
  LeafNode(const size_t total, const size_t offset)
    : PatternNode<Entity, NullNode, NullNode>(total, offset)
  {
  }

  const size_t left_total() const
  {
    assert(0);
    return 0;
  }

  const size_t right_total() const
  {
    assert(0);
    return 0;
  }
};

template <typename Entity, typename Left, typename Right>
class BalancedNode : public PatternNode<Entity, Left, Right> {
public:
  BalancedNode(const size_t total, const size_t offset)
    : PatternNode<Entity, Left, Right>(
          total,
          offset,
          Left(total / 2, offset),
          Right(total / 2, offset + total / 2))
  {
  }
};

#endif
