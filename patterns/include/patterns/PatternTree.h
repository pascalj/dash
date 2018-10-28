#ifndef PATTERNS__PATTERN_TREE_H
#define PATTERNS__PATTERN_TREE_H

#include "Entity.h"

template<typename Root, typename DefaultEntity = Process>
class PatternTree {
public:
  Root root;
};

#endif
