#pragma once

#include <unordered_map>
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

  AlpakaExecutor() = default;

  sync_queue_t& sync_queue(const Entity &entity) {
    auto idx = entity.index();
    if (!_sync_queues.count(idx)) {
      _sync_queues.emplace(idx, entity.device());
    }
    return _sync_queues.at(idx);
  }

  async_queue_t& async_queue(Entity &entity) {
    const auto idx = entity.index();
    if (!_async_queues.count(idx)) {
      _async_queues.emplace(idx, entity.device());
    }
    return _async_queues.at(idx);
  }

  auto entities() {
    return Entity::all();
  }

private:
  std::unordered_map<int, async_queue_t> _async_queues;
  std::unordered_map<int, sync_queue_t> _sync_queues;
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
