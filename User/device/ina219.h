/// @file      device/ina219.h
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include "peripheral/i2c.h"

namespace mik
{
class INA219;
constexpr uint8_t INA219_SLAVE_ADDR0 = 0x41 << 1;
constexpr uint8_t INA219_SLAVE_ADDR1 = 0x40 << 1;
} // namespace mik

/// @brief INA219制御クラス（電流センサ）
class mik::INA219
{
  INA219() = delete;                          ///< デフォルトコンストラクタ削除
  INA219(INA219 const &) = delete;            ///< コピーコンストラクタ削除
  INA219 &operator=(INA219 const &) = delete; ///< 代入演算子削除
  INA219(INA219 &&) = delete;                 ///< moveコンストラクタ削除
  INA219 &operator=(INA219 &&) = delete;      ///< move演算子削除

  I2C *i2c_;          ///< I2C通信クラス
  uint8_t slaveAddr_; ///< スレーブアドレス

  uint32_t calValue_;
  uint32_t currentDivider_mA_;
  uint32_t powerMultiplier_mW_;

  /// @brief キャリブレーションレジスタを更新する
  /// @return I2C通信結果
  I2C::Result updateCalibration();
  /// @brief レジスタに値を書き込む
  /// @param [in] reg レジスタ
  /// @param [in] v 値
  /// @return I2C通信結果
  I2C::Result writeReg(uint8_t reg, uint16_t v);
  /// @brief レジスタを読み込む
  /// @param [in] reg レジスタ
  /// @param [out] v 値
  /// @return I2C通信結果
  I2C::Result readReg(uint8_t reg, uint16_t &v);

  I2C::Result setCalibration_32V_2A();
  I2C::Result setCalibration_32V_1A();
  I2C::Result setCalibration_16V_400mA();

public:
  /// @brief コンストラクタ
  /// @param [in] i2c I2C通信オブジェクト
  /// @param [in] slaveAddr スレーブアドレス
  explicit INA219(I2C *i2c, uint8_t slaveAddr);
  /// @brief デストラクタ
  virtual ~INA219();
  /// @brief 初期化
  /// @return I2C通信結果
  I2C::Result init();
  /// @brief 電流値を取得する
  /// @param [out] v 電流値
  /// @return I2C通信結果
  I2C::Result getShuntCurrent(float &v);
  /// @brief バス電圧を取得する
  /// @param [out] v バス電圧
  /// @return I2C通信結果
  I2C::Result getBusVoltage(float &v);
  /// @brief シャント電圧を取得する
  /// @param [out] v シャント電圧
  /// @return I2C通信結果
  I2C::Result getShuntVoltage(float &v);
};
