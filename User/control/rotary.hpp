/// @file      control/rotary.hpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

namespace mik
{
template <typename T>
class Rotary;
}

/// @brief ロータリーエンコーダクラス
/// @tparam T エンコーダカウンタの型
template <typename T>
class mik::Rotary
{
  T v_; ///< 値
public:
  /// @brief コンストラクタ
  Rotary() : v_(0) {}
  /// @brief デストラクタ
  virtual ~Rotary() {}
  /// @brief 値を設定する
  /// @param [in] v 値
  void set(T v) { v_ = v; }
  /// @brief 値と基準値の差分を取得する
  /// @return 値と基準値の差分
  T get() const { return v_; }
};
