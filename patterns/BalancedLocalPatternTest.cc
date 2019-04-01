#include "BalancedLocalPatternTest.h"

#include "test_entity.h"
#include "patterns/local_pattern.h"


TEST(BalancedLocalPatternTest, Trivial) {
    dash::SizeSpec<1> size_spec(500);
    TestEntity e1(0, 2);
    TestEntity e2(1, 2);

    using BasePattern = dash::BlockPattern<1>;

    BasePattern base(size_spec, dash::BLOCKED, dash::TeamSpec<1>(dash::Team::All()));
    patterns::RoundRobinLocalPattern<BasePattern, TestEntity> pattern(base);

    EXPECT_EQ(pattern.blocks_for_entity(e1), 1);
    EXPECT_EQ(pattern.blocks_for_entity(e2), 0);
}
