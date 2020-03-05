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
    const T    init,
    BinaryOp   binary_op,
    UnaryOp    unary_op)
{
  auto& pattern = in_first.pattern();

  using executor_t  = typename std::decay<Executor>::type;
  using pattern_t   = typename InputIt::pattern_type;
  using shape_t     = typename pattern_t::viewspec_type;
  using dim         = alpaka::dim::DimInt<pattern_t::ndim()>;
  using acc_t       = typename executor_t::acc_t;
  using dev_t       = typename executor_t::dev_t;
  using value_type  = typename InputIt::value_type;
  // FIXME: evaluate from binary_op
  using result_type = uint64_t;


  std::vector<std::promise<result_type>> results{};
  results.reserve(1024);


  for (auto& entity : executor.entities()) {
    auto& queue       = executor.async_queue(entity);
    for (auto& block : pattern.blocks_local_for_entity(entity)) {
      if (block.size() == 0) {
        continue;
      }

      auto extents = alpaka::vec::
          createVecFromIndexedFnWorkaround<dim, std::size_t, arr_to_vec>(
              block.extents());

      // 2. create host buffer
      // FIXME: this assumes first == pattern.first()
      auto offsets = block.offsets();
      auto block_begin = in_first + pattern.local_at(offsets);
      assert(block_begin.is_local());

      auto host = alpaka::dev::DevCpu{
          alpaka::pltf::getDevByIdx<alpaka::pltf::PltfCpu>(0u)};
      auto host_buf = alpaka::mem::view::createStaticDevMemView(
          block_begin.local(), host, extents);
      results.emplace_back();
      dreduce<result_type>(
          std::addressof(results.back()), host, entity, queue, block.size(), host_buf, binary_op);
    }
  }

  using local_result_t = struct dash::internal::local_result<result_type>;
  local_result_t local_result;
  local_result_t global_result;

  local_result.value = std::accumulate(
      results.begin(),
      results.end(),
      init,
      [binary_op](auto accum, auto& promise) {
        auto fut = promise.get_future();
        return binary_op(accum, fut.get());
      });
  local_result.valid = true;

  dart_operation_t dop =
      dash::internal::dart_reduce_operation<BinaryOp>::value;
  dart_datatype_t dtype = dash::dart_storage<result_type>::dtype;

  if (dop == DART_OP_UNDEFINED || dtype == DART_TYPE_UNDEFINED) {
    dart_type_create_custom(sizeof(local_result_t), &dtype);

    // we need a custom reduction operation because not every unit
    // may have valid values
    dart_op_create(
        &dash::internal::reduce_custom_fn<result_type, BinaryOp>,
        &binary_op,
        true,
        dtype,
        true,
        &dop);
    dart_allreduce(&local_result, &global_result, 1, dtype, dop, pattern.team().dart_id());
    dart_op_destroy(&dop);
    dart_type_destroy(&dtype);
  } else {
    // ideal case: we can use DART predefined reductions
    dart_allreduce(
        &local_result.value,
        &global_result.value,
        1,
        dtype,
        dop,
        pattern.team().dart_id());
  }
  return global_result.value;
}

}  // namespace dash
