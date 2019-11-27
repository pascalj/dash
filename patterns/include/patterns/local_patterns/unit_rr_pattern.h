#pragma once

namespace patterns {

/**
 * A unit round robin pattern
 *
 * Distributes blocks not based on their relative local position but based on
 * the unit id, e.g. the assigned entity depends solely on the unit's ID.
 * Useful for homogeneous environments, i.e. where the number of accelerators
 * per host is constant.
 */
template <typename Base, typename Entity>
class UnitRRPattern : public Base {
  constexpr static size_t NumDimensions = Base::ndim();

public:
  using block_type  = typename Base::viewspec_type;
  using entity_type = Entity;

  UnitRRPattern() = delete;

  UnitRRPattern(Base base)
    : Base(base), _local_blocks(base.local_blockspec().size())
  { std::cout << "Number of local blocks:" << _local_blocks << std::endl; }

  auto block_for_entity(Entity entity, size_t index) const
  {
    return this->local_block(index);
  }

  auto block_local_for_entity(Entity entity, size_t index) const
  {
    return this->local_block(index);
  }

  std::vector<block_type> blocks_local_for_entity(const Entity &entity) const
  {
    const auto entity_blocks = nblocks_for_entity(entity);
    std::vector<block_type> vec{entity_blocks};

    for(std::size_t i = 0; i < entity_blocks; i++) {
      vec.push_back(this->local_block(i));
    }
    return vec;
  }

  size_t nblocks_for_entity(const Entity &entity) const
  {
    return _entity_matches_unit(entity) ? _local_blocks : 0;
  }

private:

  std::size_t _entity_matches_unit(const Entity &entity) const {
    return this->team().myid() % Entity::total() == entity.index();
  }

  size_t _local_blocks = 1;
};

}
