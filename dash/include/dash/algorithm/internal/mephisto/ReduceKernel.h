#include <alpaka/alpaka.hpp>
#include "iterator.h"
#include "Config.h"



//#############################################################################
//! A cheap wrapper around a C-style array in heap memory.
template <typename T, uint64_t size>
struct cheapArray
{
    T data[size];

    //-----------------------------------------------------------------------------
    //! Access operator.
    //!
    //! \param index The index of the element to be accessed.
    //!
    //! Returns the requested element per reference.
    ALPAKA_FN_HOST_ACC ALPAKA_FN_INLINE T &operator[](uint64_t index)
    {
        return data[index];
    }

    //-----------------------------------------------------------------------------
    //! Access operator.
    //!
    //! \param index The index of the element to be accessed.
    //!
    //! Returns the requested element per constant reference.
    ALPAKA_FN_HOST_ACC ALPAKA_FN_INLINE const T &operator[](uint64_t index) const
    {
        return data[index];
    }
};

//#############################################################################
//! A reduction kernel.
//!
//! \tparam TBlockSize The block size.
//! \tparam T The data type.
//! \tparam TFunc The Functor type for the reduction function.
template <uint32_t TBlockSize, typename T, typename TFunc>
struct ReduceKernel
{
    ALPAKA_NO_HOST_ACC_WARNING

    //-----------------------------------------------------------------------------
    //! The kernel entry point.
    //!
    //! \tparam TAcc The accelerator environment.
    //! \tparam TElem The element type.
    //! \tparam TIdx The index type.
    //!
    //! \param acc The accelerator object.
    //! \param source The source memory.
    //! \param destination The destination memory.
    //! \param n The problem size.
    //! \param func The reduction function.
    template <typename TAcc, typename TElem, typename TIdx>
    ALPAKA_FN_ACC auto operator()(TAcc const &acc,
                                  TElem const *const source,
                                  TElem *destination,
                                  TIdx const &n,
                                  TFunc func) const -> void
    {
        auto &sdata(
            alpaka::block::shared::st::allocVar<cheapArray<T, TBlockSize>,
                                                __COUNTER__>(acc));

        const uint32_t blockIndex(static_cast<uint32_t>(
            alpaka::idx::getIdx<alpaka::Grid, alpaka::Blocks>(acc)[0]));
        const uint32_t threadIndex(static_cast<uint32_t>(
            alpaka::idx::getIdx<alpaka::Block, alpaka::Threads>(acc)[0]));
        const uint32_t gridDimension(static_cast<uint32_t>(
            alpaka::workdiv::getWorkDiv<alpaka::Grid, alpaka::Blocks>(acc)[0]));

        // equivalent to blockIndex * TBlockSize + threadIndex
        const uint32_t linearizedIndex(static_cast<uint32_t>(
            alpaka::idx::getIdx<alpaka::Grid, alpaka::Thread>(acc)[0]));

        typename GetIterator<T, TElem, TAcc>::Iterator it(
            acc, source, linearizedIndex, gridDimension * TBlockSize, n);

        T result = 0; // suppresses compiler warnings

        if (threadIndex < n)
            result = *(it++); // avoids using the
                              // neutral element of specific

        // --------
        // Level 1: grid reduce, reading from global memory
        // --------

        // reduce per thread with increased ILP by 4x unrolling sum.
        // the thread of our block reduces its 4 grid-neighbored threads and
        // advances by grid-striding loop (maybe 128bit load improve perf)

        while (it + 3 < it.end())
        {
            result = func(
                func(func(result, func(*it, *(it + 1))), *(it + 2)), *(it + 3));
            it += 4;
        }

        // doing the remaining blocks
        while (it < it.end())
            result = func(result, *(it++));

        if (threadIndex < n)
            sdata[threadIndex] = result;

        alpaka::block::sync::syncBlockThreads(acc);

        // --------
        // Level 2: block + warp reduce, reading from shared memory
        // --------

        ALPAKA_UNROLL()
        for (uint32_t currentBlockSize = TBlockSize,
                      currentBlockSizeUp =
                          (TBlockSize + 1) / 2; // ceil(TBlockSize/2.0)
             currentBlockSize > 1;
             currentBlockSize = currentBlockSize / 2,
                      currentBlockSizeUp = (currentBlockSize + 1) /
                                           2) // ceil(currentBlockSize/2.0)
        {
            bool cond =
                threadIndex < currentBlockSizeUp // only first half of block
                                                 // is working
                && (threadIndex + currentBlockSizeUp) <
                       TBlockSize // index for second half must be in bounds
                && (blockIndex * TBlockSize + threadIndex +
                    currentBlockSizeUp) < n &&
                threadIndex <
                    n; // if elem in second half has been initialized before

            if (cond)
                sdata[threadIndex] =
                    func(sdata[threadIndex],
                         sdata[threadIndex + currentBlockSizeUp]);

            alpaka::block::sync::syncBlockThreads(acc);
        }

        // store block result to gmem
        if (threadIndex == 0 && threadIndex < n)
            destination[blockIndex] = sdata[0];
    }
};

//#############################################################################
//! A reduction kernel.
//!
//! \tparam TBlockSize The block size.
//! \tparam T The data type.
//! \tparam TFunc The Functor type for the reduction function.
template <typename T, typename UnaryFunc, typename BinaryFunc>
struct DReduceKernel
{
    //-----------------------------------------------------------------------------
    //! The kernel entry point.
    //!
    //! \tparam TAcc The accelerator environment.
    //! \tparam TElem The element type.
    //! \tparam TIdx The index type.
    //!
    //! \param acc The accelerator object.
    //! \param source The source memory.
    //! \param destination The destination memory.
    //! \param n The problem size.
    //! \param func The reduction function.
    template <typename TAcc, typename TElem, typename TIdx>
    ALPAKA_FN_ACC auto operator()(
        TAcc const &       acc,
        TElem const *const source,
        TElem *            destination,
        TIdx const &       n,
        UnaryFunc          unary_func,
        BinaryFunc         binary_func) const -> void
    {
      auto *const sdata = alpaka::block::shared::dyn::getMem<T>(acc);

      const uint32_t blockIndex(static_cast<uint32_t>(
          alpaka::idx::getIdx<alpaka::Grid, alpaka::Blocks>(acc)[0]));
      const uint32_t threadIndex(static_cast<uint32_t>(
          alpaka::idx::getIdx<alpaka::Block, alpaka::Threads>(acc)[0]));
      const uint32_t gridDimension(static_cast<uint32_t>(
          alpaka::workdiv::getWorkDiv<alpaka::Grid, alpaka::Blocks>(acc)[0]));
      const uint32_t blockSize(static_cast<uint32_t>(
          alpaka::workdiv::getWorkDiv<alpaka::Block, alpaka::Threads>(
              acc)[0]));

      // equivalent to blockIndex * TBlockSize + threadIndex
      const uint32_t linearizedIndex(static_cast<uint32_t>(
          alpaka::idx::getIdx<alpaka::Grid, alpaka::Threads>(acc)[0]));

      typename GetIterator<T, TElem, TAcc>::Iterator it(
          acc, source, linearizedIndex, gridDimension * blockSize, n);

      T result = 0;  // suppresses compiler warnings

      if (threadIndex < n)
        result = unary_func(*(it++));  // avoids using the
                                       // neutral element of specific

      // --------
      // Level 1: grid reduce, reading from global memory
      // --------

      // reduce per thread with increased ILP by 4x unrolling sum.
      // the thread of our block reduces its 4 grid-neighbored threads and
      // advances by grid-striding loop (maybe 128bit load improve perf)

      while (it + 3 < it.end()) {
        result = binary_func(
            binary_func(
                binary_func(
                    result,
                    binary_func(unary_func(*it), unary_func(*(it + 1)))),
                unary_func(*(it + 2))),
            unary_func(*(it + 3)));
        it += 4;
      }

      // doing the remaining blocks
      while (it < it.end()) result = binary_func(result, unary_func(*(it++)));

      if (threadIndex < n) sdata[threadIndex] = result;

      alpaka::block::sync::syncBlockThreads(acc);

      // --------
      // Level 2: block + warp reduce, reading from shared memory
      // --------

      ALPAKA_UNROLL()
      for (uint32_t currentBlockSize = blockSize,
                    currentBlockSizeUp =
                        (blockSize + 1) / 2;  // ceil(TBlockSize/2.0)
           currentBlockSize > 1;
           currentBlockSize            = currentBlockSize / 2,
                    currentBlockSizeUp = (currentBlockSize + 1) /
                                         2)  // ceil(currentBlockSize/2.0)
      {
        bool cond =
            threadIndex < currentBlockSizeUp  // only first half of block
                                              // is working
            && (threadIndex + currentBlockSizeUp) <
                   blockSize  // index for second half must be in bounds
            &&
            (blockIndex * blockSize + threadIndex + currentBlockSizeUp) < n &&
            threadIndex <
                n;  // if elem in second half has been initialized before

        if (cond)
          sdata[threadIndex] = binary_func(
              sdata[threadIndex], sdata[threadIndex + currentBlockSizeUp]);

        alpaka::block::sync::syncBlockThreads(acc);
      }

      // store block result to gmem
      if (threadIndex == 0 && threadIndex < n)
        destination[blockIndex] = sdata[0];
    }
};

template <
    typename T,
    typename DevHost,
    typename Entity,
    typename QueueAcc,
    typename HostBuf,
    typename TFunc>
T mreduce(
    DevHost  devHost,
    Entity   entity,
    QueueAcc queue,
    uint64_t n,
    HostBuf  hostMemory,
    TFunc    func)
{
  using dev_t = typename Entity::dev_t;
  using acc_t = typename Entity::acc_t;

  const auto device = entity.device();

  static constexpr uint64_t blockSize  = 256;
  uint32_t blockCount = static_cast<uint32_t>(
      alpaka::acc::getAccDevProps<acc_t, dev_t>(device).m_multiProcessorCount *
      8);
  uint32_t maxBlockCount = static_cast<uint32_t>(
      (((n + 1) / 2) - 1) / blockSize + 1);  // ceil(ceil(n/2.0)/blockSize)

  if (blockCount > maxBlockCount)
      blockCount = maxBlockCount;

  using Dim    = alpaka::dim::DimInt<1u>;
  using Idx    = uint64_t;
  using Extent = uint64_t;

  alpaka::mem::buf::Buf<dev_t, T, Dim, Extent> sourceDeviceMemory =
      alpaka::mem::buf::alloc<T, Idx>(device, n);

  alpaka::mem::buf::Buf<dev_t, T, Dim, Extent> destinationDeviceMemory =
      alpaka::mem::buf::alloc<T, Idx>(
          device, static_cast<Extent>(blockCount));

  // copy the data to the GPU
  alpaka::mem::view::copy(queue, sourceDeviceMemory, hostMemory, n);

  // create kernels with their workdivs
  ReduceKernel<blockSize, T, TFunc> kernel1, kernel2;

  WorkDiv                           workDiv1{static_cast<Extent>(blockCount),
                   static_cast<Extent>(blockSize),
                   static_cast<Extent>(1)};
  WorkDiv                           workDiv2{static_cast<Extent>(1),
                   static_cast<Extent>(blockSize),
                   static_cast<Extent>(1)};

  // create main reduction kernel execution task
  auto const taskKernelReduceMain(alpaka::kernel::createTaskKernel<acc_t>(
      workDiv1,
      kernel1,
      alpaka::mem::view::getPtrNative(sourceDeviceMemory),
      alpaka::mem::view::getPtrNative(destinationDeviceMemory),
      n,
      func));

  // create last block reduction kernel execution task
  auto const taskKernelReduceLastBlock(alpaka::kernel::createTaskKernel<acc_t>(
      workDiv2,
      kernel2,
      alpaka::mem::view::getPtrNative(destinationDeviceMemory),
      alpaka::mem::view::getPtrNative(destinationDeviceMemory),
      blockCount,
      func));

  // enqueue both kernel execution tasks
  alpaka::queue::enqueue(queue, taskKernelReduceMain);
  alpaka::queue::enqueue(queue, taskKernelReduceLastBlock);

  //  download result from GPU
  T    resultGpuHost;
  auto resultGpuDevice =
      alpaka::mem::view::ViewPlainPtr<DevHost, T, Dim, Idx>(
          &resultGpuHost, devHost, static_cast<Extent>(blockSize));

  alpaka::mem::view::copy(queue, resultGpuDevice, destinationDeviceMemory, 1);

  return resultGpuHost;
}

namespace alpaka {
namespace kernel {
namespace traits {
template <typename T, typename UnaryFunc, typename TFunc, typename TAcc>
struct BlockSharedMemDynSizeBytes<DReduceKernel<T, UnaryFunc, TFunc>, TAcc> {
  template <typename TVec, typename TElem, typename TIdx>
  ALPAKA_FN_HOST_ACC static auto getBlockSharedMemDynSizeBytes(
      DReduceKernel<T, UnaryFunc, TFunc> const &kern,
      TVec const &                              blockThreadExtent,
      TVec const &                              threadElemExtent,
      TElem const *const,
      TElem *,
      TIdx const &,
      UnaryFunc,
      TFunc)
  {
		// We have one result per thread
    return static_cast<idx::Idx<TAcc>>(
        sizeof(TElem) * blockThreadExtent.prod());
  }
};
}
}
}

template <
    typename T,
    typename DevHost,
    typename Entity,
    typename QueueAcc,
    typename HostBuf,
    typename UnaryFunc,
    typename BinaryFunc>
void dreduce(
    std::promise<T> *promise,
    DevHost          devHost,
    Entity           entity,
    QueueAcc &       queue,
    uint64_t         n,
    HostBuf          hostMemory,
    UnaryFunc        unary_func,
    BinaryFunc       binary_func)
{
  using dev_t = typename Entity::dev_t;
  using acc_t = typename Entity::acc_t;

  const auto device = entity.device();

  using Dim    = alpaka::dim::DimInt<1u>;
  using Idx    = uint64_t;
  using Extent = uint64_t;

  auto workDiv = dash::MephWorkDiv<Entity>::getOptimal(entity, n);
  auto blockCount =
      alpaka::workdiv::getWorkDiv<alpaka::Grid, alpaka::Blocks>(workDiv)[0];

  alpaka::mem::buf::Buf<dev_t, T, Dim, Extent> destinationDeviceMemory =
      alpaka::mem::buf::alloc<T, Idx>(
          device, static_cast<Extent>(blockCount));

  // copy the data to the GPU
 
  T* input; 
  if(false) { // no unified memory
    alpaka::mem::buf::Buf<dev_t, T, Dim, Extent> sourceDeviceMemory =
      alpaka::mem::buf::alloc<T, Idx>(device, n);
    alpaka::mem::view::copy(queue, sourceDeviceMemory, hostMemory, n);
    input = alpaka::mem::view::getPtrNative(sourceDeviceMemory);
  } else {
    input = alpaka::mem::view::getPtrNative(hostMemory);
#ifdef CUDACC
      auto mdev = queue.m_spQueueImpl->m_dev.m_iDevice;
      cudaMemPrefetchAsync(input, n*sizeof(T), mdev, NULL);
#endif

  }
        

  // create kernels with their workdivs
  //
  DReduceKernel<T, UnaryFunc, BinaryFunc> kernel1;

  auto destination = alpaka::mem::view::getPtrNative(destinationDeviceMemory);
  // create main reduction kernel execution task
  auto const taskKernelReduceMain(alpaka::kernel::createTaskKernel<acc_t>(
      workDiv,
      kernel1,
      input,
      destination,
      n,
      unary_func,
      binary_func));

  // enqueue both kernel execution tasks
  alpaka::queue::enqueue(queue, taskKernelReduceMain);

  std::thread t([devHost, destinationDeviceMemory, blockCount, &queue, promise] {
    //  Block results
    std::vector<T> results;
    results.resize(blockCount);
    auto result_ptr =
        alpaka::mem::view::ViewPlainPtr<DevHost, T, Dim, Idx>(
            results.data(), devHost, static_cast<Extent>(blockCount));

    alpaka::mem::view::copy(
        queue, result_ptr, destinationDeviceMemory, blockCount);
    alpaka::wait::wait(queue);

    T result = 0;
    for (size_t i = 0; i < blockCount; i++) {
      alpaka::wait::wait(queue);
      result += results[i];
    }
    promise->set_value(result);
  });
  t.detach();
}
