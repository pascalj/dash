#include <dash/algorithm/LocalRange.h>
#include <dash/algorithm/Reduce.h>
#include <dash/iterator/IteratorTraits.h>
#include <dash/Execution.h>

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
    class ExecutionPolicy,
    class InputIt,
    class T,
    class BinaryOperation,
    class UnaryOperation>
typename std::enable_if<
    is_execution_policy<typename std::decay<ExecutionPolicy>::type>::value,
    T>::type
transform_reduce(
    ExecutionPolicy&& policy,
    InputIt           in_first,
    InputIt           in_last,
    T                 init,
    BinaryOperation   binary_op,
    UnaryOperation    unary_op)
{
  using value_type = typename std::iterator_traits<InputIt>::value_type;

  auto& executor = policy.executor();
  // This is not standard, but necessary to reserve the right amount of resources
  size_t         concurrency = executor.concurrency();
  std::vector<T> results(concurrency, init);

  policy.executor().bulk_twoway_execute(
      // note: we can only capture by copy here
      [=](size_t      block_index,
          size_t      element_index,
          T*          res,
          value_type* block_first) {
        res[block_index] =
            binary_op(res[block_index], unary_op(block_first[element_index]));
      },
      in_first,                                      // "shape"
      [&]() -> std::vector<T>& { return results; },  // result factory
      [=] {
        return std::make_pair(in_first, in_last);
      });  // shared state, unused here

  in_first.pattern().team().barrier();

  // Use dash's reduce for the non-local parts
  return reduce(results.begin(), results.end(), init, binary_op);
}

}  // namespace dash
