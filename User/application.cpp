/// @file      application.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#include "application.h"
#include "message/msgdef.h"
#include "peripheral/encoder.h"
#include <algorithm>

mik::Application::Application()        //
    : motor0_(PWM_TIM,                 //
              LL_TIM_OC_SetCompareCH1, //
              {INA1_Pin,               //
               INA1_GPIO_Port},        //
              {INA2_Pin,               //
               INA2_GPIO_Port},        //
              {MOTOR1_LED_Pin,         //
               MOTOR1_LED_GPIO_Port}), //
      motor1_(PWM_TIM,                 //
              LL_TIM_OC_SetCompareCH2, //
              {INB1_Pin,               //
               INB1_GPIO_Port},        //
              {INB2_Pin,               //
               INB2_GPIO_Port},        //
              {MOTOR2_LED_Pin,         //
               MOTOR2_LED_GPIO_Port})  //
{
  initEncoder();
  LL_TIM_CC_EnableChannel(PWM_TIM, LL_TIM_CHANNEL_CH1);
  LL_TIM_CC_EnableChannel(PWM_TIM, LL_TIM_CHANNEL_CH2);
  LL_TIM_EnableCounter(PWM_TIM);
  LL_TIM_EnableAllOutputs(PWM_TIM);
}
void mik::Application::control()
{
  motor0_.control();
  motor1_.control();
}
void mik::Application::update(msg::Message const *msg)
{
  switch (msg->type)
  {
  case msg::KEY_MOTOR1_LEFT:
    motor(0).changeRunningMode();
    resetEncoder(1 | 4);
    break;
  case msg::KEY_MOTOR1_RIGHT:
    motor(0).changeControlMode();
    resetEncoder(1 | 4);
    break;
  case msg::KEY_MOTOR2_LEFT:
    motor(1).changeRunningMode();
    resetEncoder(2 | 8);
    break;
  case msg::KEY_MOTOR2_RIGHT:
    motor(1).changeControlMode();
    resetEncoder(2 | 8);
    break;
  case msg::KEY_USR_BTN:
    LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    break;
  case msg::ENCODER_DATA_NOTIFY:
  {
    auto enc = *reinterpret_cast<msg::EncoderData const *>(msg->bytes);
    for (uint32_t i = 0; i < MOTOR_COUNT; ++i)
    {
      auto &m = motor(i);
      m.encoder().set(enc.motor[i]);
      m.nob().set(enc.rotary[i]);
      m.setVelocity(enc.motorVelocity[i]);
    }
    control(); // 制御周期を一定にするためここで呼ぶ。（ここだと100Hz）
    break;
  }
  case msg::CURRENT_DATA_NOTIFY:
  {
    auto c = *reinterpret_cast<msg::CurrentData const *>(msg->bytes);
    for (uint32_t i = 0; i < MOTOR_COUNT; ++i)
    {
      auto &m = motor(i);
      m.setCurrent(c.current[i]);
      m.setBusVoltage(c.busVoltage[i]);
      m.setShuntVoltage(c.shuntVoltage[i]);
    }
    break;
  }
  }
}
