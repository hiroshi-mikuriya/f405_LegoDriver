/// @file      control/motor.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#include "motor.h"
#include <algorithm>

namespace
{
constexpr float KP_VELOCITY_CTRL = 0.5f; ///< 速度制御のP制御比率
constexpr float KI_VELOCITY_CTRL = 0.0f; ///< 速度制御のI制御比率
constexpr float KD_VELOCITY_CTRL = 0.5f; ///< 速度制御のD制御比率

/// @brief 値を範囲内に収める
/// @tparam T 値の型
/// @param [in] min 最小値
/// @param [inout] v 値
/// @param [in] max 最大値
template <typename T>
inline void compress(T const &min, T &v, T const &max)
{
  if (v < min)
  {
    v = min;
  }
  if (max < v)
  {
    v = max;
  }
}
} // namespace

void mik::Motor::controlPosition()
{
  int32_t d = encoder_.get() - nob_.get();
  if (0 < d)
  {
    controlVelocity(-2);
  }
  else if (d < 0)
  {
    controlVelocity(2);
  }
  else
  {
    controlVelocity(0);
  }
}
void mik::Motor::controlVelocity(int32_t targetVelocity)
{
  power_ = velocityPID_.calc(targetVelocity, velocity_);
}
void mik::Motor::reset()
{
  if (running_)
  {
    led_.high();
    velocityPID_.reset();
  }
  else
  {
    led_.low();
  }
}

mik::Motor::Motor(                   //
    TIM_TypeDef *pwmTim,             //
    setPwmProc setPwm,               //
    Gpio const &in1,                 //
    Gpio const &in2,                 //
    Gpio const &led)                 //
    : pwmTim_(pwmTim),               //
      setPwm_(setPwm),               //
      in1_(in1),                     //
      in2_(in2),                     //
      led_(led),                     //
      encoder_(),                    //
      nob_(),                        //
      running_(false),               //
      mode_(POSITION),               //
      power_(0),                     //
      current_(0),                   //
      busVoltage_(0),                //
      velocity_(0),                  //
      velocityPID_(KP_VELOCITY_CTRL, //
                   KI_VELOCITY_CTRL, //
                   KD_VELOCITY_CTRL) //
{
  reset();
}
void mik::Motor::changeRunningMode()
{
  running_ = !running_;
  reset();
}
void mik::Motor::changeControlMode()
{
  mode_ = static_cast<MotorMode>((mode_ + 1) % MODE_COUNT);
  reset();
}
void mik::Motor::control()
{
  if (running_)
  {
    switch (mode_)
    {
    case POSITION:
      controlPosition();
      break;
    case VELOCITY:
      controlVelocity(nob_.get());
      break;
    default:
      break;
    }
    compress(-1.0f, power_, 1.0f);
    if (0 < power_)
    {
      in1_.high();
      in2_.low();
    }
    else
    {
      in1_.low();
      in2_.high();
    }
  }
  else
  {
    in1_.low();
    in2_.low();
  }
  uint32_t max = LL_TIM_GetAutoReload(pwmTim_);
  uint32_t pwm = static_cast<uint32_t>(max * std::abs(power_));
  setPwm_(pwmTim_, pwm);
}