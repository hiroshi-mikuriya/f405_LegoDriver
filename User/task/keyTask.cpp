/// @file      task/keyTask.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#include "main.h"
#include "message/msgdef.h"

namespace
{
constexpr uint32_t KEY_COUNT = 5;
constexpr msg::ID KEY_IDS[KEY_COUNT] = {
    msg::KEY_MOTOR1_LEFT,  //
    msg::KEY_MOTOR1_RIGHT, //
    msg::KEY_MOTOR2_LEFT,  //
    msg::KEY_MOTOR2_RIGHT, //
    msg::KEY_USR_BTN,      //
};
struct KeyLevels
{
  bool level[KEY_COUNT];
};
KeyLevels getKeyLevels()
{
  KeyLevels dst{};
  dst.level[0] = LL_GPIO_IsInputPinSet(MOTOR1_SWT1_GPIO_Port, MOTOR1_SWT2_Pin) != 0;
  dst.level[1] = LL_GPIO_IsInputPinSet(MOTOR1_SWT2_GPIO_Port, MOTOR1_SWT1_Pin) != 0;
  dst.level[2] = LL_GPIO_IsInputPinSet(MOTOR2_SWT1_GPIO_Port, MOTOR2_SWT2_Pin) != 0;
  dst.level[3] = LL_GPIO_IsInputPinSet(MOTOR2_SWT2_GPIO_Port, MOTOR2_SWT1_Pin) != 0;
  dst.level[4] = LL_GPIO_IsInputPinSet(USR_BTN_GPIO_Port, USR_BTN_Pin) != 0;
  return dst;
}
} // namespace

extern "C"
{
  void keyTaskProc(void *argument)
  {
    auto pre = getKeyLevels();
    for (;;)
    {
      osDelay(10);
      auto cur = getKeyLevels();
      for (uint32_t i = 0; i < KEY_COUNT; ++i)
      {
        if (pre.level[i] && !cur.level[i])
        {
          msg::send(appTaskHandle, KEY_IDS[i]);
        }
      }
      pre = cur;
    }
  }
}