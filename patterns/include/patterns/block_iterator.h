#ifndef PATTERNS_BLOCK_ITERATOR_H
#define PATTERNS_BLOCK_ITERATOR_H

#include <cstddef>
#include <iterator>
#include <dash/Types.h>

namespace patterns {
template <typename PatternT>
class EntityBlockIterator {
public:
  using block_type  = typename PatternT::block_type;
  using entity_type = typename PatternT::entity_type;

  using difference_type   = std::ptrdiff_t;
  using value_type        = block_type;
  using reference         = const value_type &;
  using pointer           = const value_type *;
  using iterator_category = std::forward_iterator_tag;

public:
  EntityBlockIterator(
      PatternT *pattern, entity_type *entity, size_t index = 0)
    : _pattern(pattern)
    , _entity(entity)
    , _index(index)
  {
  }

  FN_HOST_ACC EntityBlockIterator &operator++()
  {
    _index++;
    return *this;
  }

  FN_HOST_ACC EntityBlockIterator operator++(int)
  {
    EntityBlockIterator res(*this);
    ++(*this);
    return res;
  }

  FN_HOST_ACC reference operator*()
  {
    _blockspec = _pattern->block_local_for_entity(*_entity, _index);
    return _blockspec;
  }

  FN_HOST_ACC bool operator!=(EntityBlockIterator &other) {
    return _index != other._index;
  }

private:
  PatternT *   _pattern = nullptr;
  entity_type *_entity  = nullptr;
  size_t       _index;
  // TODO: find a better solution for this, operator* should be const
  value_type   _blockspec;
};
}  // namespace patterns

#endif
