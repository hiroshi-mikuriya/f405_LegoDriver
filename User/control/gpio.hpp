/// @file      control/gpio.hpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include "stm32f4xx_ll_gpio.h"

namespace mik
{
struct Gpio;
}

/// @brief GPIO制御クラス
struct mik::Gpio
{
  uint32_t pin;       ///< ピン番号
  GPIO_TypeDef *port; ///< ポート種類

  /// @brief 信号レベルをHIGHにする
  /// @note 出力ピンのみ有効
  void high() const { LL_GPIO_SetOutputPin(port, pin); }
  /// @brief 信号レベルをLOWにする
  /// @note 出力ピンのみ有効
  void low() const { LL_GPIO_ResetOutputPin(port, pin); }
  /// @brief 信号レベルをトグルする
  /// @note 出力ピンのみ有効
  void toggle() const { LL_GPIO_TogglePin(port, pin); }
  /// @brief 信号レベルを取得する
  /// @retval true HIGH
  /// @retval false LOW
  /// @note 入力ピンのみ有効
  bool level() const { return LL_GPIO_IsInputPinSet(port, pin) != 0; }
};
