/// @file      device/ssd1306.h
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include "application.h"
#include "common/alloc.hpp"
#include "i2c_device_base.hpp"

namespace mik
{
class SSD1306;
constexpr uint8_t SSD1306_SLAVE_ADDR0 = 0x3C;
constexpr uint8_t SSD1306_SLAVE_ADDR1 = 0x3D;
} // namespace mik

/// @brief SSD1306制御クラス（OLED）
class mik::SSD1306 : public mik::I2CDeviceBase
{
  SSD1306() = delete;                           ///< デフォルトコンストラクタ削除
  SSD1306(SSD1306 const &) = delete;            ///< コピーコンストラクタ削除
  SSD1306 &operator=(SSD1306 const &) = delete; ///< 代入演算子削除
  SSD1306(SSD1306 &&) = delete;                 ///< moveコンストラクタ削除
  SSD1306 &operator=(SSD1306 &&) = delete;      ///< move演算子削除

public:
  static constexpr uint8_t WIDTH = 128;                    ///< 横ピクセル数
  static constexpr uint8_t HEIGHT = 64;                    ///< 縦ピクセル数
  static constexpr uint8_t PAGE = 8;                       ///< ページ数
  static constexpr uint32_t BUF_SIZE = WIDTH * HEIGHT / 8; ///< 画面の全ピクセルデータをバッファするのに必要なサイズ

private:
  /// 表示用バッファ
  uint8_t buffer_[BUF_SIZE + 1];
  /// 通信可否
  const bool ok_;
  /// @brief LCD初期化
  /// @retval true 通信成功
  /// @retval false 通信失敗
  bool init() const noexcept;
  /// @brief 全ページ分のバッファをOLEDに書き込む
  /// @retval true 通信成功
  /// @retval false 通信失敗
  bool sendBufferToDevice() noexcept;
  /// @brief 画面表示を更新する
  /// @param [in] motor モータ
  /// @param [in] i モータID
  void writeBuffer(Motor const &motor, uint32_t i);

public:
  /// @brief コンストラクタ
  /// @param [in] i2c I2C通信オブジェクト
  /// @param [in] slaveAddr スレーブアドレス
  explicit SSD1306(I2C *i2c, uint8_t slaveAddr) noexcept;
  /// @brief デストラクタ
  virtual ~SSD1306() {}
  /// @brief 初期化成功・失敗を取得する
  /// @retval true 成功
  /// @retval false 失敗
  explicit operator bool() const noexcept override { return ok_; };
  /// @brief 画面表示を更新する
  /// @param [in] app アプリケーション
  void update(Application const *app);
};
