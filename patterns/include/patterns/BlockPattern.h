#ifndef PATTERNS__BLOCK_PATTERN_H
#define PATTERNS__BLOCK_PATTERN_H

#include "Configuration.h"
#include "PatternTree.h"
#include "Entity.h"
#include "EntityMapping.h"

class BlockPattern {
private:
  using Configuration = Configuration<Process>;
  using PatternTree = PatternNode<Configuration, void, void>;
  using EntityMapping = RoundRobinEntityMapping<PatternTree>;

public:
  template<typename Entity>
  size_t lbegin(const Entity &entity) {
    constexpr auto location = entity_mapping.map(entity);
    return 1;
  }

private:
  size_t total;
  PatternTree tree;
  EntityMapping entity_mapping;
};

#endif
