#ifndef DASH__ALGORITHM__TRANSFORM_H__
#define DASH__ALGORITHM__TRANSFORM_H__

#include <dash/GlobAsyncRef.h>
#include <dash/GlobRef.h>

#include <dash/algorithm/LocalRange.h>
#include <dash/algorithm/Operation.h>

#include <dash/Iterator.h>

#include <dash/internal/Config.h>
#include <dash/util/Trace.h>

#include <dash/dart/if/dart_communication.h>
#include <dash/Execution.h>
#include <cstddef>

#include <dash/algorithm/internal/TransformReduce.h>

#include <dash/Mephisto.h>

#ifdef DASH_ENABLE_OPENMP
#include <omp.h>
#endif

namespace dash {

#ifdef DOXYGEN

/**
 * Apply a given function to elements in a range and store the result in
 * another range, beginning at \c out_first.
 * Corresponding to \c MPI_Accumulate, the unary operation is executed
 * atomically on single elements.
 *
 * Precondition: All elements in the input range are contained in a single
 * block so that
 *
 * <tt>
 *   g_out_last == g_out_first + (l_in_last - l_in_first)
 * </tt>
 *
 * Semantics:
 *
 * <tt>
 *   unary_op(in_first[0]), unary_op(in_first[1]), ..., unary_op(in_first[n])
 * </tt>
 *
 * \returns  Output iterator to the element past the last element transformed.
 *
 * \ingroup  DashAlgorithms
 */
template<
  typename ValueType,
  class InputIt,
  class OutputIt,
  class UnaryOperation >
OutputIt transform(
  InputIt        in_first,
  InputIt        in_last,
  OutputIt       out_first,
  UnaryOperation unary_op);

/**
 * Apply a given function to pairs of elements from two ranges and store the
 * result in another range, beginning at \c out_first.
 *
 * Corresponding to \c MPI_Accumulate, the binary operation is executed
 * atomically on single elements.
 *
 * Precondition: All elements in the input range are contained in a single
 * block so that
 *
 *   g_out_last == g_out_first + (l_in_last - l_in_first)
 *
 * Semantics:
 *
 *   binary_op(in_a[0], in_b[0]),
 *   binary_op(in_a[1], in_b[1]),
 *   ...,
 *   binary_op(in_a[n], in_b[n])
 *
 * Example:
 * \code
 *   gptr_diff_t num_transformed_elements =
 *                 dash::distance(
 *                   dash::transform(in.begin(), in.end(),  // A
 *                                   out.begin(),           // B
 *                                   out.begin(),           // C = op(A, B)
 *                                   dash::plus<int>()),    // op
 *                   out.end());
 *
 * \endcode
 *
 * \returns  Output iterator to the element past the last element transformed.
 * \see      dash::reduce
 * \see      DashReduceOperations
 *
 * \tparam   InputIt         Iterator on first (local) input range
 * \tparam   GlobInputIt     Iterator on second (global) input range
 * \tparam   GlobOutputIt    Iterator on global result range
 * \tparam   BinaryOperation Reduce operation type
 *
 * \ingroup  DashAlgorithms
 */
template<
  class InputIt1,
  class GlobInputIt,
  class GlobOutputIt,
  class BinaryOperation >
GlobOutputIt transform(
  /// Iterator on begin of first local range
  InputIt1         in_a_first,
  /// Iterator after last element of local range
  InputIt1         in_a_last,
  /// Iterator on begin of second local range
  GlobInputIt     in_b_first,
  /// Iterator on first element of global output range
  GlobOutputIt    out_first,
  /// Reduce operation
  BinaryOperation binary_op);

#else

namespace internal {
template <typename UnaryOp>
struct transform_kernel {
  transform_kernel(UnaryOp& unary_op) : unary_op(unary_op) {}

  template <typename TAcc, typename InputT, typename OutputT>
  void operator()(
      TAcc const&   acc,
      const InputT* begin,
      OutputT*      out,
      std::size_t   num_elems) const
  {
    const auto gridThreadIdx(
        alpaka::idx::getIdx<alpaka::Grid, alpaka::Threads>(acc)[0u]);
    const auto threadElemExtent(
        alpaka::workdiv::getWorkDiv<alpaka::Thread, alpaka::Elems>(acc)[0u]);
    const auto threadFirstElemIdx(gridThreadIdx * threadElemExtent);

    if (threadFirstElemIdx < num_elems) {
      // Calculate the number of elements to compute in this thread.
      // The result is uniform for all but the last thread.
      const auto threadLastElemIdx(threadFirstElemIdx + threadElemExtent);
      const auto threadLastElemIdxClipped(
          (num_elems > threadLastElemIdx) ? threadLastElemIdx : num_elems);

      for (auto i = threadFirstElemIdx; i < threadLastElemIdxClipped; ++i) {
        out[i] = unary_op(begin[i]);
      }
    }
  }

  UnaryOp unary_op;
  };

  /**
   * Wrapper of the blocking DART accumulate operation.
   */
  template <typename ValueType>
  inline dart_ret_t transform_blocking_impl(
      dart_gptr_t      dest,
      ValueType*       values,
      size_t           nvalues,
      dart_operation_t op)
  {
    static_assert(
        dash::dart_datatype<ValueType>::value != DART_TYPE_UNDEFINED,
        "Cannot accumulate unknown type!");

    dart_ret_t result = dart_accumulate(
        dest, (values), nvalues, dash::dart_datatype<ValueType>::value, op);
    dart_flush(dest);
    return result;
}

/**
 * Wrapper of the non-blocking DART accumulate operation.
 */
template< typename ValueType >
dart_ret_t transform_impl(
  dart_gptr_t        dest,
  ValueType        * values,
  size_t             nvalues,
  dart_operation_t   op)
{
  static_assert(dash::dart_datatype<ValueType>::value != DART_TYPE_UNDEFINED,
      "Cannot accumulate unknown type!");

  dart_ret_t result = dart_accumulate(
                        dest,
                        (values),
                        nvalues,
                        dash::dart_datatype<ValueType>::value,
                        op);
  dart_flush_local(dest);
  return result;
}

struct transform_impl_local_input_it{};
struct transform_impl_glob_input_it{};

/**
 * Transform operation on ranges with identical distribution and start
 * offset.
 * In this case, no communication is needed as all output values can be
 * obtained from input values in local memory:
 *
 * \note
 * This function does not execute the transformation as atomic operation
 * on elements. Use \c dash::transform if concurrent access to elements is
 * possible.
 *
 * <pre>
 *   input a: [ u0 | u1 | u2 | ... ]
 *              op   op   op   ...
 *   input b: [ u0 | u1 | u2 | ... ]
 *              =    =    =    ...
 *   output:  [ u0 | u1 | u2 | ... ]
 * </pre>
 */
template <
    typename ValueType,
    class InputAIt,
    class InputBIt,
    class GlobOutputIt,
    class BinaryOperation>
GlobOutputIt transform_local(
    InputAIt        in_a_first,
    InputAIt        in_a_last,
    InputBIt        in_b_first,
    GlobOutputIt    out_first,
    BinaryOperation binary_op)
{
  DASH_LOG_DEBUG("dash::transform_local()");
  DASH_ASSERT_MSG(in_a_first.pattern() == in_b_first.pattern(),
                  "dash::transform_local: "
                  "distributions of input ranges differ");
  DASH_ASSERT_MSG(in_a_first.pattern() == out_first.pattern(),
                  "dash::transform_local: "
                  "distributions of input- and output ranges differ");
  // Local subrange of input range a:
  auto local_range_a   = dash::local_range(in_a_first, in_a_last);
  ValueType * lbegin_a = local_range_a.begin;
  ValueType * lend_a   = local_range_a.end;
  if (lbegin_a == lend_a) {
    // Local input range is empty, return initial output iterator to indicate
    // that no values have been transformed:
    DASH_LOG_DEBUG("dash::transform_local", "local range empty");
    return out_first;
  }
  // Global offset of first local element:
  auto g_offset_first    = in_a_first.pattern().global(0);
  // Number of elements in global ranges:
  auto num_gvalues       = dash::distance(in_a_first, in_b_first);
  DASH_LOG_TRACE_VAR("dash::transform_local", num_gvalues);
  // Number of local elements:
  DASH_LOG_TRACE("dash::transform_local", "local elements:", lend_a-lbegin_a);
  // Local subrange of input range b:
  ValueType * lbegin_b   = (in_b_first + g_offset_first).local();
  // Local pointer of initial output element:
  ValueType * lbegin_out = (out_first  + g_offset_first).local();
  // Generate output values:
#ifdef DASH_ENABLE_OPENMP
  dash::util::UnitLocality uloc;
  auto n_threads = uloc.num_domain_threads();
  DASH_LOG_DEBUG("dash::transform_local", "thread capacity:",  n_threads);
  if (n_threads > 1) {
    auto l_size = lend_a - lbegin_a;
    // TODO: Vectorize.
    // Documentation of Intel MIC intrinsics, see:
    // https://software.intel.com/de-de/node/523533
    // https://software.intel.com/de-de/node/523387
    #pragma omp parallel for num_threads(n_threads) schedule(static)
    for (int i = 0; i < l_size; i++) {
      lbegin_out[i] = binary_op(lbegin_a[i], lbegin_b[i]);
    }
    return out_first + num_gvalues;
  }
#endif
  // No OpenMP or insufficient number of threads for parallelization:
  for (; lbegin_a != lend_a; ++lbegin_a, ++lbegin_b, ++lbegin_out) {
    *lbegin_out = binary_op(*lbegin_a, *lbegin_b);
  }
  // Return out_end iterator past final transformed element;
  return out_first + num_gvalues;
}


/**
 * Specialization of \c dash::transform for global lhs input range.
 */
template <
    class InputIt,
    class GlobInputIt,
    class GlobOutputIt,
    class BinaryOperation>
GlobOutputIt transform(
    /// Iterator on begin of first local range
    InputIt in_a_first,
    /// Iterator after last element of local range
    InputIt in_a_last,
    /// Iterator on begin of second local range
    GlobInputIt in_b_first,
    /// Iterator on first element of global output range
    GlobOutputIt out_first,
    /// Reduce operation
    BinaryOperation binary_op,
    /// Specialization for a global input iterator
    transform_impl_glob_input_it /*unused*/)
{
  using iterator_traits = dash::iterator_traits<InputIt>;
  DASH_LOG_DEBUG("dash::transform(gaf, gal, gbf, goutf, binop)");
  auto in_first = in_a_first;
  auto in_last  = in_a_last;

  if (in_b_first == out_first) {
    // Output range is rhs input range: C += A
    // Input is (in_a_first, in_a_last).
  } else {
    DASH_THROW(
      dash::exception::NotImplemented,
      "dash::transform is only implemented for out = op(in,out)");
    // Output range different from rhs input range: C = A+B
    // Input is (in_a_first, in_a_last) + (in_b_first, in_b_last):

    // TODO:
    // in_range.allocate(...);
    // in_first = in_range.begin();
    // in_last  = in_range.end();
  }

  dash::util::Trace trace("transform");

  // Pattern of input ranges a and b, and output range:
  const auto& pattern_in_a = in_a_first.pattern();
  const auto& pattern_in_b = in_b_first.pattern();
  const auto& pattern_out  = out_first.pattern();

#if __NON_ATOMIC__
  // Fast path: check if transform operation is local-only:
  if (pattern_in_a == pattern_in_b &&
      pattern_in_a == pattern_out) {
    // Identical pattern in all ranges
    if (in_a_first.pos() == in_b_first.pos() &&
        in_a_first.pos() == out_first.pos()) {
      trace.enter_state("local");
      // All units operate on local ranges that have identical distribution:
      auto out_last = dash::transform_local<iterator_traits::value_type>(
                        in_a_first,
                        in_a_last,
                        in_b_first,
                        out_first,
                        binary_op);
      trace.exit_state("local");
      return out_last;
    }
  }
#endif
  // Resolve teams from global iterators:
  dash::Team & team_in_a        = pattern_in_a.team();
  DASH_ASSERT_MSG(
    team_in_a == pattern_in_b.team(),
    "dash::transform: Different teams in input ranges");
  DASH_ASSERT_MSG(
    team_in_a == pattern_out.team(),
    "dash::transform: Different teams in input- and output ranges");
  // Resolve local range from global range:
  auto l_index_range_in_a       = local_index_range(in_a_first, in_a_last);
  DASH_LOG_TRACE_VAR("dash::transform", l_index_range_in_a.begin);
  DASH_LOG_TRACE_VAR("dash::transform", l_index_range_in_a.end);
  // Local range to global offset:
  auto global_offset            = pattern_in_a.global(
                                    l_index_range_in_a.begin);
  DASH_LOG_TRACE_VAR("dash::transform", global_offset);
  // Number of elements in local range:
  size_t num_local_elements     = l_index_range_in_a.end -
                                  l_index_range_in_a.begin;
  DASH_LOG_TRACE_VAR("dash::transform", num_local_elements);
  // Global iterator to dart_gptr_t:
  dart_gptr_t dest_gptr         = (out_first + global_offset).dart_gptr();
  // Native pointer to local sub-range:
  auto l_values          = (in_a_first + global_offset).local();
  // Send accumulate message:
  trace.enter_state("transform_blocking");
  dash::internal::transform_blocking_impl(
      dest_gptr,
      l_values,
      num_local_elements,
      binary_op.dart_operation());
  trace.exit_state("transform_blocking");

  return out_first + global_offset + num_local_elements;

}

template <
    class InputIt,
    class GlobInputIt,
    class GlobOutputIt,
    class BinaryOperation>
GlobOutputIt transform(
    /// Iterator on begin of first local range
    InputIt in_a_first,
    /// Iterator after last element of local range
    InputIt in_a_last,
    /// Iterator on begin of second local range
    GlobInputIt in_b_first,
    /// Iterator on first element of global output range
    GlobOutputIt out_first,
    /// Reduce operation
    BinaryOperation binary_op,
    transform_impl_local_input_it /*unused*/)
{
  DASH_LOG_DEBUG("dash::transform(af, al, bf, outf, binop)");
  // Outut range different from rhs input range is not supported yet
  auto in_first = in_a_first;
  auto in_last  = in_a_last;

  using value_type = typename dash::iterator_traits<InputIt>::value_type;
  std::vector<value_type> in_range;
  if (in_b_first == out_first) {
    // Output range is rhs input range: C += A
    // Input is (in_a_first, in_a_last).
  } else {
    // Output range different from rhs input range: C = A+B
    // Input is (in_a_first, in_a_last) + (in_b_first, in_b_last):
    std::transform(
      in_a_first, in_a_last,
      in_b_first,
      std::back_inserter(in_range),
      binary_op);
    in_first = in_range.data();
    in_last  = in_first + in_range.size();
  }

  dash::util::Trace trace("transform");

  // Resolve local range from global range:
  // Number of elements in local range:
  size_t num_local_elements     = std::distance(in_first, in_last);
  // Global iterator to dart_gptr_t:
  dart_gptr_t dest_gptr         = out_first.dart_gptr();
  // Send accumulate message:
  trace.enter_state("transform_blocking");
  dash::internal::transform_blocking_impl(
      dest_gptr,
      in_first,
      num_local_elements,
      binary_op.dart_operation());
  trace.exit_state("transform_blocking");
  // The position past the last element transformed in global element space
  // cannot be resolved from the size of the local range if the local range
  // spans over more than one block. Otherwise, the difference of two global
  // iterators is not well-defined. The corresponding invariant is:
  //   g_out_last == g_out_first + (l_in_last - l_in_first)
  // Example:
  //   unit:            0       1       0
  //   local offset:  | 0 1 2 | 0 1 2 | 3 4 5 | ...
  //   global offset: | 0 1 2   3 4 5   6 7 8   ...
  //   range:          [- - -           - -]
  // When iterating in local memory range [0,5[ of unit 0, the position of the
  // global iterator to return is 8 != 5
  // For ranges over block borders, we would have to resolve the global
  // position past the last element transformed from the iterator's pattern
  // (see dash::PatternIterator).
  return out_first + num_local_elements;
}

}

/**
 * Implementation for execution policies with a unary operation.
 *
 * This implementation assumes nothing about the allocation of first and
 * out_first, but optimizes when first == out_first.
 *
 * When the policy provides an executor, the bulk_twoway_execute function
 * is called with global *iterators* as input. The executor has to handle
 * DASH's iterators.
 *
 */
template <
    class Executor,
    class GlobInputIt,
    class GlobOutputIt,
    class UnaryOp>
typename std::enable_if<
    dash::is_executor<typename std::decay<Executor>::type >::value,
    GlobOutputIt>::type
transform(
    // The execution policy to use
    Executor&& executor,
    /// Iterator on begin of first local range
    GlobInputIt first,
    /// Iterator after last element of local range
    GlobInputIt last,
    /// Iterator on first element of global output range
    GlobOutputIt out_first,
    /// Reduce operation
    UnaryOp unary_op)
{
  DASH_LOG_DEBUG("dash::transform(executor, af, al, bf, outf, binop)");
  // Outut range different from rhs input range is not supported yet

  using value_type = typename dash::iterator_traits<GlobInputIt>::value_type;
  std::vector<value_type> result(std::distance(first, last));
  if (first  != out_first) {
    DASH_THROW(
      dash::exception::NotImplemented,
      "dash::transform is only implemented for out = op(in,out)");
    // Output range is rhs input range: C += A
    // Input is (in_a_first, in_a_last).
  } else {
    auto& pattern  = first.pattern();

    using executor_t  = typename std::decay<Executor>::type;
    using kernel_t    = internal::transform_kernel<UnaryOp>;
    using pattern_t   = typename GlobInputIt::pattern_type;
    using shape_t     = typename pattern_t::viewspec_type;
    using dim         = alpaka::dim::DimInt<pattern.ndim()>;
    using acc_t       = typename executor_t::acc_t;
    using dev_t       = typename executor_t::dev_t;
    using value_type  = typename GlobInputIt::value_type;
    using result_type = typename GlobOutputIt::value_type;

    auto  nelems      = dash::distance(first, last);
    auto  out_end     = std::next(out_first, nelems);
    auto  local_range = dash::local_range(out_first, out_end);
    auto& queue       = executor.sync_queue();

    for (auto& entity : executor.entities()) {
      for (auto& block : pattern.blocks_local_for_entity(entity)) {
        if (block.size() == 0) {
          continue;
        }

        // 1. create workdiv from block
        auto extents = alpaka::vec::createVecFromIndexedFnWorkaround<
            dim,
            std::size_t,
            arr_to_vec>(block.extents());
        int nblocks = 1;

        if(char *blocks_override = getenv("NBLOCKS")) {
          nblocks = atoi(blocks_override);
        }
        auto thread_extent = alpaka::vec::createVecFromIndexedFnWorkaround<
            dim,
            std::size_t,
            unity_vec>(
            static_cast<std::size_t>(block.size() / nblocks));

        auto offsets = block.offsets();

        alpaka::workdiv::WorkDivMembers<dim, std::size_t> const workDiv{
            alpaka::workdiv::getValidWorkDiv<acc_t>(
                entity.device(), extents, thread_extent, false)};

        // 2. create host buffer
        // FIXME: this assumes first == pattern.first()
        auto block_begin =
            first + pattern.local_at(offsets);

        assert(block_begin.is_local());

        auto host = alpaka::dev::DevCpu{
            alpaka::pltf::getDevByIdx<alpaka::pltf::PltfCpu>(0u)};
        auto host_buf = alpaka::mem::view::createStaticDevMemView(
            block_begin.local(), host, extents);

        // 3. copy the buffer to the entity
        using device_buf_t =
            alpaka::mem::buf::Buf<dev_t, value_type, dim, std::size_t>;
        device_buf_t device_buf(alpaka::mem::buf::alloc<value_type, std::size_t>(
            entity.device(), extents));
        alpaka::mem::view::copy(queue, device_buf, host_buf, extents);

        // allocate the result buffer
        using device_result_t =
            alpaka::mem::buf::Buf<dev_t, result_type, dim, std::size_t>;
        device_result_t device_result_buf(alpaka::mem::buf::alloc<result_type, std::size_t>(
            entity.device(), extents));

        // 4. work
        kernel_t kernel(unary_op);
        auto const taskKernel = alpaka::kernel::createTaskKernel<acc_t>(
            workDiv,
            kernel,
            alpaka::mem::view::getPtrNative(device_buf),
            alpaka::mem::view::getPtrNative(device_result_buf),
            block.size());

        DASH_LOG_DEBUG("Startking kernel on block", block);
        alpaka::queue::enqueue(queue, taskKernel);

        auto result_block_begin = out_first + pattern.local_at(offsets);
        auto result_buf = alpaka::mem::view::createStaticDevMemView(
            result_block_begin.local(), host, extents);


        DASH_LOG_DEBUG("Result native", alpaka::mem::view::getPtrNative(device_result_buf)[0]);

        // copy result buffer back to host
        alpaka::mem::view::copy(
            queue, result_buf, device_result_buf, extents);
        DASH_LOG_DEBUG("Result out", result_block_begin.local()[0]);
      }
    }
  }

  return out_first;
}

template <
    class InputIt,
    class GlobInputIt,
    class GlobOutputIt,
    class BinaryOperation>
GlobOutputIt transform(
    InputIt         in_a_first,
    InputIt         in_a_last,
    GlobInputIt     in_b_first,
    GlobOutputIt    out_first,
    BinaryOperation binary_op)
{
  using InputIt_traits_t    = dash::iterator_traits<InputIt>;
  using InputIt_is_global_t = typename InputIt_traits_t::is_global_iterator;

  using GlobInputIt_traits_t  = dash::iterator_traits<GlobInputIt>;
  using GlobOutputIt_traits_t = dash::iterator_traits<GlobOutputIt>;

  // currently we support only two cases: the range [in_a_first, in_a_last] may
  // be defined by global or non-global  iterators (i.e., any STL iterator).
  // However, in_b_first and out_first have to be global iterators.
  static_assert(
      GlobInputIt_traits_t::is_global_iterator::value,
      "in_b_first must be a global iterator");

  static_assert(
      GlobOutputIt_traits_t::is_global_iterator::value,
      "out_first must be a global iterator");

  return internal::transform(
      in_a_first,
      in_a_last,
      in_b_first,
      out_first,
      binary_op,
      typename std::conditional<
          InputIt_is_global_t::value,
          internal::transform_impl_glob_input_it,
          internal::transform_impl_local_input_it>::type());
}

/**
 * Specialization of \c dash::transform as non-blocking operation.
 *
 * \tparam   InputIt         Iterator on first (typically local) input range
 * \tparam   GlobInputIt     Iterator on second (typically global) input range
 * \tparam   GlobOutputIt    Iterator on global result range
 * \tparam   BinaryOperation Reduce operation type
 */
template <
    typename ValueType,
    class InputIt,
    class GlobInputIt,
    class BinaryOperation>
GlobAsyncRef<ValueType> transform(
    InputIt /*in_a_first*/,
    InputIt /*in_a_last*/,
    GlobInputIt /*in_b_first*/,
    GlobAsyncRef<ValueType> /*out_first*/,
    BinaryOperation /*binary_op*/ = dash::plus<ValueType>())
{
  DASH_THROW(
    dash::exception::NotImplemented,
    "Async variant of dash::transform is not implemented");
}

#endif

} // namespace dash

#endif // DASH__ALGORITHM__TRANSFORM_H__
