/// @file      common/mutex.hpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include "cmsis_os.h"

namespace mik
{
class Mutex;
template <typename T>
class LockGuard;
} // namespace mik

/// @brief FreeRTOSのミューテックスを扱うクラス
/// @note std::lock_guard から使えるように、関数の分け方を std::mutex に合わせている。
class mik::Mutex
{
  /// @brief コピーコンストラクタ削除
  Mutex(Mutex const &) = delete;
  /// @brief 代入演算子削除
  Mutex &operator=(Mutex const &) = delete;

  /// ミューテックス
  osMutexId m_;
  /// @brief ミューテックス生成 @retval osOK 成功 @retval osErrorOS 失敗
  osStatus create()
  {
    osMutexDef_t def{};
    m_ = osMutexCreate(&def);
    return m_ ? osOK : osErrorOS;
  }

public:
  /// @brief デフォルトコンストラクタ
  Mutex() noexcept : m_(0) { create(); }
  /// @brief moveコンストラクタ @param[in] that 移動元
  Mutex(Mutex &&that) noexcept : m_(that.m_) { that.m_ = 0; }
  /// @brief move演算子 @param[in] that 移動元 @return 自身の参照
  Mutex &operator=(Mutex &&that) noexcept
  {
    if (this != &that)
    {
      remove();
      m_ = that.m_;
      that.m_ = 0;
    }
    return *this;
  }
  /// @brief デストラクタ
  virtual ~Mutex() { remove(); }
  /// @brief ロックを取得する
  void lock() noexcept
  {
    if (m_)
    {
      osMutexWait(m_, osWaitForever);
    }
  }
  /// @brief ロックを手放す
  void unlock() noexcept
  {
    if (m_)
    {
      osMutexRelease(m_);
    }
  }
  /// @brief ロックの取得を試みる @retval true 所有権を取得できた場合 @retval false 所有権が取得できなかった場合
  bool try_lock() noexcept
  {
    if (m_)
    {
      return osOK == osMutexWait(m_, 0);
    }
    return false;
  }
  /// @brief ミューテックス破棄 @return 結果
  osStatus remove()
  {
    osStatus res = osOK;
    if (m_)
    {
      res = osMutexDelete(m_);
    }
    m_ = 0;
    return res;
  }
};

/// @brief コンストラクタで lock を呼び出し、デストラクタで unkock を呼び出すクラス
template <typename T>
class mik::LockGuard
{
  T &m_;

public:
  explicit LockGuard(T &m) : m_(m) { m_.lock(); }
  virtual ~LockGuard() { m_.unlock(); }
};
