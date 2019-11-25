#pragma once

#include <alpaka/alpaka.hpp>
#include <cstddef>

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

#ifdef ALPAKA_ACC_CPU_B_SEQ_T_SEQ_ENABLED
template<std::size_t Size>
using CpuSerialEntity = Entity<Size, alpaka::acc::AccCpuSerial>;
#endif

#ifdef ALPAKA_ACC_CPU_B_SEQ_T_THREADS_ENABLED
template<std::size_t Size>
using CpuThreadEntity = Entity<Size, alpaka::acc::AccCpuThreads>;
#endif


#ifdef ALPAKA_ACC_GPU_CUDA_ENABLED
template<std::size_t Size>
using CudaEntity = Entity<Size, alpaka::acc::AccGpuCudaRt>;
#endif

}  // namespace dash
