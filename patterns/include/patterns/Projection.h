#ifndef PATTERNS_PROJECTION_H
#define PATTERNS_PROJECTION_H

#include <cstddef>

enum struct Assignment {
  Left,
  Right,
  Here
};

struct RootProjection {
  static constexpr Assignment assignment(const size_t total, const size_t idx) {
    return idx < (total / 2) ? Assignment::Left : Assignment::Right;
  };

  static constexpr size_t offset(const size_t idx)
  {
    return idx;
  }
};

template <typename Parent>
struct IdProjection {
  typedef Parent parent_t;

  static constexpr Assignment assignment(const size_t total, const size_t idx) {
    return idx < (total / 2) ? Assignment::Left : Assignment::Right;
  };

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
