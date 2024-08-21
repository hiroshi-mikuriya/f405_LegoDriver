/// @file      device/ssd1306.h
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2024 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include "application.h"
#include "peripheral/i2c.h"

namespace mik
{
class SSD1306;
constexpr uint8_t SSD1306_SLAVE_ADDR0 = 0x3C << 1; // 0x78
constexpr uint8_t SSD1306_SLAVE_ADDR1 = 0x3D << 1; // 0x7A
} // namespace mik

/// @brief SSD1306(0.96インチOLED)制御クラス
/// @see https://monoedge.net/raspi-ssd1306/
class mik::SSD1306
{
  SSD1306() = delete;                           ///< デフォルトコンストラクタ削除
  SSD1306(SSD1306 const &) = delete;            ///< コピーコンストラクタ削除
  SSD1306 &operator=(SSD1306 const &) = delete; ///< 代入演算子削除
  SSD1306(SSD1306 &&) = delete;                 ///< moveコンストラクタ削除
  SSD1306 &operator=(SSD1306 &&) = delete;      ///< move演算子削除

  I2C *i2c_;          ///< I2C通信クラス
  uint8_t slaveAddr_; ///< スレーブアドレス
  uint8_t *buffer_;   ///< 表示用バッファ

  /// @brief 全ページ分のバッファをOLEDに書き込む
  /// @return I2C通信結果
  I2C::Result sendBufferToDevice();
  /// @brief 画面表示を更新する
  /// @param [in] motor モータ
  /// @param [in] i モータID
  void writeBuffer(Motor const &motor, uint32_t i);

public:
  /// @brief コンストラクタ
  /// @param [in] i2c I2C通信オブジェクト
  /// @param [in] slaveAddr スレーブアドレス
  explicit SSD1306(I2C *i2c, uint8_t slaveAddr);
  /// @brief デストラクタ
  virtual ~SSD1306();
  /// @brief LCD初期化
  /// @return I2C通信結果
  I2C::Result init();
  /// @brief 画面を黒くする
  /// @return I2C通信結果
  I2C::Result black();
  /// @brief 画面を白くする
  /// @return I2C通信結果
  I2C::Result white();
  /// @brief 文字列を表示する
  /// @param [in] txt 表示する文字列
  /// @return I2C通信結果
  I2C::Result showText(const char *txt);
  /// @brief 画面表示を更新する
  /// @param [in] app アプリケーション
  /// @return I2C通信結果
  I2C::Result update(Application const *app);
};
