/// @file      message/msgdef.h
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include "constants.h"
#include "msglib.h"

namespace msg
{
namespace cat
{
constexpr uint32_t SHIFT = 12;
constexpr ID KEY = 1 << SHIFT;
constexpr ID USB = 2 << SHIFT;
constexpr ID PERIPH = 3 << SHIFT;
constexpr ID SYSTEM = 4 << SHIFT;
} // namespace cat

constexpr ID KEY_MOTOR1_LEFT = 0 | cat::KEY;        ///< モータ1左キー押下
constexpr ID KEY_MOTOR1_RIGHT = 1 | cat::KEY;       ///< モータ1右キー押下
constexpr ID KEY_MOTOR2_LEFT = 2 | cat::KEY;        ///< モータ2左キー押下
constexpr ID KEY_MOTOR2_RIGHT = 3 | cat::KEY;       ///< モータ2右キー押下
constexpr ID KEY_USR_BTN = 4 | cat::KEY;            ///< ユーザボタン押下
constexpr ID USB_TX_REQ = 0 | cat::USB;             ///< USB送信要求
constexpr ID ENCODER_DATA_NOTIFY = 0 | cat::PERIPH; ///< エンコーダデータ通知
constexpr ID CURRENT_DATA_NOTIFY = 1 | cat::PERIPH; ///< 電流値通知
constexpr ID APP_POINTER_NOTIFY = 0 | cat::SYSTEM;  ///< アプリケーションインスタンスポインタ通知

/// @brief エンコーダデータ通知 の付随データ
struct EncoderData
{
  int32_t rotary[MOTOR_COUNT];
  int32_t motor[MOTOR_COUNT];
  int32_t motorVelocity[MOTOR_COUNT];
};
/// @brief 電流値通知 の付随データ
struct CurrentData
{
  float current[MOTOR_COUNT];
  float busVoltage[MOTOR_COUNT];
  float shuntVoltage[MOTOR_COUNT];
};
/// @brief アプリケーションインスタンスポインタ通知 の付随データ
struct AppPointer
{
  void *app;
};
} // namespace msg