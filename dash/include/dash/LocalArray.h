#ifndef DASH__LOCAL_ARRAY_H__INCLUDED
#define DASH__LOCAL_ARRAY_H__INCLUDED

#include <cstddef>
#include <dash/Types.h>

/**
 * A simple, local, fixed size array - mostly identical to std::array.
 * However, it works on CUDA because the methods are annotated. This array is
 * intended for use with the patterns instead of std::array, so the code can
 * work on accelerators.
 */
template<
    typename TType,
    std::size_t TSize
>
struct LocalArray
{
  TType m_data[TSize];

  using iterator       = TType*;
  using const_iterator = TType*;

  template <typename TInput>
  FN_HOST LocalArray(TInput inputArr)
  {
    size_t i = 0;
    for (auto extent : inputArr) {
      m_data[i++] = extent;
    }
    }

    LocalArray() = default;

    LocalArray(const LocalArray<TType, TSize> &) = default;

   // LocalArray(LocalArray<TType, TSize> &&) = default;

    FN_HOST_ACC
    LocalArray(std::initializer_list<TType> ilist) {
        size_t i = 0;
        for(auto extent : ilist) {
            m_data[i++] = extent;
        }
    }

    FN_HOST LocalArray(const std::array<TType, TSize>& array) {
        size_t i = 0;
        for(auto extent : array) {
            m_data[i++] = extent;
        }
    }

    template<
        typename TIdx
    >
    constexpr
    FN_HOST_ACC const TType &
    operator[](const TIdx idx) const noexcept {
        return m_data[idx];
    }

    template<
        typename TIdx
    >
    FN_HOST_ACC TType &
    operator[](const TIdx idx) noexcept {
        return m_data[idx];
    }

    constexpr
    FN_HOST_ACC size_t
    size() const { return TSize; }

    constexpr
    FN_HOST_ACC bool
    empty() const noexcept { return size() == 0; }

    iterator begin() noexcept {
      return iterator(m_data);
    }
    iterator end() noexcept {
      return iterator(m_data + TSize);
    }

    operator std::array<TType, TSize>() const
    {
      std::array<TType, TSize> arr;
      std::copy(std::begin(m_data), std::end(m_data), std::begin(arr));
      return arr;
    }

    template <typename T, std::size_t S>
    friend std::ostream& operator<<(std::ostream& out, const LocalArray<T, S>& arr);
};

template <typename T, std::size_t S>
std::ostream& operator<<(std::ostream& out, const LocalArray<T, S>& arr)
{
  for (const auto &a : arr) {
    out << a << " ";
  }
  return out;
}

#endif
