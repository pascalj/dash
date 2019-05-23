#ifndef PATTERNS__LOCAL_PATTERN_H
#define PATTERNS__LOCAL_PATTERN_H

#include <cmath>
#include <cstddef>

#include "block_range.h"

namespace patterns {

template <typename Base, typename Entity>
class RoundRobinLocalPattern : public Base {
  constexpr static size_t NumDimensions = Base::ndim();

public:
  RoundRobinLocalPattern() = delete;

  RoundRobinLocalPattern(Base base)
    : Base(base)
  {
    _local_blocks = this->local_blockspec().size();
  }

  auto block_for_entity(Entity entity, size_t index) const
  {
    auto block_index = index * entity.total() + entity.index();
    return this->local_block(block_index);
  }

  auto block_local_for_entity(Entity entity, size_t index) const
  {
    auto block_index = index * entity.total() + entity.index();
    return this->local_block_local(block_index);
  }

  size_t nblocks_for_entity(Entity entity) const
  {
    auto const entity_blocks = _local_blocks / entity.total();
    auto const add =
        ((_local_blocks % entity.total()) <= entity.index()) ? 0 : 1;
    return entity_blocks + add;
  }

  size_t lbegin(Entity entity)
  {
    auto first_block = this->local_block_local(entity.index());
    return this->local_at(first_block.offsets());
  }

  size_t lend(Entity entity)
  {
    auto last_block = local_block_local(
        _local_blocks - (_local_blocks % entity.total()) + entity.index());
    return this->local_at(last_block.offsets()) + last_block.size();
  }

private:
  size_t _local_blocks = 1;
};

template <typename Base, typename Entity>
class BalancedLocalPattern : public Base {
  constexpr static size_t NumDimensions = Base::ndim();

public:
  using block_type  = typename Base::viewspec_type;
  using entity_type = Entity;

  using block_range = BlockRange<BalancedLocalPattern>;

  BalancedLocalPattern() = delete;

  BalancedLocalPattern(Base base)
    : Base(base), _local_blocks(base.local_blockspec().size())
  { }

  auto block_for_entity(Entity entity, size_t index) const
  {
    auto block_index =
        (std::ceil(static_cast<float>(_local_blocks) / entity.total()) *
         entity.index()) +
        index;
    return this->local_block(block_index);
  }

  auto block_local_for_entity(Entity entity, size_t index) const
  {
    auto block_index =
        (std::ceil(static_cast<float>(_local_blocks) / entity.total()) *
         entity.index()) +
        index;
    return this->local_block_local(block_index);
  }

  block_range blocks_local_for_entity(const Entity &entity) const
  {
    return block_range{this, &entity};
  }

  size_t nblocks_for_entity(Entity entity) const
  {
    auto const entity_blocks = _local_blocks / entity.total();
    auto const add =
        ((_local_blocks % entity.total()) <= entity.index()) ? 0 : 1;
    return entity_blocks + add;
  }

private:
  size_t _local_blocks = 1;
};

}  // namespace patterns

#endif
