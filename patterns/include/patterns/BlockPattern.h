#ifndef PATTERNS__BLOCK_PATTERN_H
#define PATTERNS__BLOCK_PATTERN_H

#include "Configuration.h"
#include "PatternTree.h"
#include "Entity.h"
#include "EntityMapping.h"


using index_t = size_t;
// coord_t is currently only one-dimensional
using coord_t = size_t;

struct ViewSpec {
  coord_t offset;
  size_t extent;
};

template<typename ValueType, typename DefaultEntity = Process>
class BlockPattern {
private:
  using ConfigurationT = Configuration<DefaultEntity>;
  using PatternTree = PatternNode<void, PatternLeaf<ConfigurationT>, PatternLeaf<ConfigurationT>>;
  using EntityMapping = LinearEntityMapping<PatternTree>;
  using ElementMapping = BalancedElementMapping<PatternTree, EntityMapping>;

public:
  BlockPattern(size_t capacity) : _capacity(capacity) {}

  template<typename Entity = DefaultEntity>
  index_t lbegin(const Entity &entity = Entity::current()) {
    auto location = entity_mapping.map(entity);
    return ElementMapping::offset(entity, location, _capacity);
  }

  template<typename Entity = DefaultEntity>
  index_t lend(const Entity &entity = Entity::current()) {
    auto location = entity_mapping.map(entity);
    auto offset = ElementMapping::offset(entity, location, _capacity);
    auto size = ElementMapping::count(entity, location, _capacity);
    return offset + size;
  };

  /**
   * Maps the local coord to a local linear index
   */
  index_t local_at(coord_t local_coord) {
    return lbegin() + local_coord;
  }

  /**
   * Maps a global coord to a global linear index
   */
  index_t global_at(coord_t global_coord) {
    return global_coord;
  }

  /**
   * Maps a global coordinate to its default entity
   */
  DefaultEntity unit_at(coord_t global_coord) {
    
  }

  // global to local
  /**
   * Maps a local coordinate to a global coordinate
   */
  coord_t global(DefaultEntity, coord_t local_coord);

  /**
   * Maps a local coordinate to a global linear index
   */
  index_t global_index(DefaultEntity, coord_t local_coord);

  // TODO: dimensions
  /* coord_t global(EntityType, coord_t local_coord); */
  /* index_t global(EntityType, index_t local_index); */
  /* index_t global(index_t local_index); */
  //
  // blocks

  /**
   * Returns the number of blocks [in each dimension]
   */
  size_t blockspec();

  /**
   * Maps a global coordinate to a block
   */
  ViewSpec block_at(coord_t global_coord);

  /**
   * Maps the global linear inde to a block
   */
  ViewSpec block(index_t global_index); 

  /**
   * Maps the local linear index to a block
   */
  ViewSpec local_block(index_t local_index); 

  /**
   * Maps the local linear index to a local viewspec/offset
   */
  ViewSpec local_block_local(index_t local_index); 

  /**
   * Test whether the global index is local to the current unit
   */
  bool is_local(index_t global_index);
  
  /**
   * Test whether the local index is local to the current unit
   */
  /* bool is_local(dim_t dim, index_t local_index, EntityType); */
  
  size_t capacity();
  size_t local_capacity(DefaultEntity);
  size_t size();

  template<typename EntityType>
  size_t local_size(EntityType);
  /* size_t extents(); */
  /* size_t extent(dim_t dimension); */
  /* size_t local_extents(dim_t dimensions); */
  /* size_t local_extent(dim_t d); */



private:
  // capacity of total elements
  size_t _capacity;
  PatternTree tree;
  EntityMapping entity_mapping;
  ElementMapping element_mapping;

public:
  using tree_t = PatternTree;
};

#endif
