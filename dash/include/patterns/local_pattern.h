#ifndef PATTERNS__LOCAL_PATTERN_H
#define PATTERNS__LOCAL_PATTERN_H

#include <cmath>
#include <cstddef>
#include <vector>

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

  size_t lbegin(Entity entity) const
  {
    auto first_block = this->local_block_local(entity.index());
    return this->local_at(first_block.offsets());
  }

  size_t lend(Entity entity) const
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
    return this->local_block(block_index);
  }

  std::vector<block_type> blocks_local_for_entity(const Entity &entity) const
  {
    const auto entity_blocks = nblocks_for_entity(entity);
    std::vector<block_type> vec{entity_blocks};
    for(std::size_t i = 0; i < entity_blocks; i++) {
      vec.push_back(block_local_for_entity(entity, i));
    }
    return vec;
  }

  size_t nblocks_for_entity(Entity entity) const
  {
    auto const entity_blocks = _local_blocks / entity.total();
    auto const add =
        ((_local_blocks % entity.total()) <= entity.index()) ? 0 : 1;
    return entity_blocks + add;
  }

#if 0
  size_t entity_lbegin(Entity entity) const
  {
    auto first_blockspec = this->block_local_for_entity(entity, 0);
    DASH_LOG_TRACE(
        "BalancedLocalPattern.lbegin", "first_blockspec", first_blockspec);
    return this->local_at(first_blockspec.offsets());
  }

  size_t entity_lend(Entity entity) const
  {
    auto total_local_blocks = nblocks_for_entity(entity);
    auto last_blockspec =
        this->block_local_for_entity(entity, total_local_blocks - 1);
    return this->local_at(last_blockspec.offsets()) + last_blockspec.size();
  }
#endif

private:
  size_t _local_blocks = 1;
};

}  // namespace patterns

#endif
