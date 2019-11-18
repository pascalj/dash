#ifndef PATTERNS__TESTS__BLOCKED_BASE_PATTERN_H
#define PATTERNS__TESTS__BLOCKED_BASE_PATTERN_H

#include <array>
#include <cstddef>

template<std::size_t Dim>
struct ViewSpec {
  using ViewType = std::array<std::size_t, Dim>;

  ViewSpec(ViewType extents, ViewType offsets)
    : _extents(extents)
    , _offsets(offsets)
  {
  }


  ViewType extents() {
    return _extents;
  }

  ViewType offsets() {
    return _offsets;
  }

  ViewType _extents;
  ViewType _offsets;
};

template<std::size_t Dim>
class BlockedBasePattern {
public:
  BlockedBasePattern(ViewSpec<Dim> &blocks)
    : _local_blocks(blocks)
  {
  }

  ViewSpec<Dim> local_blockspec() {
    return _local_blocks;
  }

  constexpr static size_t ndim() {
    return Dim;
  }

private:
  ViewSpec<Dim> _local_blocks;
};

#endif
