#ifndef PATTERNS__TESTS__LOCAL_PATTERN_TEST_H
#define PATTERNS__TESTS__LOCAL_PATTERN_TEST_H

#include "dash/pattern/BlockPattern.h"
#include "test_entity.h"
#include "patterns/local_pattern.h"

template<int Dim>
using BasePattern = dash::BlockPattern<Dim>;

template<int Dim>
using RoundRobinTestPattern =
    patterns::RoundRobinLocalPattern<BasePattern<Dim>,TestEntity>;

#endif
