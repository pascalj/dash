
#include "MinMaxElementTest.h"

#include <dash/algorithm/MinMax.h>
#include <dash/Array.h>


TEST_F(MinMaxElementTest, TestFindArrayDefault)
{
  // Initialize global array:
  Array_t array(_num_elem);
  Element_t max_value = (array.size() * 11) + 1;
  Element_t min_value = -max_value;
  if (dash::myid() == 0) {
    for (auto i = 0; i < array.size(); ++i) {
      Element_t value = i * 11;
      array[i] = value;
    }
    // Set minimum and maximum elements around the center position:
    index_t min_pos = array.size() / 2 - 1;
    index_t max_pos = array.size() / 2;
    array[min_pos] = min_value;
    array[max_pos] = max_value;
  }
  // Wait for array initialization
  array.barrier();

  auto min_git = array.end();
  auto max_git = array.end();
  // Run max_element on complete array
  std::tie(min_git, max_git) = dash::minmax_element(
                     array.begin(),
                     array.end());
  // Check that a minimum has been found (found != last):
  EXPECT_NE_U(min_git, array.end());
  // Check that a maximum has been found (found != last):
  EXPECT_NE_U(max_git, array.end());
  // Check minimum value found
  Element_t found_min = *min_git;
  // Check maximum value found
  Element_t found_max = *max_git;
  LOG_MESSAGE("Expected min value: %d, found min value %d",
              static_cast<int>(min_value),
              static_cast<int>(found_min));
  LOG_MESSAGE("Expected max value: %d, found max value %d",
              static_cast<int>(max_value),
              static_cast<int>(found_max));
  EXPECT_EQ(min_value, found_min);
  EXPECT_EQ(max_value, found_max);
}

TEST_F(MinMaxElementTest, TestFindArrayDistributeBlockcyclic)
{
  // Using a prime as block size for 'inconvenient' strides.
  int block_size   = 7;
  size_t num_units = dash::Team::All().size();
  LOG_MESSAGE("Units: %zu, block size: %d, elements: %d",
              num_units, block_size, static_cast<int>(_num_elem));

  // Initialize global array:
  Array_t array(_num_elem, dash::BLOCKCYCLIC(block_size));
  Element_t max_value = (array.size() * 23) + 1;
  Element_t min_value = -max_value;
  if (dash::myid() == 0) {
    for (auto i = 0; i < array.size(); ++i) {
      Element_t value = i * 23;
      array[i] = value;
    }
    // Set maximum element somewhere in the first half:
    index_t max_pos = array.size() / 3;
    // Set minimum element somewhere in the second half:
    index_t min_pos = array.size() * 0.7;
    array[min_pos] = min_value;
    array[max_pos] = max_value;
  }
  // Wait for array initialization
  array.barrier();

  auto min_git = array.end();
  auto max_git = array.end();
  std::tie(min_git, max_git) = dash::minmax_element(
                     array.begin(),
                     array.end());
  // Check that a maximum has been found (found != last):
  EXPECT_NE_U(min_git, array.end());
  EXPECT_NE_U(max_git, array.end());
  // Check maximum value found
  Element_t found_min = *min_git;
  Element_t found_max = *max_git;
  LOG_MESSAGE("Expected min value: %d, found minimum value %d",
              static_cast<int>(min_value),
              static_cast<int>(found_min));
  LOG_MESSAGE("Expected max value: %d, found maximum value %d",
              static_cast<int>(max_value),
              static_cast<int>(found_max));
  EXPECT_EQ(min_value, found_min);
  EXPECT_EQ(max_value, found_max);
}

TEST_F(MinMaxElementTest, TestFindArrayUnderfilled)
{
  // Choose block size and number of blocks so at least
  // one unit has an empty local range and one unit has an
  // underfilled block.
  // Using a prime as block size for 'inconvenient' strides.
  int block_size   = 19;
  size_t num_units = dash::Team::All().size();
  size_t num_elem  = ((num_units - 1) * block_size) - block_size / 2;
  if (num_units < 2) {
    num_elem = block_size - 1;
  }
  LOG_MESSAGE("Units: %zu, block size: %d, elements: %d",
              num_units, block_size, static_cast<int>(num_elem));
  // Initialize global array:
  Array_t array(num_elem, dash::BLOCKCYCLIC(block_size));
  Element_t max_value = (array.size() * 23) + 1;
  Element_t min_value = -max_value;
  if (dash::myid() == 0) {
    for (auto i = 0; i < array.size(); ++i) {
      Element_t value = i * 23;
      array[i] = value;
    }
    // Set maximum element in the last position which is located
    // in the underfilled block, for extra nastyness:
    index_t max_pos = array.size() - 1;
    array[0] = min_value;
    array[max_pos] = max_value;
  }
  // Wait for array initialization
  array.barrier();
  auto min_git = array.end();
  auto max_git = array.end();
  std::tie(min_git, max_git) = dash::minmax_element(
                     array.begin(),
                     array.end());
  // Check that a maximum has been found (found != last):
  EXPECT_NE_U(min_git, array.end());
  EXPECT_NE_U(max_git, array.end());
  // Check maximum value found
  Element_t found_min = *min_git;
  Element_t found_max = *max_git;
  LOG_MESSAGE("Expected min value: %d, found minimum value %d",
              static_cast<int>(min_value),
              static_cast<int>(found_min));
  LOG_MESSAGE("Expected max value: %d, found maximum value %d",
              static_cast<int>(max_value),
              static_cast<int>(found_max));
  EXPECT_EQ(min_value, found_min);
  EXPECT_EQ(max_value, found_max);
}
