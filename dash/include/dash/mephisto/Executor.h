#pragma once

#include <alpaka/alpaka.hpp>

namespace dash {

template <typename Entity>
struct AlpakaExecutor {
  using acc_t         = typename Entity::acc_t;
  using dev_t         = typename Entity::dev_t;
  using pltf_t        = typename Entity::pltf_t;
  using sync_queue_t  = alpaka::queue::QueueCpuSync;
  using async_queue_t = alpaka::queue::QueueCpuAsync;

  AlpakaExecutor()
    : _sync_queue(sync_queue_t())
  {
  }

  sync_queue_t& sync_queue() {
    return _sync_queue;
  }

  /* async_queue_t& async_queue() { */
  /*   return _async_queue; */
  /* } */

private:
  /* async_queue_t _async_queue; */
  sync_queue_t _sync_queue;
};

}  // namespace dash
