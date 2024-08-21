/// @file      device/ina219.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

// 参考URL：　https://github.com/adafruit/Adafruit_INA219

#include "ina219.h"
#include "common/big_endian.hpp"

using namespace mik;

namespace
{
constexpr uint8_t REG_CONFIG = 0x00;            ///< コンフィグ
constexpr uint8_t REG_SHUNT_VOLTAGE = 0x01;     ///< シャント電圧
constexpr uint8_t REG_BUS_VOLTAGE = 0x02;       ///< バス電圧
constexpr uint8_t REG_POWER_CONSUMPTION = 0x03; ///< 消費電力
constexpr uint8_t REG_SHUNT_CURRENT = 0x04;     ///< シャント電流
constexpr uint8_t REG_CARIBRATION = 0x05;       ///< キャリブレーション

/** bus voltage range values **/
enum
{
  CONFIG_BVOLTAGERANGE_16V = (0x0000), ///< 0-16V Range
  CONFIG_BVOLTAGERANGE_32V = (0x2000), ///< 0-32V Range
};
/** values for gain bits **/
enum
{
  CONFIG_GAIN_1_40MV = (0x0000),  ///< Gain 1, 40mV Range
  CONFIG_GAIN_2_80MV = (0x0800),  ///< Gain 2, 80mV Range
  CONFIG_GAIN_4_160MV = (0x1000), ///< Gain 4, 160mV Range
  CONFIG_GAIN_8_320MV = (0x1800), ///< Gain 8, 320mV Range
};
/** values for bus ADC resolution **/
enum
{
  CONFIG_BADCRES_9BIT = (0x0000),             ///< 9-bit bus res = 0..511
  CONFIG_BADCRES_10BIT = (0x0080),            ///< 10-bit bus res = 0..1023
  CONFIG_BADCRES_11BIT = (0x0100),            ///< 11-bit bus res = 0..2047
  CONFIG_BADCRES_12BIT = (0x0180),            ///< 12-bit bus res = 0..4097
  CONFIG_BADCRES_12BIT_2S_1060US = (0x0480),  ///< 2 x 12-bit bus samples averaged together
  CONFIG_BADCRES_12BIT_4S_2130US = (0x0500),  ///< 4 x 12-bit bus samples averaged together
  CONFIG_BADCRES_12BIT_8S_4260US = (0x0580),  ///< 8 x 12-bit bus samples averaged together
  CONFIG_BADCRES_12BIT_16S_8510US = (0x0600), ///< 16 x 12-bit bus samples averaged together
  CONFIG_BADCRES_12BIT_32S_17MS = (0x0680),   ///< 32 x 12-bit bus samples averaged together
  CONFIG_BADCRES_12BIT_64S_34MS = (0x0700),   ///< 64 x 12-bit bus samples averaged together
  CONFIG_BADCRES_12BIT_128S_69MS = (0x0780),  ///< 128 x 12-bit bus samples averaged together
};
/** values for shunt ADC resolution **/
enum
{
  CONFIG_SADCRES_9BIT_1S_84US = (0x0000),     ///< 1 x 9-bit shunt sample
  CONFIG_SADCRES_10BIT_1S_148US = (0x0008),   ///< 1 x 10-bit shunt sample
  CONFIG_SADCRES_11BIT_1S_276US = (0x0010),   ///< 1 x 11-bit shunt sample
  CONFIG_SADCRES_12BIT_1S_532US = (0x0018),   ///< 1 x 12-bit shunt sample
  CONFIG_SADCRES_12BIT_2S_1060US = (0x0048),  ///< 2 x 12-bit shunt samples averaged together
  CONFIG_SADCRES_12BIT_4S_2130US = (0x0050),  ///< 4 x 12-bit shunt samples averaged together
  CONFIG_SADCRES_12BIT_8S_4260US = (0x0058),  ///< 8 x 12-bit shunt samples averaged together
  CONFIG_SADCRES_12BIT_16S_8510US = (0x0060), ///< 16 x 12-bit shunt samples averaged together
  CONFIG_SADCRES_12BIT_32S_17MS = (0x0068),   ///< 32 x 12-bit shunt samples averaged together
  CONFIG_SADCRES_12BIT_64S_34MS = (0x0070),   ///< 64 x 12-bit shunt samples averaged together
  CONFIG_SADCRES_12BIT_128S_69MS = (0x0078),  ///< 128 x 12-bit shunt samples averaged together
};
/** values for operating mode **/
enum
{
  CONFIG_MODE_POWERDOWN = 0b000,            ///< power down
  CONFIG_MODE_SVOLT_TRIGGERED = 0b001,      ///< shunt voltage triggered
  CONFIG_MODE_BVOLT_TRIGGERED = 0b010,      ///< bus voltage triggered
  CONFIG_MODE_SANDBVOLT_TRIGGERED = 0b011,  ///< shunt and bus voltage triggered
  CONFIG_MODE_ADCOFF = 0b100,               ///< ADC off
  CONFIG_MODE_SVOLT_CONTINUOUS = 0b101,     ///< shunt voltage continuous
  CONFIG_MODE_BVOLT_CONTINUOUS = 0b110,     ///< bus voltage continuous
  CONFIG_MODE_SANDBVOLT_CONTINUOUS = 0b111, ///< shunt and bus voltage continuous
};
} // namespace

I2C::Result INA219::updateCalibration()
{
  return writeReg(REG_CARIBRATION, calValue_);
}

I2C::Result INA219::writeReg(uint8_t reg, uint16_t v)
{
  uint8_t d[3] = {reg};
  BE<uint16_t>::set(d + 1, v);
  return i2c_->write(slaveAddr_, d, sizeof(d));
}

I2C::Result INA219::readReg(uint8_t reg, uint16_t &v)
{
  uint8_t a[2] = {0};
  I2C::Result res = i2c_->readReg(slaveAddr_, reg, a, sizeof(a));
  v = BE<uint16_t>::get(a);
  return res;
}

I2C::Result INA219::setCalibration_32V_2A()
{
  calValue_ = 4096;
  currentDivider_mA_ = 10; // Current LSB = 100uA per bit (1000/100 = 10)
  powerMultiplier_mW_ = 2; // Power LSB = 1mW per bit (2/1)

  constexpr uint16_t config = CONFIG_BVOLTAGERANGE_32V |      //
                              CONFIG_GAIN_8_320MV |           //
                              CONFIG_BADCRES_12BIT |          //
                              CONFIG_SADCRES_12BIT_1S_532US | //
                              CONFIG_MODE_SANDBVOLT_CONTINUOUS;
  I2C::Result res = updateCalibration();
  if (res != I2C::OK)
  {
    return res;
  }
  return writeReg(REG_CONFIG, config);
}

I2C::Result INA219::setCalibration_32V_1A()
{
  calValue_ = 10240;
  currentDivider_mA_ = 25;    // Current LSB = 40uA per bit (1000/40 = 25)
  powerMultiplier_mW_ = 0.8f; // Power LSB = 800uW per bit

  constexpr uint16_t config = CONFIG_BVOLTAGERANGE_32V |      //
                              CONFIG_GAIN_8_320MV |           //
                              CONFIG_BADCRES_12BIT |          //
                              CONFIG_SADCRES_12BIT_1S_532US | //
                              CONFIG_MODE_SANDBVOLT_CONTINUOUS;
  I2C::Result res = updateCalibration();
  if (res != I2C::OK)
  {
    return res;
  }
  return writeReg(REG_CONFIG, config);
}

I2C::Result INA219::setCalibration_16V_400mA()
{
  calValue_ = 8192;
  currentDivider_mA_ = 20;    // Current LSB = 50uA per bit (1000/50 = 20)
  powerMultiplier_mW_ = 1.0f; // Power LSB = 1mW per bit

  constexpr uint16_t config = CONFIG_BVOLTAGERANGE_16V |      //
                              CONFIG_GAIN_1_40MV |            //
                              CONFIG_BADCRES_12BIT |          //
                              CONFIG_SADCRES_12BIT_1S_532US | //
                              CONFIG_MODE_SANDBVOLT_CONTINUOUS;
  I2C::Result res = updateCalibration();
  if (res != I2C::OK)
  {
    return res;
  }
  return writeReg(REG_CONFIG, config);
}

INA219::INA219(I2C *i2c, uint8_t slaveAddr) //
    : i2c_(i2c),                            //
      slaveAddr_(slaveAddr),                //
      calValue_(0),                         //
      currentDivider_mA_(0),                //
      powerMultiplier_mW_(0)                //
{
}

INA219::~INA219() {}

I2C::Result INA219::init()
{
  return setCalibration_32V_2A();
  // return setCalibration_32V_1A();
  // return setCalibration_16V_400mA();
}

I2C::Result INA219::getShuntCurrent(float &v)
{
  updateCalibration();
  uint16_t u = 0;
  I2C::Result res = readReg(REG_SHUNT_CURRENT, u);
  v = static_cast<float>(static_cast<int16_t>(u)) / currentDivider_mA_;
  return res;
}

I2C::Result INA219::getBusVoltage(float &v)
{
  uint16_t u = 0;
  I2C::Result res = readReg(REG_BUS_VOLTAGE, u);
  int16_t raw = (u >> 3) * 4;
  v = raw * 0.001f;
  return res;
}

I2C::Result INA219::getShuntVoltage(float &v)
{
  uint16_t u = 0;
  I2C::Result res = readReg(REG_SHUNT_VOLTAGE, u);
  v = static_cast<int16_t>(u) * 0.01f;
  return res;
}