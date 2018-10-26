#include "gtest/gtest.h"
#include "patterns/BlockPattern.h"
#include "DummyUnit.hpp"

using value_t = float;


TEST(BlockPattern, ConceptTest) {
  DummyUnit dummy{2};
  BlockPattern<float, DummyUnit> pattern(100);
  auto index = pattern.local_at(23);
  EXPECT_EQ(index, 23);
  EXPECT_EQ(pattern.lbegin(dummy), 0);
  EXPECT_EQ(pattern.lend(dummy), 50);
}
