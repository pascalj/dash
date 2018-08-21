#ifndef PATTERNS__NODE_MAPPING_H
#define PATTERNS__NODE_MAPPING_H

#include <cstddef>
#include <cmath>
#include <type_traits>
#include "Configuration.h"
#include "PatternTree.h"
#include "Projection.h"

template <typename PatternTree, typename EntityMapping>
struct BalancedElementMapping {

  template<typename Entity>
  static constexpr size_t left_offset(Entity &e, const size_t total)
  {
    return EntityMapping::position_on_node(e) * total;
  };

  template<typename Entity>
  static constexpr size_t right_offset(Entity &e, const size_t total)
  {
    return left_count(e, total);
  };


  template<typename Entity>
  static constexpr size_t left_count(Entity &e, const size_t total)
  {
    return std::ceil(total * 0.5);
  };

  template<typename Entity>
  static constexpr size_t right_count(Entity &e, const size_t total)
  {
    return std::floor(total * 0.5);
  };

  template<typename Entity>
  static constexpr size_t offset(Entity &e, const RuntimeLocation loc, const size_t total) {
    if (loc.level == 0) {
      return 0;
    };
    if (loc.level > loc.offset) {
      return left_offset(e, total) +
             BalancedElementMapping<typename get_left<PatternTree>::type, EntityMapping>::
                 offset(e, loc.next(), left_count(e, total));
    } else {
      return right_offset(e, total) +
             BalancedElementMapping<typename get_right<PatternTree>::type, EntityMapping>::
                 offset(e, loc.next(), right_count(e, total));
    }
  }

  template<typename Entity>
  static constexpr size_t count(Entity &e, const RuntimeLocation loc, const size_t total) {
    if (loc.level == 0) {
      return total / EntityMapping::entities_on_node(e);
    };
    if (loc.level > loc.offset) {
      return BalancedElementMapping<typename get_left<PatternTree>::type, EntityMapping>::
                 count(e, loc.next(), left_count(e, total));
    } else {
      return BalancedElementMapping<typename get_right<PatternTree>::type, EntityMapping>::
                 count(e, loc.next(), right_count(e, total));
    }
  }
};


template <typename Mapping>
struct get_count;

template <template <class> class Mapping, typename Node>
struct get_count<Mapping<Node>> {
  template<int Level, int Offset> struct at {
    static constexpr bool descent_left = Level < Offset;

    static constexpr size_t count(
        const size_t total)
    {
      using Left = typename get_left<Node>::type;

      using Right = typename get_right<Node>::type;
      // Traverse the tree down to the specified node via
      // Level and Offset. This reduces the number of elements
      // in each step by getting the count of the child node and
      // starting over.
      return descent_left ? get_count<Mapping<Left>>::
                                template at<Level - 1, Offset / 2>::count(
                                    Mapping<Node>::left_count(total))
                          : get_count<Mapping<Right>>::
                                template at<Level - 1, Offset / 2>::count(
                                    Mapping<Node>::right_count(total));
    };
  };

  template<int Offset> struct at<0, Offset> {
    static constexpr size_t count(const size_t total) {
      return total;
    }
  };
};

template <typename Mapping>
struct get_offset;

template <template <class> class Mapping, typename Node>

struct get_offset<Mapping<Node>> {
  template<int Level, int Offset> struct at {
    static constexpr bool descent_left = Level < Offset;

    static constexpr size_t offset(
        const size_t total)
    {
      if (Level == 0) {
        return 0;
      } else {
      using Left = typename get_left<Node>::type;
      using Right = typename get_right<Node>::type;
      // Traverse the tree down to the specified node via
      // Level and Offset. This reduces the number of elements
      // in each step by getting the count of the child node and
      // starting over.
      auto left_offset = Mapping<Node>::left_offset(total);
      auto right_offset = Mapping<Node>::right_offset(total);
      return descent_left
                 ? get_offset<Mapping<Left>>::
                           template at<Level - 1, Offset / 2>::offset(
                               left_offset) + left_offset
                 : get_offset<Mapping<Right>>::
                       template at<Level - 1, Offset / 2>::offset(
                           right_offset) + right_offset;
      }
    };
  };

  template<int Offset> struct at<0, Offset> {
    static constexpr size_t offset(
        const size_t total)
    {
        return 0;
    };
  };
};

/* template <typename PatternTree> */
/* struct ElementMapping { */

/* template <typename Entity = void> */
/* constexpr inline typename std::enable_if<!std::is_same<PatternTree, void>::value, NodeAddress>::type */
/*     map_to_node( */
/*         size_t total, size_t index, const int Level = 0, const int Offset = 0) */
/* { */
/*   using Configuration = typename get_config<PatternTree>::type; */
/*   using Projection    = typename get_projection<Configuration>::type; */
/*   using Left          = typename get_left<PatternTree>::type; */
/*   using Right         = typename get_right<PatternTree>::type; */

/*   constexpr auto correct_entity = */
/*       std::is_same<Entity, typename get_entity_t<Configuration>::type>::value; */

/*   if(correct_entity) { */
/*     return NodeAddress{Level, Offset}; */
/*   } else { */
/*     if(Projection::assignment(total, index) == Assignment::Left) { */
/*       return ElementMapping<Left>::template map_to_node<Entity>(total, index, Level + 1, Offset * 2); */
/*     } else { */
/*       return ElementMapping<Right>::template map_to_node<Entity>( */
/*           total, index, Level + 1, Offset * 2 + 1); */
/*     } */
/*   } */
/* }; */

/* template <typename Entity = void> */
/* constexpr inline typename std::enable_if<std::is_same<PatternTree, void>::value, NodeAddress>::type */
/*     map_to_node(size_t total, const size_t, const int, const int) */
/* { */
/*   return NodeAddress{-1, -1}; */
/* } */
/* }; */

#endif

