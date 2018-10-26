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

  template<typename Entity>
  ValueType lbegin(const Entity &entity) {
    auto location = entity_mapping.map(entity);
    return ElementMapping::offset(entity, location, _capacity);
  }

  template<typename Entity>
  ValueType lend(const Entity &entity) {
    auto location = entity_mapping.map(entity);
    auto offset = ElementMapping::offset(entity, location, _capacity);
    auto size = ElementMapping::count(entity, location, _capacity);
    return offset + size;
  };

  index_t local_at(coord_t local_coord) {
    return lbegin(DefaultEntity::current()) + local_coord;
  }
  index_t global_at(coord_t global_coord);
  DefaultEntity unit_at(coord_t global_coord);

  // global to local
  coord_t global(DefaultEntity, coord_t local_coord);
  index_t global_index(DefaultEntity, coord_t local_coord);
  /* coord_t global(EntityType, coord_t local_coord); */
  /* index_t global(EntityType, index_t local_index); */
  /* index_t global(index_t local_index); */
  //
  // blocks
  size_t blockspec();
  ViewSpec block_at(coord_t global_coord);
  ViewSpec block(index_t global_index); 
  ViewSpec local_block(index_t local_index); 
  ViewSpec local_block_local(index_t local_index); 

  // locality test
  bool is_local(index_t global_index, DefaultEntity);
  // TODO Dimensions
  /* bool is_local(dim_t dim, index_t local_index, EntityType); */
  
  size_t capacity();
  size_t local_capacity(DefaultEntity);
  size_t size();
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
