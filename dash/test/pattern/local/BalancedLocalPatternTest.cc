#include "BalancedLocalPatternTest.h"

#include "dash/Dimensional.h"
#include "test_entity.h"
#include "patterns/local_pattern.h"


TEST_F(BalancedLocalPatternTest, OneBlock) {
    TestEntity e1(0, 2);
    TestEntity e2(1, 2);

    using BasePattern = dash::BlockPattern<1>;

    BasePattern base(
        500, dash::BLOCKED, dash::TeamSpec<1>(dash::Team::All()));
    patterns::BalancedLocalPattern<BasePattern, TestEntity> pattern(base);

    // Locally we only have 1 block, regardless of the number of units
    EXPECT_EQ(base.local_blockspec(), 1);

    // Because there is only one block per unit, only the first entity should
    // be assigned a block
    EXPECT_EQ(pattern.blocks_for_entity(e1), 1);
    EXPECT_EQ(pattern.blocks_for_entity(e2), 0);
}

TEST_F(BalancedLocalPatternTest, EqualBlocks) {
    TestEntity e1(0, 3);
    TestEntity e2(1, 3);
    TestEntity e3(2, 3);

    using BasePattern = dash::TilePattern<1>;

    dash::DistributionSpec<1> distspec(dash::BLOCKCYCLIC(10));

    BasePattern base(
        500, dash::BLOCKCYCLIC(10), dash::TeamSpec<1>(dash::Team::All()));
    patterns::BalancedLocalPattern<BasePattern, TestEntity> pattern(base);

    // Because there is only one block per unit, only the first entity should
    // be assigned a block
    //
    auto local_blocks = pattern.local_blockspec();
    auto e1_blocks = pattern.blocks_for_entity(e1);
    auto e2_blocks = pattern.blocks_for_entity(e2);
    auto e3_blocks = pattern.blocks_for_entity(e3);

    EXPECT_EQ(e1_blocks + e2_blocks + e3_blocks, local_blocks.size());
}

TEST_F(BalancedLocalPatternTest, TwoDimensional) {
    TestEntity e1(0, 3);
    TestEntity e2(1, 3);
    TestEntity e3(2, 3);

    using BasePattern = dash::TilePattern<2>;

    dash::SizeSpec<2> sizespec(20, 30);
    dash::DistributionSpec<2> distspec(dash::BLOCKCYCLIC(10), dash::NONE);

    BasePattern base(
        sizespec,
        distspec,
        dash::TeamSpec<2>(dash::Team::All()));
    patterns::BalancedLocalPattern<BasePattern, TestEntity> pattern(base);

    // Because there is only one block per unit, only the first entity should
    // be assigned a block
    //
    auto local_blocks = pattern.local_blockspec();
    auto e1_blocks = pattern.blocks_for_entity(e1);
    auto e2_blocks = pattern.blocks_for_entity(e2);
    auto e3_blocks = pattern.blocks_for_entity(e3);

    EXPECT_EQ(e1_blocks + e2_blocks + e3_blocks, local_blocks.size());
}
