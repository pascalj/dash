#include "gtest/gtest.h"
#include "local_pattern_test.h"

namespace {
  TEST(RoundRobinLocalPatternTest, Trivial) {
    dash::SizeSpec<1> size_spec(500);
    TestEntity e1(0, 2);
    TestEntity e2(1, 2);

    BasePattern<1> base(size_spec, dash::BLOCKED, dash::TeamSpec<1>(dash::Team::All()));
    RoundRobinTestPattern<1> pattern(base);

    EXPECT_EQ(pattern.blocks_for_entity(e1), 1);
    EXPECT_EQ(pattern.blocks_for_entity(e2), 0);
  }

  TEST(RoundRobinLocalPatternTest, BlockCountMultiple) {
    dash::SizeSpec<3> size_spec(5, 3, 7);
    TestEntity e1(0, 2);
    TestEntity e2(1, 2);

    BasePattern<3> base(size_spec);
    RoundRobinTestPattern<3> pattern(base);

    auto e1_entities = pattern.blocks_for_entity(e1);
    auto e2_entities = pattern.blocks_for_entity(e2);

    EXPECT_EQ(e1_entities, 53);
    EXPECT_EQ(e2_entities, 52);
    EXPECT_EQ(5 * 3 * 7, e1_entities + e2_entities);
  }


  TEST(BalancedLocalPatternTest, Trivial) {
    dash::SizeSpec<3> size_spec(5, 3, 7);
    TestEntity e1(0, 2);
    TestEntity e2(1, 2);

    BasePattern<3> base(size_spec);
    patterns::BalancedLocalPattern<BasePattern<3>, TestEntity> pattern(base);

    auto e1_entities = pattern.blocks_for_entity(e1);
    auto e2_entities = pattern.blocks_for_entity(e2);

    EXPECT_EQ(e1_entities, 53);
    EXPECT_EQ(e2_entities, 52);
    EXPECT_EQ(5 * 3 * 7, e1_entities + e2_entities);
  }

  TEST(BalancedLocalPatternTest, Mapping) {
    dash::SizeSpec<3> size_spec(5, 3, 7);
    TestEntity e1(0, 2);
    TestEntity e2(1, 2);

    BasePattern<3> base(size_spec);
    patterns::BalancedLocalPattern<BasePattern<3>, TestEntity> pattern(base);

    // EXPECT_EQ(pattern.block_for_entity(e1, 0), 0);
  }
}
