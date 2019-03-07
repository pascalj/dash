#ifndef PATTERNS__LOCAL_PATTERN_H
#define PATTERNS__LOCAL_PATTERN_H

#include <cstddef>

namespace patterns {

template <typename Base, typename Entity>
class RoundRobinLocalPattern : public Base {
  constexpr static size_t NumDimensions = Base::ndim();

public:
  RoundRobinLocalPattern() = delete;

  RoundRobinLocalPattern(Base base)
    : Base(base)
  {
    for (auto blockcount : this->local_blockspec().extents()) {
      _local_blocks *= blockcount;
    }
  }

  auto block_forentity(Entity entity, size_t index)
  {
    auto block_index = index * entity.total() + entity.index();
    return this->local_block(block_index);
  }

  auto block_local_forentity(Entity entity, size_t index)
  {
    auto block_index = index * entity.total() + entity.index();
    return this->local_block_local(block_index);
  }

  size_t blocks_for_entity(Entity entity)
  {
    auto const entity_blocks = _local_blocks / entity.total();
    auto const add = _local_blocks % entity.total() < entity.index() ? 0 : 1;
    return entity_blocks + add;
  }

  size_t lbegin(Entity entity)
  {
    auto first_block = local_block_local(entity.index());
    this->local_at(first_block.offsets());
  }

  size_t lend(Entity entity)
  {
    auto last_block = local_block_local(
        _local_blocks - (_local_blocks % entity.total()) + entity.index());
    this->local_at(last_block.offsets()) + last_block.size();
  }

private:
  size_t _local_blocks = 1;
};

}  // namespace patterns

#endif
