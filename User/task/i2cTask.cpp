/// @file      task/i2cTask.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#include "common/alloc.hpp"
#include "device/ina219.h"
#include "main.h"
#include "message/msgdef.h"

namespace
{
mik::UniquePtr<mik::I2C> s_i2c;
constexpr int32_t SIG_TIMER = 1;
} // namespace

extern "C"
{
  void i2cTaskProc(void *argument)
  {
    s_i2c = mik::makeUnique<mik::I2C>(I2C2, DMA1, LL_DMA_STREAM_2, LL_DMA_STREAM_7);
    auto current0 = mik::makeUnique<mik::INA219>(s_i2c.get(), mik::INA219_SLAVE_ADDR0);
    auto current1 = mik::makeUnique<mik::INA219>(s_i2c.get(), mik::INA219_SLAVE_ADDR1);
    for (;;)
    {
      osSignalWait(SIG_TIMER, osWaitForever);
      msg::CurrentData cd{};
      cd.current[0] = current0->getShuntCurrent();
      cd.current[1] = current1->getShuntCurrent();
      cd.busVoltage[0] = current0->getBusVoltage();
      cd.busVoltage[1] = current1->getBusVoltage();
      // cd.shuntVoltage[0] = current0->getShuntVoltage();
      // cd.shuntVoltage[1] = current1->getShuntVoltage();
      msg::send(appTaskHandle, msg::CURRENT_DATA_NOTIFY, cd);
    }
  }

  /// @brief I2C2 イベント割り込み
  void i2c2EvIrqHandler(void) { s_i2c->notifyEvIRQ(); }
  /// @brief I2C2 エラー割り込み
  void i2c2ErIrqHandler(void) { s_i2c->notifyErIRQ(); }
  /// @brief I2C2 DMA受信割り込み
  void i2c2RxIrqHandler(void) { s_i2c->notifyRxEndIRQ(); }
  /// @brief I2C2 DMA受信エラー割り込み
  void i2c2RxErrIrqHandler(void) { s_i2c->notifyRxErrorIRQ(); }
  /// @brief I2C2 DMA送信割り込み
  void i2c2TxIrqHandler(void) { s_i2c->notifyTxEndIRQ(); }
  /// @brief I2C2 DMA送信エラー割り込み
  void i2c2TxErrIrqHandler(void) { s_i2c->notifyTxErrorIRQ(); }
  /// @brief 電流値読み込み依頼
  void updateCurrentIRQ(void) { osSignalSet(i2cTaskHandle, SIG_TIMER); }
}