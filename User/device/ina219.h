/// @file      device/ina219.h
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include "i2c_device_base.hpp"

namespace mik
{
class INA219;
constexpr uint8_t INA219_SLAVE_ADDR0 = 0x41;
constexpr uint8_t INA219_SLAVE_ADDR1 = 0x40;
} // namespace mik

/// @brief INA219制御クラス（電流センサ）
class mik::INA219 : public I2CDeviceBase
{
  uint32_t calValue_;
  uint32_t currentDivider_mA_;
  uint32_t powerMultiplier_mW_;
  bool ok_;

  /// @brief キャリブレーションレジスタを更新する
  /// @retval true 通信成功
  /// @retval false 通信失敗
  bool updateCalibration() const;
  /// @brief レジスタに値を書き込む
  /// @param [in] reg レジスタ
  /// @param [in] v 値
  /// @retval true 通信成功
  /// @retval false 通信失敗
  bool writeReg(uint8_t reg, uint16_t v) const;
  /// @brief レジスタを読み込む
  /// @param [in] reg レジスタ
  /// @param [out] v 値
  /// @retval true 通信成功
  /// @retval false 通信失敗
  bool readReg(uint8_t reg, uint16_t &v) const;

  bool setCalibration_32V_2A();
  bool setCalibration_32V_1A();
  bool setCalibration_16V_400mA();

public:
  /// @brief コンストラクタ
  /// @param [in] i2c I2C通信オブジェクト
  /// @param [in] slaveAddr スレーブアドレス
  explicit INA219(I2C *i2c, uint8_t slaveAddr);
  /// @brief デストラクタ
  virtual ~INA219() {}
  /// @brief 電流値を取得する
  /// @return 電流値
  float getShuntCurrent() const;
  /// @brief バス電圧を取得する
  /// @return バス電圧
  float getBusVoltage() const;
  /// @brief シャント電圧を取得する
  /// @return シャント電圧
  float getShuntVoltage() const;
  /// @brief デバイス状態を取得する @retval true 正常 @retval false 異常あり
  explicit operator bool() const noexcept { return ok_; }
};
