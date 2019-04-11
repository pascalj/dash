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

  FN_HOST_ACC LocalArray()
    : m_data{}
  {
  }

  template <typename TInput>
  FN_HOST_ACC LocalArray(TInput inputArr)
  {
    size_t i = 0;
    for (auto extent : inputArr) {
      m_data[i++] = extent;
    }
    }

  /*
    LocalArray() = default;

    constexpr LocalArray(const LocalArray<TType, TSize> &) = default;

    LocalArray(LocalArray<TType, TSize> &&) = default;


    FN_HOST LocalArray(const std::array<TType, TSize>& array) {
        size_t i = 0;
        for(auto extent : array) {
            m_data[i++] = extent;
        }
    }
    */
    FN_HOST_ACC
    LocalArray(std::initializer_list<TType> ilist) {
        size_t i = 0;
        for(auto extent : ilist) {
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

    FN_HOST_ACC iterator begin() const noexcept {
      return iterator(m_data);
    }
    FN_HOST_ACC iterator end() const noexcept {
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
  for (auto const &a : arr) {
    out << a << " ";
  }
  return out;
}

template <typename TType, std::size_t TSize>
bool operator==(
    const std::array<TType, TSize>& lhs, const LocalArray<TType, TSize>& rhs)
{
  for (size_t i = 0; i < TSize; i++) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

template <typename TType, std::size_t TSize>
bool operator==(
    const LocalArray<TType, TSize>& lhs, const LocalArray<TType, TSize>& rhs)
{
  for (size_t i = 0; i < TSize; i++) {
    if (lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

#endif
