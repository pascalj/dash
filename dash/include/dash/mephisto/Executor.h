#pragma once

#include <alpaka/alpaka.hpp>

namespace dash {

template <typename Entity>
struct AlpakaExecutor {
  using acc_t         = typename Entity::acc_t;
  using dev_t         = typename Entity::dev_t;
  using pltf_t        = typename Entity::pltf_t;
  using sync_queue_t  = alpaka::queue::Queue<dev_t, alpaka::queue::Blocking>;
  using async_queue_t =
      alpaka::queue::Queue<dev_t, alpaka::queue::NonBlocking>;



  AlpakaExecutor()
    : _sync_queue(alpaka::pltf::getDevByIdx<pltf_t>(0u)), _async_queue(alpaka::pltf::getDevByIdx<pltf_t>(0u))
  {
  }

  sync_queue_t& sync_queue() {
    return _sync_queue;
  }

  auto entities() {
    return Entity::all();
  }

  async_queue_t& async_queue() {
    return _async_queue;
  }

private:
  async_queue_t _async_queue;
  sync_queue_t _sync_queue;
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
