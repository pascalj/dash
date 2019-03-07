#ifndef PATTERNS__TESTS__BLOCKED_BASE_PATTERN_H
#define PATTERNS__TESTS__BLOCKED_BASE_PATTERN_H

#include <array>
#include <cstddef>

template<std::size_t Dim>
struct ViewSpec {
  using ViewType = std::array<std::size_t, Dim>;
  ViewType _extents;
  ViewType _offsets;

  ViewSpec() = delete;

  ViewType extents() {
    return _extents;
  }

  ViewType offsets() {
    return _offsets;
  }

};

template<std::size_t Dim>
class BlockedBasePattern {
public:
  BlockedBasePattern(ViewSpec<Dim> &blocks)
    : _local_blocks(blocks)
  {
  }

  ViewSpec<Dim> local_blockspec() {
  }

  constexpr static size_t ndim() {
    return Dim;
  }

private:
  ViewSpec<Dim> _local_blocks;
};

#endif
