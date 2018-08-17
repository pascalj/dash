#ifndef PATTERNS__PATTERN_TREE_H
#define PATTERNS__PATTERN_TREE_H

#include "Projection.h"
#include "Configuration.h"
#include <type_traits>

template <typename ConfigurationT, typename Left, typename Right>
class PatternNode {
public:
  using config_t = ConfigurationT;
  using left_t   = Left;
  using right_t  = Right;

private:
  ConfigurationT *config;
};

template <typename Configuration>
using PatternLeaf = PatternNode<Configuration, void, void>;

template<typename Node>
struct get_left {
  typedef void type;
};

template<typename Left, typename Right, typename Config>
struct get_left<PatternNode<Config, Left, Right>> {
  typedef Left type;
};

template<typename Node>
struct get_right {
  typedef void type;
};

template<typename Left, typename Right, typename Config>
struct get_right<PatternNode<Config, Left, Right>> {
  typedef Right type;
};

template <typename T>
struct split;
template <typename Configuration>
struct split<PatternNode<Configuration, void, void>> {
  using type = PatternNode<
      Configuration,
      PatternLeaf<Configuration>,
      PatternLeaf<Configuration>>;
};

template <typename Node>
struct replace_left;

template <typename Configuration, typename Left, typename Right>
struct replace_left<PatternNode<Configuration, Left, Right>> {
  template <typename NewLeft>
  struct with {
    typedef PatternNode<Configuration, NewLeft, Right> type;
  };
};

template <typename Node>
struct replace_right;

template <typename Configuration, typename Left, typename Right>
struct replace_right<PatternNode<Configuration, Left, Right>> {
  template <typename NewRight>
  struct with {
    typedef PatternNode<Configuration, Left, NewRight> type;
  };
};

template <typename Node>
struct split_left;

template <typename Configuration, typename Left, typename Right>
struct split_left<PatternNode<Configuration, Left, Right>> {
  typedef PatternNode<Configuration, typename split<Left>::type, Right> type;
};

template <typename Node>
struct split_right;

template <typename Configuration, typename Left, typename Right>
struct split_right<PatternNode<Configuration, Left, Right>> {
  typedef PatternNode<Configuration, Left, typename split<Right>::type> type;
};

template <typename Node>
struct replace_config_t;

template <typename ConfigurationT, typename Left, typename Right>
struct replace_config_t<PatternNode<ConfigurationT, Left, Right>> {
  template <typename NewConfigurationT>
  struct with {
    typedef PatternNode<NewConfigurationT, Left, Right> type;
  };
};


template <typename Node>
struct get_config_t;

template <typename ConfigurationT, typename Left, typename Right>
struct get_config_t<PatternNode<ConfigurationT, Left, Right>> {
  typedef ConfigurationT type;
};

template <typename Node>
struct get_node;

template <typename ConfigurationT, typename Left, typename Right>
struct get_node<PatternNode<ConfigurationT, Left, Right>> {
  template <int Level, int Offset>
  struct at {
    typedef typename get_node<PatternNode<ConfigurationT, Left, Right>>::
        template at<Level - 1, Offset / 2>::type type;
  };

  template <int Offset>
  struct at<0, Offset> {
    typedef typename std::conditional<Offset == 0, Left, Right>::type type;
  };
};


template<typename T>
struct true_pred : std::true_type { };

template <typename Node, typename P = true_pred<Node>>
struct number_of_nodes;

template <
    typename ConfigurationT,
    typename Left,
    typename Right,
    template <class Node> class P>
struct number_of_nodes<
    PatternNode<ConfigurationT, Left, Right>,
    P<PatternNode<ConfigurationT, Left, Right>>> {
  using NodeT                     = PatternNode<ConfigurationT, Left, Right>;

  static constexpr int leftCount  = number_of_nodes<Left, P<Left>>::value;
  static constexpr int rightCount = number_of_nodes<Right, P<Right>>::value;
  static constexpr int nodeCount  = P<NodeT>::value ? 1 : 0;
  static const int     value      = nodeCount + leftCount + rightCount;
};

template <typename ConfigurationT, typename Left, template <class> class P>
struct number_of_nodes<
    PatternNode<ConfigurationT, Left, void>,
    P<PatternNode<ConfigurationT, Left, void>>> {
  using NodeT                    = PatternNode<ConfigurationT, Left, void>;

  static constexpr int leftCount = number_of_nodes<Left, P<Left>>::value;
  static constexpr int nodeCount = P<NodeT>::value ? 1 : 0;
  static const int     value     = nodeCount + leftCount;
};

template <typename ConfigurationT, typename Right, template <class> class P>
struct number_of_nodes<
    PatternNode<ConfigurationT, void, Right>,
    P<PatternNode<ConfigurationT, void, Right>>> {
  using NodeT                     = PatternNode<ConfigurationT, void, Right>;

  static constexpr int rightCount = number_of_nodes<Right, P<Right>>::value;
  static constexpr int nodeCount  = P<NodeT>::value ? 1 : 0;
  static constexpr int value      = nodeCount + rightCount;
};

template <typename ConfigurationT, template <class> class P>
struct number_of_nodes<
    PatternNode<ConfigurationT, void, void>,
    P<PatternNode<ConfigurationT, void, void>>> {
  using NodeT                = PatternNode<ConfigurationT, void, void>;

  static constexpr int value = P<NodeT>::value ? 1 : 0;
};


// Finding nodes
/* template<template <class> class P, int N = 1> */
/* struct find_node { */
/*   template<typename Node, int Level = 0, int Offset = 0> */
/*     struct in_tree; */

/*   template <typename ConfigurationT, typename Left, typename Right, int Level, int Offset> */
/*   static int in_tree<PatternNode<ConfigurationT, Left, Right>> { */
/*     using NodeT = PatternNode<ConfigurationT, Left, Right>; */
/*     using LeftSearch = */
/*         typename find_node<P>::template in_tree<Left, Level, Offset>; */

/*     static constexpr bool inLeftSubtree = number_of_nodes<Left, P<Left>>::value > N; */
/*     static constexpr int  leftLevel     = LeftSearch::level; */
/*     static constexpr int  leftOffset    = LeftSearch::offset; */

/*     static constexpr int level  = P<NodeT>::value ? Level : 0; */
/*     static constexpr int offset = P<NodeT>::value ? Offset : 0; */
/*   }; */

/* }; */


#endif
