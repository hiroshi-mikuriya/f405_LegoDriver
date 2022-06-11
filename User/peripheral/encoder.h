/// @file      peripheral/encoder.h
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include "constants.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /// @brief エンコーダ初期化
  void initEncoder(void);

  /// @brief エンコーダ更新タイマ割り込み
  void updateEncorderIRQ(void);

  /// @brief エンコーダ値をリセットする
  /// @param [in] encoderType
  ///   @arg 1 ノブ１
  ///   @arg 2 ノブ２
  ///   @arg 4 モータ１
  ///   @arg 8 モータ２
  void resetEncoder(uint32_t encoderType);
#ifdef __cplusplus
}
#endif
