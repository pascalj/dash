#ifndef PATTERNS__LOCAL_PATTERN_H
#define PATTERNS__LOCAL_PATTERN_H

#include <cstddef>
#include <dash/Dimensional.h>
#include <dash/Cartesian.h>
#include <dash/LocalArray.h>

namespace patterns {

template<int NumDimensions, typename Base, typename Entity>
class RoundRobinLocalPattern : public Base {
public:
  RoundRobinLocalPattern(dash::SizeSpec<NumDimensions> size_spec, Entity entity)
    : Base(size_spec), _entity(entity)
  {
    for (auto block : this->local_blockspec().extents()) {
      _local_blocks++;
    }
  }

  dash::ViewSpec<NumDimensions> block_for_entity(Entity entity, size_t index)
  {
    auto block_index = index * _entity.total() + entity.index();
    return this->local_block(block_index);
  }

  dash::ViewSpec<NumDimensions> block_local_for_entity(Entity entity, size_t index)
  {
    auto block_index = index * _entity.total() + entity.index();
    return this->local_block_local(block_index);
  }

  size_t blocks_for_entity(Entity entity) {
    auto const entity_blocks = _local_blocks / _entity.total();
    auto const add =
        _local_blocks % _entity.total() < _entity.index() ? 0 : 1;
    return entity_blocks + add;
  }

  size_t lbegin(Entity entity) {
    auto first_block = local_block_local(entity.index());
    this->local_at(first_block.offsets());
  }

  size_t lend(Entity entity) {
    auto last_block = local_block_local(
        _local_blocks - (_local_blocks % entity.total()) + entity.index());
    this->local_at(last_block .offsets()) + last_block.size();
  }

private:
  Entity _entity;
  size_t _local_blocks;
};

}


#endif
