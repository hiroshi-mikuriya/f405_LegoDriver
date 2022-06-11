/// @file      control/motor.h
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include "gpio.hpp"
#include "main.h"
#include "pid.hpp"
#include "rotary.hpp"

namespace mik
{
class Motor;

/// @brief モータモード定義
enum MotorMode
{
  POSITION = 0, ///< 位置制御モード
  VELOCITY,     ///< 速度制御モード
  MODE_COUNT,   ///< モード総数
};
} // namespace mik

/// @brief モータクラス
class mik::Motor
{
  Motor() = delete;                         ///< デフォルトコンストラクタ削除
  Motor(Motor const &) = delete;            ///< コピーコンストラクタ削除
  Motor(Motor &&) = delete;                 ///< 代入演算子削除
  Motor &operator=(Motor const &) = delete; ///< moveコンストラクタ削除
  Motor &operator=(Motor &&) = delete;      ///< move演算子削除

  /// PWM信号を送る関数型
  typedef void (*setPwmProc)(TIM_TypeDef *, uint32_t);

  TIM_TypeDef *pwmTim_;     ///< PWM TIM
  setPwmProc setPwm_;       ///< モータドライバにPWM信号を送る関数
  Gpio in1_;                ///< モータドライバに回転方向を指示するGPIO1
  Gpio in2_;                ///< モータドライバに回転方向を指示するGPIO2
  Gpio led_;                ///< LED制御GPIO
  Rotary<int32_t> encoder_; ///< エンコーダ値
  Rotary<int32_t> nob_;     ///< ノブの回転位置
  bool running_;            ///< 稼働状態 @arg true 稼働中 @arg false 停止中
  MotorMode mode_;          ///< モータモード
  float power_;             ///< PWM制御の比率（-1.0 〜 1.0）
  float current_;           ///< 電流値
  float busVoltage_;        ///< バス電圧
  float shuntVoltage_;      ///< シャント電圧
  int32_t velocity_;        ///< 速度
  PID velocityPID_;         ///< 速度制御のPID制御計算機

  /// @brief 位置制御する
  void controlPosition();
  /// @brief 速度制御する
  /// @param [in] targetVelocity 目標速度
  void controlVelocity(int32_t targetVelocity);
  /// @brief モード切り替えリセット等
  void reset();

public:
  /// @brief コンストラクタ
  /// @param [in] pwmTim PWM TIM
  /// @param [in] setPwm モータドライバにPWM信号を送る関数
  /// @param [in] in1 モータドライバに回転方向を指示するGPIO1
  /// @param [in] in2 モータドライバに回転方向を指示するGPIO2
  /// @param [in] led LEDを制御するGPIO
  explicit Motor(TIM_TypeDef *pwmTim, //
                 setPwmProc setPwm,   //
                 Gpio const &in1,     //
                 Gpio const &in2,     //
                 Gpio const &led);
  /// @brief デストラクタ
  virtual ~Motor() {}
  /// @brief 電流値を設定する
  /// @param [in] cur 電流値
  void setCurrent(float cur) { current_ = cur; }
  /// @brief 電流値を取得する
  /// @return 電流値
  float getCurrent() const { return current_; }
  /// @brief バス電圧値を設定する
  /// @param [in] v バス電圧値
  void setBusVoltage(float v) { busVoltage_ = v; }
  /// @brief バス電圧値を取得する
  /// @return バス電圧値
  float getBusVoltage() const { return busVoltage_; }
  /// @brief シャント電圧値を設定する
  /// @param [in] v シャント電圧値
  void setShuntVoltage(float v) { shuntVoltage_ = v; }
  /// @brief シャント電圧値を取得する
  /// @return シャント電圧値
  float getShuntVoltage() const { return shuntVoltage_; }
  /// @brief 速度を設定する
  /// @param [in] velocity  速度
  void setVelocity(int32_t velocity) { velocity_ = velocity; }
  /// @brief 速度を取得する
  /// @return 速度
  int32_t getVelocity() const { return velocity_; }
  /// @brief 稼働状態を変更する
  void changeRunningMode();
  /// @brief 制御状態を変更する
  void changeControlMode();
  /// @brief 稼働状態を取得する
  /// @retval true 稼働中
  /// @retval false 停止中
  bool isRunning() const { return running_; }
  /// @brief モードを取得する
  /// @return モード
  MotorMode mode() const { return mode_; }
  /// @brief エンコーダを取得する @return エンコーダ
  Rotary<int32_t> &encoder() { return encoder_; }
  /// @brief エンコーダを取得する @return エンコーダ
  Rotary<int32_t> const &encoder() const { return encoder_; }
  /// @brief ノブを取得する @return ノブ
  Rotary<int32_t> &nob() { return nob_; }
  /// @brief ノブを取得する @return ノブ
  Rotary<int32_t> const &nob() const { return nob_; }
  /// @brief モータ制御する（定期的に呼び出すこと）
  void control();
};