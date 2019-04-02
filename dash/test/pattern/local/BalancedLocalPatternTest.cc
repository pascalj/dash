#include "BalancedLocalPatternTest.h"

#include "dash/Dimensional.h"
#include "patterns/local_pattern.h"
#include "test_entity.h"

TEST_F(BalancedLocalPatternTest, OneBlock)
{
  TestEntity e1(0, 2);
  TestEntity e2(1, 2);

  using BasePattern = dash::BlockPattern<1>;

  BasePattern base(500, dash::BLOCKED, dash::TeamSpec<1>(dash::Team::All()));
  patterns::BalancedLocalPattern<BasePattern, TestEntity> pattern(base);

  // Locally we only have 1 block, regardless of the number of units
  auto local_blocks = base.local_blockspec();
  EXPECT_EQ_U(local_blocks, 1);

  // Because there is only one block per unit, only the first entity should
  // be assigned a block
  EXPECT_EQ_U(pattern.blocks_for_entity(e1), 1);
  EXPECT_EQ_U(pattern.blocks_for_entity(e2), 0);
}

TEST_F(BalancedLocalPatternTest, EqualBlocks)
{
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
  auto e1_blocks    = pattern.blocks_for_entity(e1);
  auto e2_blocks    = pattern.blocks_for_entity(e2);
  auto e3_blocks    = pattern.blocks_for_entity(e3);

  EXPECT_EQ_U(e1_blocks + e2_blocks + e3_blocks, local_blocks.size());
}

TEST_F(BalancedLocalPatternTest, TwoDimensional)
{
  TestEntity e1(0, 3);
  TestEntity e2(1, 3);
  TestEntity e3(2, 3);

  const auto x            = 50;
  const auto y            = 20;
  const auto block_height = 5;

  using BasePattern = dash::TilePattern<2>;

  dash::SizeSpec<2>         sizespec(x, y);
  dash::DistributionSpec<2> distspec(
      dash::BLOCKCYCLIC(block_height), dash::NONE);

  BasePattern base{sizespec, distspec, dash::TeamSpec<2>(dash::Team::All())};
  patterns::BalancedLocalPattern<BasePattern, TestEntity> pattern(base);

  const auto local_blocks = pattern.local_blockspec();
  const auto e1_blocks    = pattern.blocks_for_entity(e1);
  const auto e2_blocks    = pattern.blocks_for_entity(e2);
  const auto e3_blocks    = pattern.blocks_for_entity(e3);

  // Check whether all local blocks were distribted to entities
  EXPECT_EQ_U(e1_blocks + e2_blocks + e3_blocks, local_blocks.size());
  const auto expected_lbegin = block_height * y * dash::myid();
  std::cout << "P: " << pattern.block_for_entity(e1, 0)
            << " (expected: " << expected_lbegin << ")" << std::endl;
  EXPECT_EQ_U(pattern.lbegin(e1), expected_lbegin);
}
