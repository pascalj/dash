#ifndef PATTERNS__BLOCK_PATTERN_H
#define PATTERNS__BLOCK_PATTERN_H

#include "Configuration.h"
#include "PatternTree.h"
#include "Entity.h"
#include "EntityMapping.h"

template<typename ValueType>
class BlockPattern {
private:
  using ConfigurationT = Configuration<Process>;
  using PatternTree = PatternNode<void, PatternLeaf<ConfigurationT>, PatternLeaf<ConfigurationT>>;
  using EntityMapping = LinearEntityMapping<PatternTree>;
  using ElementMapping = BalancedElementMapping<PatternTree, EntityMapping>;

public:
  BlockPattern(size_t total) : total(total) {}

  template<typename Entity>
  ValueType lbegin(const Entity &entity) {
    auto location = entity_mapping.map(entity);
    return ElementMapping::offset(entity, location, total);
  }

  template<typename Entity>
  ValueType lend(const Entity &entity) {
    auto location = entity_mapping.map(entity);
    auto offset = ElementMapping::offset(entity, location, total);
    auto size = ElementMapping::count(entity, location, total);
    return offset + size;
  };

private:
  size_t total;
  PatternTree tree;
  EntityMapping entity_mapping;
  ElementMapping element_mapping;

public:
  using tree_t = PatternTree;
};

#endif
