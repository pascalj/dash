#pragma once

#include <alpaka/alpaka.hpp>

namespace dash {

template <typename Entity>
struct AlpakaExecutor {
  using acc_t         = typename Entity::acc_t;
  using dev_t         = typename Entity::dev_t;
  using pltf_t        = typename Entity::pltf_t;
#ifdef ALPAKA_ACC_GPU_CUDA_ENABLED
  using sync_queue_t  = alpaka::queue::QueueCudaRtBlocking;
  using async_queue_t = alpaka::queue::QueueCudaRtNonBlocking;
#else
  using sync_queue_t  = alpaka::queue::QueueCpuBlocking;
  using async_queue_t = alpaka::queue::QueueCpuNonBlocking;
#endif

  AlpakaExecutor() {
    for(std::size_t i = 0; i < Entity::total(); i++) {
      _sync_queues.emplace_back(alpaka::pltf::getDevByIdx<pltf_t>(i));
    }
  }

  sync_queue_t& sync_queue(const Entity &entity) {
    return _sync_queues.at(entity.index());
  }

  auto entities() {
    return Entity::all();
  }

  /* async_queue_t& async_queue() { */
  /*   return _async_queue; */
  /* } */

private:
  /* async_queue_t _async_queue; */
  std::vector<sync_queue_t> _sync_queues;
};

template<typename T>
struct is_executor
  : public std::integral_constant<bool, false> {
};

template<typename Entity>
struct is_executor<AlpakaExecutor<Entity>>
  : public std::integral_constant<bool, true> {
};


}  // namespace dash
