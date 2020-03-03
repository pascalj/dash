#pragma once


#include <alpaka/alpaka.hpp>
#include <cstddef>
#ifdef ALPAKA_ACC_CPU_B_SEQ_T_OMP2_ENABLED
#include <omp.h>
#endif

namespace dash {

/**
 * An entity represents a computing element that can execute code.
 *
 * It is the glue between Alpaka and DASH. Alpaka already supports devices and
 * DASH supports entities. These entities here have some extras added to allow
 * the mapping from DASH-entities to devices and vice versa.
 */
template <
    std::size_t Dim,
    template <typename, typename>
    class AlpakaAcc,
    typename Size = std::size_t>
struct Entity {
  using alpaka_dim_t = alpaka::dim::DimInt<Dim>;
  using acc_t  = AlpakaAcc<alpaka_dim_t, Size>;
  using dev_t  = alpaka::dev::Dev<acc_t>;
  using pltf_t = alpaka::pltf::Pltf<dev_t>;

  constexpr static std::size_t NDim = Dim;

  Entity(std::size_t i)
    : _index(i)
  {
  }

  std::size_t index() const
  {
    return _index;
  }

  static std::size_t total()
  {
    return alpaka::pltf::getDevCount<pltf_t>();
  }

  dev_t device() const
  {
    return dev_t{alpaka::pltf::getDevByIdx<pltf_t>(_index)};
  }

  auto concurrency() const {
    return alpaka::acc::getAccDevProps<acc_t, dev_t>(device()).m_blockThreadCountMax * 4;
  }


  static auto all()
  {
    std::vector<Entity> entities;
    entities.reserve(total());
    for (std::size_t i = 0; i < total(); i++) {
      entities.emplace_back(i);
    }
    return entities;
  }

private:
  std::size_t _index = 0u;
};

template <typename Entity, uint64_t TSize>
static constexpr uint64_t getMaxBlockSize()
{
    return (Entity::acc_t::MaxBlockSize::value > TSize) ? TSize
                                               : Entity::acc_t::MaxBlockSize::value;
}

/**
 * Default work div, only used as fallback
 */
template <typename Entity, typename SFINAE=void>
struct MephWorkDiv {
  static auto getOptimal(const Entity &entity, const size_t nelems)
  {
    using acc_t = typename Entity::acc_t;
    return alpaka::workdiv::
        WorkDivMembers<typename Entity::alpaka_dim_t, size_t>(
            alpaka::workdiv::getValidWorkDiv<acc_t>(
                entity.device(), nelems, alpaka::idx::Idx<size_t>(1), false));
  }
};

template <typename Entity>
struct BalancedThreadStrategy {
  static auto getOptimal(const Entity &entity, const size_t nelems)
  {
    using vec_t =
        alpaka::vec::Vec<typename Entity::alpaka_dim_t, std::size_t>;
    // For threads, we want a small amount of blocks with a large amount of
    // elements per thread.
    auto blockThreadExtent = vec_t::all(omp_get_max_threads());
    vec_t threadElementExtent((nelems / blockThreadExtent[0]) + 1);
    auto gridBlockExtent = vec_t::all(1);
    return alpaka::workdiv::
        WorkDivMembers<typename Entity::alpaka_dim_t, std::size_t>(
            gridBlockExtent, blockThreadExtent, threadElementExtent);
  }
};

/**
 * Convenience entities
 */
#ifdef ALPAKA_ACC_CPU_B_SEQ_T_SEQ_ENABLED
template<std::size_t Size>
using CpuSerialEntity = Entity<Size, alpaka::acc::AccCpuSerial>;
#endif

#ifdef ALPAKA_ACC_CPU_B_SEQ_T_THREADS_ENABLED
template<std::size_t Size>
using CpuThreadEntity = Entity<Size, alpaka::acc::AccCpuThreads>;
struct MephWorkDiv<CpuSerialEntity<Size>> : public BalancedThreadStrategy<CpuThreadEntity<Size>> {};
#endif

#ifdef ALPAKA_ACC_CPU_B_SEQ_T_OMP2_ENABLED
template <std::size_t Size>
using CpuOmp2Entity = Entity<Size, alpaka::acc::AccCpuOmp2Threads>;

template<std::size_t Size>
struct MephWorkDiv<CpuOmp2Entity<Size>> : public BalancedThreadStrategy<CpuOmp2Entity<Size>> {};

#endif  

#ifdef ALPAKA_ACC_GPU_CUDA_ENABLED
template<std::size_t Size>
using CudaEntity = Entity<Size, alpaka::acc::AccGpuUniformCudaHipRt>;
#endif

}  // namespace dash
