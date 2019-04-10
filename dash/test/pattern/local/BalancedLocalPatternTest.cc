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
  EXPECT_EQ_U(pattern.nblocks_for_entity(e1), 1);
  EXPECT_EQ_U(pattern.nblocks_for_entity(e2), 0);
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
  auto e1_blocks    = pattern.nblocks_for_entity(e1);
  auto e2_blocks    = pattern.nblocks_for_entity(e2);
  auto e3_blocks    = pattern.nblocks_for_entity(e3);

  EXPECT_EQ_U(e1_blocks + e2_blocks + e3_blocks, local_blocks.size());
}

TEST_F(BalancedLocalPatternTest, TwoDimensional)
{
  const auto x            = 50;
  const auto y            = 20;
  const auto block_height = 5;

  using BasePattern = dash::TilePattern<2>;

  dash::SizeSpec<2>         sizespec(x, y);
  dash::DistributionSpec<2> distspec(
      dash::BLOCKCYCLIC(block_height), dash::NONE);

  BasePattern base{sizespec, distspec, dash::TeamSpec<2>(dash::Team::All())};
  patterns::BalancedLocalPattern<BasePattern, TestEntity> pattern{base};

  size_t sum = 0;
  for(size_t i = 0; i < 3; i++) {
    TestEntity e{i, 3};
    const auto nblocks = pattern.nblocks_for_entity(e);
    if(nblocks > 0) {
      const auto expected_lbegin = i * block_height * y;
      EXPECT_EQ_U(pattern.lbegin(e), expected_lbegin);
    }

    sum += nblocks;
  }

  // Check whether all local blocks were distribted to entities
  const auto local_blocks = pattern.local_blockspec();
  EXPECT_EQ_U(sum, local_blocks.size());
}

TEST_F(BalancedLocalPatternTest, BlockRange) {
  const auto x            = 50;
  const auto y            = 20;
  const auto block_height = 5;

  using BasePattern = dash::TilePattern<2>;

  dash::SizeSpec<2>         sizespec(x, y);
  dash::DistributionSpec<2> distspec(
      dash::BLOCKCYCLIC(block_height), dash::NONE);

  BasePattern base{sizespec, distspec, dash::TeamSpec<2>(dash::Team::All())};
  patterns::BalancedLocalPattern<BasePattern, TestEntity> pattern{base};

  // force some empty entities
  const size_t total_entities = x / block_height / dash::size() - 1;

  // Each new block should be offset by block_height (regardless of entity)
  size_t x_offset = 0;
  for (size_t i = 0; i < total_entities; i++) {
    TestEntity e{i, total_entities};
    for (auto& block : pattern.blocks_local_for_entity(e)) {
      EXPECT_EQ_U(x_offset, block.offsets()[0]);
      x_offset += block_height;
    }
  }
}

