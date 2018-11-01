#ifndef PATTERNS__ENTITY_PATTERN_H
#define PATTERNS__ENTITY_PATTERN_H

// TODO: removeme
#include <array>
#include "TypeTraits.h"

template<typename Index, size_t Dim>
using SizeSpec_t = std::array<Index, Dim>;

template<size_t Dim, typename Index>
using TeamSpec = std::array<Index, Dim>;

template<size_t Dim>
using DistributionSpec = std::array<size_t, Dim>;

// -----------------

template<typename ...EntityPatterns>
class EntityPattern : public EntityPatterns... {
};




template<int Dim, typename Entity, typename Pattern>
class ProxyPattern {
private:
  using IndexType  = size_t;
  using TeamType   = typename team<Entity>::type;
  using CoordsType = std::array<IndexType, Dim>;

public:
  using entity_t  = Entity;
  using pattern_t = Pattern;

public:
  ProxyPattern(
      /// Pattern size (extent, number of elements) in every dimension
      const SizeSpec_t<size_t, Dim>& sizespec,
      /// Distribution type (BLOCKED, CYCLIC, BLOCKCYCLIC, TILE or NONE) of
      /// all dimensions.
      const DistributionSpec<Dim> dist,
      /// Cartesian arrangement of units within the team
      const TeamSpec<Dim, size_t> team)
    : pattern(sizespec, dist, team)
  {
  }

  IndexType lbegin(Entity &e) const {
    return pattern.lbegin();
  }

  IndexType lend() const {
    return pattern.lend();
  }

  TeamType unit_at(const CoordsType& coords, const CoordsType& viewspec) const
  {
    pattern.unit_at(coords, viewspec);
  }

  TeamType unit_at(const CoordsType& coords) const
  {
    pattern.unit_at(coords);
  }
private:

  Pattern pattern;
};

#endif

