#ifndef PATTERNS_BLOCK_RANGE_H
#define PATTERNS_BLOCK_RANGE_H

#include "block_iterator.h"

namespace patterns {

template <typename PatternT>
class BlockRange {
public:
  using entity_type  = typename PatternT::entity_type;
  using iterator     = EntityBlockIterator<PatternT>;
  using pattern_type = PatternT;

private:
  pattern_type* _pattern;
  entity_type*  _entity;

public:
  BlockRange(PatternT* pattern, entity_type* entity)
    : _pattern(pattern)
    , _entity(entity)
  {
  }

  FN_HOST_ACC EntityBlockIterator<PatternT> begin()
  {
    return iterator{_pattern, _entity};
  }

  FN_HOST_ACC EntityBlockIterator<PatternT> end()
  {
    return iterator{
        _pattern, _entity, _pattern->nblocks_for_entity(*_entity)};
  }
};
}  // namespace patterns

#endif
