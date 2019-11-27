
#include "ForEachTest.h"

#include <dash/Array.h>
#include <dash/Matrix.h>
#include <dash/SharedCounter.h>
#include <dash/algorithm/Fill.h>
#include <dash/algorithm/ForEach.h>
#include <dash/algorithm/Reduce.h>

#include <dash/Mephisto.h>
#include <patterns/local_pattern.h>

#include <functional>


TEST_F(ForEachTest, TestArrayAllInvoked) {
    // Shared variable for total number of invoked callbacks:
    dash::SharedCounter<size_t> count_invokes;
    // Create for_each callback from member function:
    std::function<void(const Array_t::value_type &)> invoke =
        std::bind(&ForEachTest::count_invoke, this, std::placeholders::_1);
    // Ensure value global counter is published to all units
    dash::Team::All().barrier();
    // Initialize global array:
    Array_t array(_num_elem);
    // Run for_each on complete array
    dash::for_each(array.begin(), array.end(), invoke);
    // Should have been invoked on every local index in the array:
    LOG_MESSAGE("Local number of inspected indices: %zu",
                _invoked_indices.size());
    EXPECT_EQ(array.lsize(), _invoked_indices.size());
    // Count number of local invokes
    count_invokes.inc(_invoked_indices.size());
    // Wait for all units
    array.barrier();
    // Test number of total invokes
    size_t num_invoked_indices_all = count_invokes.get();
    LOG_MESSAGE("Total number of inspected indices: %zu",
                num_invoked_indices_all);
    EXPECT_EQ(_num_elem, num_invoked_indices_all);
}

TEST_F(ForEachTest, ForEachWithIndex)
{
    if (dash::size() == 3) {
      // TODO: Fix this
      SKIP_TEST();
    }

    std::function<void(const Element_t &, index_t)> dummy_fct =
    [](Element_t el, index_t idx) {
        EXPECT_EQ_U(
            el,
            static_cast<Element_t>(dash::myid())
        );
    };

    // Test Array
    Array_t array(_num_elem);
    dash::fill(
        array.begin(),
        array.end(),
        static_cast<Element_t>(dash::myid()));

    dash::for_each_with_index(
        array.begin(),
        array.end(),
        dummy_fct);

    // Test Matrix
    dash::Matrix<Element_t, 2> matrix(dash::SizeSpec<2>(
                                        dash::size(),
                                        dash::size()));
    dash::fill(
        matrix.begin(),
        matrix.end(),
        static_cast<Element_t>(dash::myid()));

    dash::for_each_with_index(
        matrix.begin(),
        matrix.end(),
        dummy_fct);
}

TEST_F(ForEachTest, ForEachWithIndexPos)
{
  dash::Array<int> array(100, dash::CYCLIC);

  // Fill
  std::function< void(const int &, index_t)>
  fill = [&array](int el, index_t i) {
    auto coords = array.pattern().coords(i);
    array[i] = coords[0];
  };

  // Verify
  std::function< void(const int &, index_t)>
    verify = [&array](int el, index_t i) {
      auto coords  = array.pattern().coords(i);
      auto desired = coords[0];
      ASSERT_EQ_U(
        desired,
        el);
    };

  // Fill
  dash::for_each_with_index(
    array.begin(),
    array.end(),
    fill);

  dash::for_each_with_index(
    array.begin(),
    array.end(),
    verify);
}

TEST_F(ForEachTest, ModifyValues)
{
  dash::Array<int> array(100, dash::TILE(10));
  dash::fill(array.begin(), array.end(), dash::myid());

  std::function< void(int &)>
    incr = [](int & el) {
      el = el+1;
  };
  std::function< void(const int & )>
    verify = [](const int & el){
      ASSERT_EQ_U(el, dash::myid()+1);
  };

  // Increment by one
  dash::for_each(array.begin(), array.end(), incr);
  // Verify
  dash::for_each(array.begin(), array.end(), verify);
}

TEST_F(ForEachTest, Lambdas)
{
  dash::Array<int> array(100, dash::TILE(10));
  dash::fill(array.begin(), array.end(), dash::myid());

  // -- dash::for_each -------------------------------------------

  // Increment by 100:
  dash::for_each(array.begin(), array.end(),
                 [](int & el) {
                   el += 100;
                 });
  // Verify:
  dash::for_each(array.begin(), array.end(),
                 [](int & el) {
                   EXPECT_EQ_U(100 + dash::myid(), el);
                 });

  // -- dash::for_each_with_index --------------------------------

  // Increment by element index:
  dash::for_each_with_index(
                 array.begin(), array.end(),
                 [](int & el, index_t gindex) {
                   el += gindex;
                 });
  // Verify:
  dash::for_each_with_index(
                 array.begin(), array.end(),
                 [](int & el, index_t gindex) {
                   EXPECT_EQ_U(100 + dash::myid() + gindex, el);
                 });
}

#ifdef ALPAKA_ACC_CPU_B_SEQ_T_SEQ_ENABLED
TEST_F(ForEachTest, MephistoBasicTest)
{
  using value_t   = int;
  using entity_t  = dash::CpuThreadEntity<1>;
  using pattern_t =
      patterns::BalancedLocalPattern<dash::BlockPattern<1>, entity_t>;

  pattern_t pattern{100 * dash::size()};
  const auto layout = dash::ROW_MAJOR;
  dash::NArray<value_t, 1, pattern_t::index_type, pattern_t> arr(pattern);

  dash::AlpakaExecutor<entity_t> executor;

  dash::fill(arr.begin(), arr.end(), 51);

  dash::for_each(
      executor, arr.begin(), arr.end(), [](int& a) {
        return a = a * 111;
      });

  auto sum = dash::reduce(arr.begin(), arr.end(), 0);
  EXPECT_EQ_U(dash::size() * 51 * 100 * 111, sum);
}
#endif

#ifdef ALPAKA_ACC_GPU_CUDA_ENABLED

void run_my_test() {
  using value_t   = uint64_t;
  using entity_t  = dash::CudaEntity<1>;
  using pattern_t =
      patterns::UnitRRPattern<dash::BlockPattern<1>, entity_t>;

  using memory_t = dash::CudaSpace;

  const size_t problem_size = 64 * 1024 * 1024 * dash::size() / sizeof(value_t);
  const value_t init = 51;

  pattern_t pattern{problem_size};
  dash::NArray<value_t, 1, pattern_t::index_type, pattern_t, memory_t> arr(pattern);

  dash::AlpakaExecutor<entity_t> executor;

  dash::fill(arr.begin(), arr.end(), init);

#ifdef __CUDACC_EXTENDED_LAMBDA__
  auto times111 = [=] __device__ (value_t &a) { return a *= 111; };
#else
#error "Need extended lambdas."
#endif
  dash::for_each(
      executor, arr.begin(), arr.end(), times111);

  cudaDeviceSynchronize();
  long long sum = dash::reduce(arr.begin(), arr.end(), 0LL);
  EXPECT_EQ(problem_size * init * 111, sum);
}


TEST_F(ForEachTest, MephistoCudaBasicTest)
{
  run_my_test();
}
#endif
