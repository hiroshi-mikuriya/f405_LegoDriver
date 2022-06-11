/// @file      control/pid.hpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

namespace mik
{
class PID;
}

/// @brief PID制御クラス
/// @see http://www.picfun.com/motor05.html
class mik::PID
{
  PID() = delete;

  const float kp_; ///< 比例項係数
  const float ki_; ///< 積分項係数
  const float kd_; ///< 微分項係数
  float mvn_;      ///< 今回操作量
  float mvn1_;     ///< 前回操作量
  float en_;       ///< 今回の偏差
  float en1_;      ///< 前回の偏差
  float en2_;      ///< 前々回の偏差

  /// @brief 今回値を前回値にコピーする
  void shift()
  {
    mvn1_ = mvn_;
    en2_ = en1_;
    en1_ = en_;
  }

public:
  /// @brief コンストラクタ
  /// @param [in] kp Kp
  /// @param [in] ki Ki
  /// @param [in] kd Kd
  explicit PID(float kp, float ki, float kd) //
      : kp_(kp), ki_(ki), kd_(kd), mvn_(0), mvn1_(0), en_(0), en1_(0), en2_(0)
  {
  }
  /// @brief デストラクタ
  virtual ~PID() {}
  /// @brief 制御値を計算する
  /// @param [in] target 目標値
  /// @param [in] fb フィードバック値
  /// @return 出力値
  float calc(float target, float fb)
  {
    shift();
    en_ = target - fb;
    // ΔMVn = Kp(en-en-1) + Ki en + Kd((en-en-1) - (en-1-en-2))
    float dmvn = kp_ * (en_ - en1_) + ki_ * en_ + kd_ * ((en_ - en1_) - (en1_ - en2_));
    // MVn = MVn-1 + ΔMVn
    return mvn_ = mvn1_ + dmvn;
  }
  /// @brief パラメータ初期化
  void reset()
  {
    mvn_ = 0;
    mvn1_ = 0;
    en_ = 0;
    en1_ = 0;
    en2_ = 0;
  }
};
