/// @file      common/alloc.hpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include <cmsis_os.h>
#include <memory> // std::unique_ptr

namespace mik
{
template <typename T>
class Deleter;

template <typename T>
using UniquePtr = std::unique_ptr<T, Deleter<T>>;

template <typename T, class... Args>
T *alloc(Args... args) noexcept;

template <typename T>
T *allocArray(std::size_t size) noexcept;

template <typename T, class... Args>
UniquePtr<T> makeUnique(Args... args) noexcept;
} // namespace mik

/// @brief RTOSからメモリ確保し、コンストラクタを呼び出しポインタを返す
/// @tparam T メモリ確保する型
/// @tparam Args コンストラクタ引数型
/// @param [in] args コンストラクタ引数
/// @return 確保したポインタ
template <typename T, class... Args>
T *mik::alloc(Args... args) noexcept
{
  void *ptr = pvPortMalloc(sizeof(T));
  return new (ptr) T(args...);
}

/// @brief RTOSからメモリ確保し、コンストラクタを呼ばずにポインタを返す
/// @tparam T メモリ確保する型
/// @param [in] size メモリ確保する数
/// @return 確保したポインタ
template <typename T>
T *mik::allocArray(std::size_t size) noexcept
{
  return static_cast<T *>(pvPortMalloc(size * sizeof(T)));
}

/// @brief RTOSからメモリ確保し、コンストラクタを呼び出したのち、UniquePtr型に変換して返す
/// @tparam T メモリ確保する型
/// @tparam Args コンストラクタ引数型
/// @param [in] args コンストラクタ引数
/// @return 確保したポインタ
template <typename T, class... Args>
mik::UniquePtr<T> mik::makeUnique(Args... args) noexcept
{
  return UniquePtr<T>(alloc<T>(args...));
}

/// @brief allocでメモリ確保したオブジェクトを破棄する
template <typename T = void>
class mik::Deleter
{
public:
  constexpr Deleter() noexcept = default;

  // 別の Deleter オブジェクトから Deleter オブジェクトを構築します。
  // このコンストラクタは U* が T* に暗黙に変換可能な場合にのみ、オーバーロード解決に参加します。
  template <typename U, typename std::enable_if<std::is_convertible<U *, T *>::value, std::nullptr_t>::type = nullptr>
  explicit Deleter(const Deleter<U> &) noexcept
  {
  }
  /// @brief オブジェクト破棄
  /// @param [in] ptr オブジェクトのポインタ
  void operator()(T *ptr) const
  {
    ptr->~T(); // 配置newしているので意図的にデストラクタを呼び出す必要がある
    vPortFree(ptr);
  }
};

/// @brief allocArrayでメモリ確保したオブジェクトを破棄する
template <typename T>
class mik::Deleter<T[]>
{
public:
  constexpr Deleter() noexcept = default;

  // 別の Deleter オブジェクトから Deleter オブジェクトを構築します。
  // このコンストラクタは U(*)[] が T(*)[] に暗黙に変換可能な場合にのみ、オーバーロード解決に参加します。
  template <typename U, typename std::enable_if<std::is_convertible<U (*)[], T (*)[]>::value, std::nullptr_t>::type = nullptr>
  Deleter(const Deleter<U[]> &) noexcept
  {
  }
  /// @brief オブジェクト破棄
  /// @param [in] ptr オブジェクトのポインタ
  void operator()(T *ptr) const { vPortFree(ptr); }
};
