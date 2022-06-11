/// @file      device/i2c_device_base.h
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include "peripheral/i2c.h"

namespace mik
{
class I2CDeviceBase;
}

/// @brief I2Cデバイス基底クラス
class mik::I2CDeviceBase
{
  I2C *const i2c_;          ///< I2C通信オブジェクト
  const uint8_t slaveAddr_; ///< スレーブアドレス

protected:
  /// @brief デバイスに書き込む
  /// @param [in] bytes 書き込みデータの先頭ポインタ
  /// @param [in] size 書き込むバイト数
  /// @retval true 成功
  /// @retval false 失敗
  bool write(uint8_t const *bytes, uint32_t size) const noexcept { return i2c_->write(slaveAddr_, bytes, size) == I2C::OK; }
  /// @brief デバイスから読み込む
  /// @param [in] buf 読み込んだデータを格納するバッファ
  /// @param [in] size 読み込むバイト数
  /// @retval true 成功
  /// @retval false 失敗
  bool readOnly(uint8_t *buffer, uint32_t size) const noexcept { return i2c_->read(slaveAddr_, buffer, size) == I2C::OK; }
  /// @brief レジスタアドレスを書き込んだのち、デバイスから読み込む
  /// @param [in] reg 読み込み先のレジスタ番号
  /// @param [in] buf 読み込んだデータを格納するバッファ
  /// @param [in] size 読み込むバイト数
  /// @retval true 成功
  /// @retval false 失敗
  bool read(uint8_t reg, uint8_t *buffer, uint32_t size) const noexcept { return write(&reg, sizeof(reg)) && readOnly(buffer, size); }

public:
  /// @brief コンストラクタ
  /// @param [in] i2c I2C通信オブジェクト
  /// @param [in] slaveAddr スレーブアドレス
  explicit I2CDeviceBase(I2C *i2c, uint8_t slaveAddr) noexcept : i2c_(i2c), slaveAddr_(slaveAddr << 1) {}
  /// @brief デストラクタ
  virtual ~I2CDeviceBase() {}
  /// @brief デバイス状態を取得する @retval true 正常 @retval false 異常あり
  virtual explicit operator bool() const noexcept = 0;
};