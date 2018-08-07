#ifndef PATTERNS__PATTERN_TREE_H
#define PATTERNS__PATTERN_TREE_H

#include <type_traits>


struct Entity {};

template <typename EntityT, typename Left, typename Right>
class PatternNode {
public:
  using entity_t = Entity;
  using  left_t = Left;
  using  right_t = Right;
private:
  EntityT *entity;
};

template<typename Entity>
using PatternLeaf = PatternNode<Entity, void, void>;

template<typename T>  struct split;
template<typename Entity>
struct split<PatternNode<Entity, void, void>> {
  using type = PatternNode<Entity, PatternLeaf<Entity>, PatternLeaf<Entity>>;
};

template<typename Node>
struct replace_left;

template<typename Entity, typename Left,typename Right>
struct replace_left<PatternNode<Entity, Left, Right>> {
  template<typename NewLeft> struct with {
    typedef PatternNode<Entity, NewLeft, Right> type;
  };
};

template<typename Node>
struct replace_right;

template<typename Entity, typename Left,typename Right>
struct replace_right<PatternNode<Entity, Left, Right>> {
  template<typename NewRight> struct with {
    typedef PatternNode<Entity, Left, NewRight> type;
  };
};

template<typename Node>
struct split_left;

template<typename Entity, typename Left, typename Right>
struct split_left<PatternNode<Entity, Left, Right>> {
  typedef PatternNode<Entity, split<Left>, Right> type;
};

template<typename Node>
struct split_right;

template<typename Entity, typename Left, typename Right>
struct split_right<PatternNode<Entity, Left, Right>> {
  typedef PatternNode<Entity, Left, split<Right>> type;
};

template <typename Node>
struct replace_entity_t;

template <typename EntityT, typename Left, typename Right>
struct replace_entity_t<PatternNode<EntityT, Left, Right>> {
  template<typename NewEntityT> struct with {
    typedef PatternNode<NewEntityT, Left, Right> type;
  };
};


template <int Level,int Offset>
struct Pos {
  static const int level = Level;
  static const int offset = Offset;
};

template<typename Pos>
struct get_node;

template<typename EntityT, typename Left, typename Right>
struct get_node<PatternNode<EntityT, Left, Right>> {
  template<int Level, int Offset>
  struct at {
    typedef typename get_node<PatternNode<EntityT, Left, Right>>::
        template at<Level - 1, Offset / 2>::type type;
  };

  template<int Offset>
  struct at<0, Offset> {
    typedef typename std::conditional<Offset == 0, Left, Right>::type type;
  };
};

using root = split<PatternLeaf<Entity>>::type;
using level1 = replace_entity_t<replace_left<root>::with<PatternLeaf<float>>::type>::with<int>::type;
using level2 = replace_left<level1>::with<split<level1::left_t>::type>::type;

#endif
