#include <dash/algorithm/LocalRange.h>
#include <dash/algorithm/Reduce.h>
#include <dash/iterator/IteratorTraits.h>
#include <dash/Execution.h>
#include <dash/Mephisto.h>

#include "mephisto/ReduceKernel.h"

namespace dash {

namespace detail {

template <class InputIt, class T, class UnaryOperation, class BinaryOperation>
T local_transform_reduce_simple(
    InputIt         lbegin,
    InputIt         lend,
    T               init,
    BinaryOperation binary_op,
    UnaryOperation  unary_op)
{
  // Transform-reduce locally in one step
  auto accu = init;
  while (lbegin != lend) {
    accu = binary_op(accu, unary_op(*lbegin++));
  }
  return accu;
}
}  // namespace detail

/**
 * Transformes and reduces a range of values
 *
 * First unary_op is applied to each element and the result is reduced using
 * binary_op with the starting value of init.
 *
 * Note: binary_op must be communitative and associative.
 */
template <class InputIt, class T, class UnaryOperation, class BinaryOperation>
T transform_reduce(
    InputIt         in_first,
    InputIt         in_last,
    T               init,
    BinaryOperation binary_op,
    UnaryOperation  unary_op)
{
  auto local_range = dash::local_range(in_first, in_last);
  auto lbegin      = local_range.begin;
  auto lend        = local_range.end;

  auto local_result = detail::local_transform_reduce_simple(
      lbegin, lend, init, binary_op, unary_op);

  // Use dash's reduce for the non-local parts
  return reduce(
      std::addressof(local_result),
      std::next(std::addressof(local_result), 1),
      init,
      binary_op,
      true,
      in_first.team());
}

/**
 * Transformes and reduces a range of values into a root
 *
 * First unary_op is applied to each element and the result is reduced using
 * binary_op with the starting value of init.
 *
 * Note: The return value on all other units except root is undefined.
 *
 * Note: binary_op must be communitative and associative.
 */
template <class InputIt, class T, class UnaryOperation, class BinaryOperation>
T transform_reduce(
    InputIt         in_first,
    InputIt         in_last,
    T               init,
    BinaryOperation binary_op,
    UnaryOperation  unary_op,
    team_unit_t     root)
{
  auto local_range = dash::local_range(in_first, in_last);
  auto lbegin      = local_range.begin;
  auto lend        = local_range.end;

  auto local_result = detail::local_transform_reduce_simple(
      lbegin, lend, init, binary_op, unary_op);

  in_first.pattern().team().barrier();

  // Use dash's reduce for the non-local parts
  return reduce(
      std::addressof(local_result),
      std::next(std::addressof(local_result), 1),
      init,
      binary_op,
      true,
      in_first.team(),
      &root);
}

namespace internal {
template <typename BinaryOp, typename UnaryOp>
struct transform_reduce_kernel {

  template <typename TAcc>
  void operator()(const TAcc& acc) const
  {
  }
};
}

template <
    class Executor,
    class InputIt,
    class T,
    class BinaryOp,
    class UnaryOp>
typename std::enable_if<
    dash::is_executor<typename std::decay<Executor>::type>::value,
    T>::type
transform_reduce(
    Executor&& executor,
    InputIt    in_first,
    InputIt    in_last,
    T          init,
    BinaryOp   binary_op,
    UnaryOp    unary_op)
{
  auto& pattern = in_first.pattern();

  using executor_t  = typename std::decay<Executor>::type;
  using kernel_t    = internal::transform_reduce_kernel<BinaryOp, UnaryOp>;
  using pattern_t   = typename InputIt::pattern_type;
  using shape_t     = typename pattern_t::viewspec_type;
  using dim         = alpaka::dim::DimInt<pattern.ndim()>;
  using acc_t       = typename executor_t::acc_t;
  using dev_t       = typename executor_t::dev_t;
  using value_type  = typename InputIt::value_type;
  using result_type = double;

  auto& queue       = executor.sync_queue();

  std::vector<result_type> results;

  for (auto& entity : executor.entities()) {
    for (auto& block : pattern.blocks_local_for_entity(entity)) {
      if (block.size() == 0) {
        continue;
      }

      // 1. create workdiv from block
      auto extents = alpaka::vec::
          createVecFromIndexedFnWorkaround<dim, std::size_t, arr_to_vec>(
              block.extents());
      int nblocks = 1;

      if (char* blocks_override = getenv("NBLOCKS")) {
        nblocks = atoi(blocks_override);
      }
      auto thread_extent = alpaka::vec::
          createVecFromIndexedFnWorkaround<dim, std::size_t, unity_vec>(
              static_cast<std::size_t>(block.size() / nblocks));

      auto offsets = block.offsets();

      alpaka::workdiv::WorkDivMembers<dim, std::size_t> const workDiv{
          alpaka::workdiv::getValidWorkDiv<acc_t>(
              entity.device(), extents, thread_extent, false)};

      // 2. create host buffer
      // FIXME: this assumes first == pattern.first()
      auto block_begin = in_first + pattern.local_at(offsets);

      assert(block_begin.is_local());

      auto host = alpaka::dev::DevCpu{
          alpaka::pltf::getDevByIdx<alpaka::pltf::PltfCpu>(0u)};
      auto host_buf = alpaka::mem::view::createStaticDevMemView(
          block_begin.local(), host, extents);

      mreduce<result_type, acc_t>(host, entity.device(), queue, block.size(), host_buf, binary_op);
    }
  }
}

}  // namespace dash
