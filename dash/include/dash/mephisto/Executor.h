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
  using host_queue_t = alpaka::queue::Queue<dev_t, alpaka::queue::NonBlocking>;

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

  auto host_entity() {
    constexpr auto dim = Entity::NDim;
    using host_entity_t = CpuOmp4Entity<dim>;
    return host_entity_t{0};
  }

  host_queue_t& host_queue() {
    if(host_queue == nullptr) {
      _host_queue = std::make_unique(host_queue_t{0});
    }
    return *_host_queue;
  }

private:
  std::unordered_map<int, async_queue_t> _async_queues;
  std::unordered_map<int, sync_queue_t> _sync_queues;
  std::unique_ptr<host_queue_t> _host_queue;
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
