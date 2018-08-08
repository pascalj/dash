#ifndef PATTERNS_PROJECTION_H
#define PATTERNS_PROJECTION_H

#include <cstddef>

struct RootProjection {
  static constexpr size_t offset(const size_t idx)
  {
    return idx;
  }
};

template <typename Parent>
struct IdProjection {
  typedef Parent parent_t;

  static constexpr size_t offset(const size_t idx)
  {
    return parent_t::offset(idx);
  }
};

template <typename Parent>
struct CyclicProjection {
  typedef Parent parent_t;

  static constexpr size_t offset(const size_t idx)
  {
    return parent_t::offset(idx);
  }
};

#endif
