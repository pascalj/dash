#ifndef PATTERNS__TESTS__LOCAL_PATTERN_TEST_H
#define PATTERNS__TESTS__LOCAL_PATTERN_TEST_H

#include "blocked_base_pattern.h"
#include "test_entity.h"
#include "patterns/local_pattern.h"

using BasePattern = BlockedBasePattern<1>;
using RoundRobinTestPattern =
    patterns::RoundRobinLocalPattern<BasePattern,TestEntity>;

#endif
