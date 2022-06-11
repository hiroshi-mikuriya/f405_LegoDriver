/// @file      peripheral/encoder.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#include "encoder.h"
#include "main.h"
#include "message/msgdef.h"
#include <initializer_list>

namespace
{
msg::EncoderData s_enc{};
}

void initEncoder(void)
{
  constexpr TIM_TypeDef *tims[] = {ENC_MOTOR1_TIM, ENC_MOTOR2_TIM, ENC_NOB1_TIM, ENC_NOB2_TIM, ENC_UPDATE_TIM};
  for (auto tim : tims)
  {
    LL_TIM_SetCounter(tim, 0);
    LL_TIM_EnableCounter(tim);
  }
  LL_TIM_EnableIT_UPDATE(ENC_UPDATE_TIM);
}

void updateEncorderIRQ(void)
{
  constexpr TIM_TypeDef *rotaryTim[MOTOR_COUNT] = {ENC_NOB1_TIM, ENC_NOB2_TIM};
  constexpr TIM_TypeDef *motorTim[MOTOR_COUNT] = {ENC_MOTOR1_TIM, ENC_MOTOR2_TIM};
  static int16_t preRotaryCount[MOTOR_COUNT] = {0};
  static int16_t preMotorCount[MOTOR_COUNT] = {0};
  for (uint32_t i = 0; i < MOTOR_COUNT; ++i)
  {
    {
      int16_t c = static_cast<int16_t>(LL_TIM_GetCounter(rotaryTim[i]));
      int32_t d = c - preRotaryCount[i];
      preRotaryCount[i] = c;
      s_enc.rotary[i] += d;
    }
    {
      int16_t c = static_cast<int16_t>(LL_TIM_GetCounter(motorTim[i]));
      int32_t d = c - preMotorCount[i];
      preMotorCount[i] = c;
      s_enc.motor[i] += d;
      s_enc.motorVelocity[i] = d;
    }
  }
  msg::send(appTaskHandle, msg::ENCODER_DATA_NOTIFY, s_enc);
}

void resetEncoder(uint32_t encoderType)
{
  NVIC_DisableIRQ(ENC_UPDATE_TIM_IRQn);
  if (encoderType & 1)
  {
    s_enc.rotary[0] = 0;
  }
  if (encoderType & 2)
  {
    s_enc.rotary[1] = 0;
  }
  if (encoderType & 4)
  {
    s_enc.motor[0] = 0;
    s_enc.motorVelocity[0] = 0;
  }
  if (encoderType & 8)
  {
    s_enc.motor[1] = 0;
    s_enc.motorVelocity[1] = 0;
  }
  NVIC_EnableIRQ(ENC_UPDATE_TIM_IRQn);
}
