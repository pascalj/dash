#ifndef PATTERNS__PATTERN_TREE_H
#define PATTERNS__PATTERN_TREE_H

#include "Projection.h"
#include "Configuration.h"
#include <type_traits>
#include <iostream>
#include <array>

template<typename T>
struct true_pred : std::true_type { };

template <typename Node, typename P = true_pred<Node>>
struct number_of_nodes;

template <typename Entity, typename Node>
struct is_entity {
  static constexpr bool value =
      std::is_same<Entity, typename get_entity_t<Node>::type>::value;
};

template <typename ConfigurationT, typename Left, typename Right>
class PatternNode {
public:
  using config_t = ConfigurationT;
  using left_t   = Left;
  using right_t  = Right;
  using total_nodes = number_of_nodes<PatternNode>;

private:
  ConfigurationT *config;
  Left *left;
  Right *right;
};

template <typename Configuration>
using PatternLeaf = PatternNode<Configuration, void, void>;

template<int Level, int Offset>
struct Location {
  using level = std::integral_constant<int, Level>;
  using offset = std::integral_constant<int, Offset>;
};

// Simple list of locations in the tree
// @see http://pdimov.com/cpp2/simple_cxx11_metaprogramming.html
template<typename... Locs>
struct Locations {};

// Size of the location list
template <typename S>
struct location_size_impl;
template <class... Locs>
struct location_size_impl<Locations<Locs...>> {
  using type = std::integral_constant<std::size_t, sizeof...(Locs)>;
};
template<class Locs> using location_size = typename location_size_impl<Locs>::type;

// Append location lists, useful for searching the tree
template<class... L> struct location_append_impl;

template<class... L> using location_append = typename location_append_impl<L...>::type;

template<> struct location_append_impl<>
{
    using type = Locations<>;
};

template<template<class...> class L, class... T> struct location_append_impl<L<T...>>
{
    using type = L<T...>;
};

template<template<class...> class L1, class... T1,
    template<class...> class L2, class... T2, class... Lr>
    struct location_append_impl<L1<T1...>, L2<T2...>, Lr...>
{
    using type = location_append<L1<T1..., T2...>, Lr...>;
};

template<int N, class Loc, class... Locs> struct location_get {
  using type = typename location_get<N - 1, Locs...>::type;
};

template<typename T, typename... Locs> struct location_get<0, T, Locs...> {
  using type = T;
};

struct RuntimeLocation {
  int level;
  int offset;

  RuntimeLocation left() const {
    return RuntimeLocation{level + 1, offset * 2};
  }

  RuntimeLocation right() const {
    return RuntimeLocation{level + 1, offset * 2 + 1};
  }

  RuntimeLocation next() const {
    return RuntimeLocation{level - 1, offset / 2};
  };
};

template<typename Loc>
struct get_level {
  static constexpr int level = 0;
};


template <int Size>
struct RuntimeLocations {
  const RuntimeLocation locations[Size];

  constexpr RuntimeLocations(std::array<int, Size> levels, std::array<int, Size> offsets) {
    for(int i = 0; i < Size; i++) {
      locations[i].level = levels[i];
      locations[i].offset = offsets[i];
    }

  }

  RuntimeLocation& operator[](size_t idx) {
    return locations[idx];
  };
};

template<typename...Locs>
struct levels;

template <typename...Locs>
struct levels<Locations<Locs...>>
{
  static constexpr std::array<int, sizeof...(Locs)> value = { { Locs::level::value... } };
};

template<typename...Locs>
struct offsets;
template <typename...Locs>
struct offsets<Locations<Locs...>>
{
  static constexpr std::array<int, sizeof...(Locs)> value = { { Locs::offset::value... } };
};


template <typename...Locs>
struct to_array { };


template <typename...Locs>
struct to_array<Locations<Locs...>>
{
  /* static constexpr RuntimeLocations<sizeof...(Locs)> value = RuntimeLocations<sizeof...(Locs)>(levels, offsets); */
  static RuntimeLocation get(const size_t idx) {
    return RuntimeLocation{levels<Locs...>::value[idx], offsets<Locs...>::value[idx]};
  }
};



/* template<typename... Locs> */
/* constexpr RuntimeLocations<sizeof...(Locs)> make_runtime_locs() { */
/*   return {{ Locs... }}; */
/* } */
/* template<int Level, int Offset> */
/* struct Location { */
/*   static constexpr int level = Level; */
/*   static constexpr int offset = Offset; */
/* }; */


std::ostream& operator<<(std::ostream& os, const RuntimeLocation& loc) {
  os << "RuntimeLocation{" << loc.level << ", " << loc.offset << "}" << std::endl;
  return os;
}

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


template <typename Node>
struct is_leaf {
  static constexpr bool value = false;
};

template <typename ConfigurationT>
struct is_leaf<PatternNode<ConfigurationT, void, void>> {
  static constexpr bool value = true;
};


// Finding nodes
template<template <class> class P>
struct find_node {
  template<typename Node, int Level = 0, int Offset = 0>
    struct in_tree {
      using type = Locations<>;
    };

  template <typename ConfigurationT, typename Left, typename Right, int Level, int Offset>
  struct in_tree<PatternNode<ConfigurationT, Left, Right>, Level, Offset> {
    using NodeT = PatternNode<ConfigurationT, Left, Right>;
    using NextLevel = std::integral_constant<int, Level + 1>;
    using LeftOffset = std::integral_constant<int, Offset * 2>;
    using RightOffset = std::integral_constant<int, Offset * 2 + 1>;
    using locations = typename std::conditional<
        P<NodeT>::value,
        Locations<Location<Level, Offset>>,
        Locations<>>::type;
    using left_locations = typename std::conditional<
        std::is_same<Left, void>::value,
        Locations<>,
        typename find_node<
            P>::template in_tree<Left, NextLevel::value, LeftOffset::value>::
            type>::type;
    using right_locations = typename std::conditional<
        std::is_same<Right, void>::value,
        Locations<>,
        typename find_node<
            P>::template in_tree<Right, NextLevel::value, RightOffset::value>::
            type>::type;

    using type = location_append<
        locations,
        left_locations,
        right_locations>;
  };
};


#endif
