#ifndef DASH__ALGORITHM__FOR_EACH_H__
#define DASH__ALGORITHM__FOR_EACH_H__

#include <dash/algorithm/LocalRange.h>
#include <dash/Execution.h>
#include <dash/Mephisto.h>
#include <dash/iterator/GlobIter.h>

#include <algorithm>


namespace dash {

/**
 * Invoke a function on every element in a range distributed by a pattern.
 * This function has the same signature as \c std::for_each but
 * Being a collaborative operation, each unit will invoke the given
 * function on its local elements only.
 * To support compiler optimization, this const version is provided
 *
 * \tparam      GlobIter      Global Iterator to iterate the sequence
 * \tparam      UnaryFunction Function to invoke for each element
 *                            in the specified range with signature
 *                            \c (void (const ElementType &)).
 *                            Signature does not need to have \c (const &)
 *                            but must be compatible to \c std::for_each.
 *
 * \complexity  O(d) + O(nl), with \c d dimensions in the global iterators'
 *              pattern and \c nl local elements within the global range
 *
 * \ingroup     DashAlgorithms
 */
template <typename GlobInputIt, class UnaryFunction>
void for_each(
    /// Iterator to the initial position in the sequence
    const GlobInputIt& first,
    /// Iterator to the final position in the sequence
    const GlobInputIt& last,
    /// Function to invoke on every index in the range
    UnaryFunction func)
{
  using iterator_traits = dash::iterator_traits<GlobInputIt>;
  static_assert(
      iterator_traits::is_global_iterator::value,
      "must be a global iterator");
  /// Global iterators to local index range:
  auto index_range  = dash::local_index_range(first, last);
  auto lbegin_index = index_range.begin;
  auto lend_index   = index_range.end;
  auto & team       = first.pattern().team();
  if (lbegin_index != lend_index) {
    // Pattern from global begin iterator:
    auto & pattern    = first.pattern();
    // Local range to native pointers:
    auto lrange_begin = (first + pattern.global(lbegin_index)).local();
    auto lrange_end   = lrange_begin + lend_index;
    std::for_each(lrange_begin, lrange_end, func);
  }
  team.barrier();
}

/**
 * Invoke a function on every element in a range distributed by a pattern.
 * Being a collaborative operation, each unit will invoke the given
 * function on its local elements only. The index passed to the function is
 * a global index.
 *
 * \tparam      GlobIter               Global Iterator to iterate the sequence
 * \tparam      UnaryFunctionWithIndex Function to invoke for each element
 *                                     in the specified range with signature
 *                                     \c void (const ElementType &, index_t)
 *                                     Signature does not need to have
 *                                     \c (const &) but must be compatible
 *                                     to \c std::for_each.
 *
 * \complexity  O(d) + O(nl), with \c d dimensions in the global iterators'
 *              pattern and \c nl local elements within the global range
 *
 * \ingroup     DashAlgorithms
 */
template <typename GlobInputIt, class UnaryFunctionWithIndex>
void for_each_with_index(
    /// Iterator to the initial position in the sequence
    const GlobInputIt& first,
    /// Iterator to the final position in the sequence
    const GlobInputIt& last,
    /// Function to invoke on every index in the range
    UnaryFunctionWithIndex func)
{
  using iterator_traits = dash::iterator_traits<GlobInputIt>;
  static_assert(
      iterator_traits::is_global_iterator::value,
      "must be a global iterator");

  /// Global iterators to local index range:
  auto index_range  = dash::local_index_range(first, last);
  auto lbegin_index = index_range.begin;
  auto lend_index   = index_range.end;
  auto & team       = first.pattern().team();
  if (lbegin_index != lend_index) {
    // Pattern from global begin iterator:
    auto & pattern    = first.pattern();
    auto first_offset = first.pos();
    // Iterate local index range:
    for (auto lindex = lbegin_index;
         lindex != lend_index;
         ++lindex) {
      auto gindex       = pattern.global(lindex);
      auto element_it   = first + (gindex - first_offset);
      func(*(element_it.local()), gindex);
    }
  }
  team.barrier();
}

namespace internal {

  template <typename Func>
  struct for_each_kernel {
    for_each_kernel(Func func) : func(func) {}

    template <typename TAcc, typename T>
    FN_HOST_ACC void operator()(const TAcc& acc, T* begin, std::size_t num_elems) const
    {
      const auto gridThreadIdx(
          alpaka::idx::getIdx<alpaka::Grid, alpaka::Threads>(acc)[0u]);
      const auto threadElemExtent(
          alpaka::workdiv::getWorkDiv<alpaka::Thread, alpaka::Elems>(
              acc)[0u]);
      const auto threadFirstElemIdx(gridThreadIdx * threadElemExtent);

      if (threadFirstElemIdx < num_elems) {
        // Calculate the number of elements to compute in this thread.
        // The result is uniform for all but the last thread.
        const auto threadLastElemIdx(threadFirstElemIdx + threadElemExtent);
        const auto threadLastElemIdxClipped(
            (num_elems > threadLastElemIdx) ? threadLastElemIdx : num_elems);

        for (auto i = threadFirstElemIdx; i < threadLastElemIdxClipped; ++i) {
          func(begin[i]);
        }
      }
    }

    Func func;
  };
  }  // namespace internal

  /**
   * Execute a function for each element in a list.
   *
   * ForEach is executed for side-effects (else you'd use transform). For this
   * reason, it is required that first/last and all other accessed elements
   * are on a memory space that are accessible by the func. [first, last) will
   * be explictly copied to the device, but no other objects will. You're free
   * to stage them at the executor's nearest memory.
   *
   * Note that due to the executor a par_unseq policy is implied. This means
   * that the iterations are *not* executed in sequence.
   */
  template <typename Executor, typename InputIt, class UnaryFunction>
  typename std::enable_if<
      dash::is_executor<typename std::decay<Executor>::type>::value>::type
  for_each(
      Executor&& executor,
      /// Iterator to the initial position in the sequence
      const InputIt& first,
      /// Iterator to the final position in the sequence
      const InputIt& last,
      /// Function to invoke on every index in the range
      UnaryFunction func)
  {
    auto& pattern = first.pattern();

    using executor_t = typename std::decay<Executor>::type;
    using kernel_t   = internal::for_each_kernel<UnaryFunction>;
    using pattern_t  = typename InputIt::pattern_type;
    using shape_t    = typename pattern_t::viewspec_type;
    using dim        = alpaka::dim::DimInt<pattern_t::ndim()>;
    using acc_t      = typename executor_t::acc_t;
    using dev_t      = typename executor_t::dev_t;
    using value_type = typename InputIt::value_type;

    auto& queue        = executor.sync_queue();
    auto  index_range  = dash::local_index_range(first, last);
    auto  lbegin_index = index_range.begin;
    auto  lend_index   = index_range.end;
    auto& team         = first.pattern().team();
    auto  host         = alpaka::dev::DevCpu{
        alpaka::pltf::getDevByIdx<alpaka::pltf::PltfCpu>(0u)};

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
                static_cast<std::size_t>(1)); //block.size() / nblocks));

        std::cout << "Thread extent: " << thread_extent << std::endl;

        auto offsets = block.offsets();

        alpaka::workdiv::WorkDivMembers<dim, std::size_t> const workDiv{
            alpaka::workdiv::getValidWorkDiv<acc_t>(
                entity.device(), extents, thread_extent, false)};


        std::cout << "Workdiv: " << workDiv << std::endl;

        // 2. create host view
        // FIXME: this assumes first == pattern.first()
        auto block_begin = first + pattern.local_at(offsets);

        assert(block_begin.is_local());

        auto host_buf = alpaka::mem::view::createStaticDevMemView(
            block_begin.local(), host, extents);

        // 4. work
        kernel_t   kernel(func);
        auto const taskKernel = alpaka::kernel::createTaskKernel<acc_t>(
            workDiv,
            kernel,
            alpaka::mem::view::getPtrNative(host_buf),
            block.size());

        alpaka::queue::enqueue(queue, taskKernel);
#ifdef ALPAKA_ACC_GPU_CUDA_ENABLED
        auto err = cudaMemPrefetchAsync(block_begin.local(), sizeof(value_type) * block.size(), cudaCpuDeviceId, queue.m_spQueueImpl->m_CudaQueue);
        std::cout << "queue: " << queue.m_spQueueImpl->m_CudaQueue << std::endl;
        std::cout << "errocode: " << err << std::endl;
#endif
      }
    }
}

} // namespace dash

#endif // DASH__ALGORITHM__FOR_EACH_H__
