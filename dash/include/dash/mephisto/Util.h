#pragma once

#include <cstddef>

/**
 * Helper to convert std::array to alpaka's Vec
 *
 * alpaka::vec::createVecFromIndexedFn<dim_t, arr_to_vec>(arr)
 */
template <std::size_t n>
struct arr_to_vec {
  template <typename T>
  static constexpr auto create(T &arr)
  {
    return arr[n];
  }
};

template <std::size_t n>
struct unity_vec {
  template <typename T>
  static constexpr auto create(T arg)
  {
    return arg;
  }
};
