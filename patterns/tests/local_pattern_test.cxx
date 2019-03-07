#include "gtest/gtest.h"
#include "local_pattern_test.h"

namespace {
  TEST(RoundRobinLocalPatternTest, Local) {
    ViewSpec<1> view({1}, {1});
    TestEntity e;
    BasePattern base(view);
    RoundRobinTestPattern pattern(base);

    EXPECT_EQ(pattern.blocks_for_entity(e), 1);
  }
}
