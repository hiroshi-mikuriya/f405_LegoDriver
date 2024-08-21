/// @file      task/i2cTask.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#include "device/ina219.h"
#include "main.h"
#include "message/msgdef.h"
#include "resource.h"
#include <initializer_list>

namespace
{
mik::I2C *s_i2c = 0;
constexpr int32_t SIG_TIMER = 1;
} // namespace

extern "C"
{
  void i2cTaskProc(void *argument)
  {
    mik::I2C i2c(I2C2, DMA1, LL_DMA_STREAM_7);
    s_i2c = &i2c;

    // ダミー書き込みしないと以降のI2C通信に失敗する
    for (uint8_t slaveAddr : {mik::INA219_SLAVE_ADDR0, mik::INA219_SLAVE_ADDR1})
    {
      uint8_t u[2] = {0};
      i2c.write(slaveAddr, u, sizeof(u));
    }

    mik::INA219 current0(&i2c, mik::INA219_SLAVE_ADDR0);
    mik::INA219 current1(&i2c, mik::INA219_SLAVE_ADDR1);
    current0.init();
    current1.init();

    for (;;)
    {
      osSignalWait(SIG_TIMER, osWaitForever);
      msg::CurrentData cd{};
      current0.getShuntCurrent(cd.current[0]);
      current1.getShuntCurrent(cd.current[1]);
      current0.getBusVoltage(cd.busVoltage[0]);
      current1.getBusVoltage(cd.busVoltage[1]);
      // current0.getShuntVoltage(cd.shuntVoltage[0]);
      // current1.getShuntVoltage(cd.shuntVoltage[1]);
      msg::send(appTaskHandle, msg::CURRENT_DATA_NOTIFY, cd);
    }
  }

  /// @brief I2C2 イベント割り込み
  void I2C2_EV_IRQHandler(void)
  {
    s_i2c->notifyEvIRQ();
  }
  /// @brief I2C2 エラー割り込み
  void I2C2_ER_IRQHandler(void)
  {
    s_i2c->notifyErIRQ();
  }
  /// @brief I2C2 DMA送信割り込み
  void DMA1_Stream7_IRQHandler(void)
  {
    if (LL_DMA_IsActiveFlag_TC7(DMA1))
    {
      LL_DMA_ClearFlag_TC7(DMA1);
      s_i2c->notifyTxEndIRQ();
    }
    if (LL_DMA_IsActiveFlag_TE7(DMA1))
    {
      LL_DMA_ClearFlag_TE7(DMA1);
      s_i2c->notifyTxErrorIRQ();
    }
  }
  void TIM7_IRQHandler(void)
  {
    if (LL_TIM_IsActiveFlag_UPDATE(ENC_UPDATE_TIM))
    {
      LL_TIM_ClearFlag_UPDATE(ENC_UPDATE_TIM);
      osSignalSet(i2cTaskHandle, SIG_TIMER);
      extern void updateEncorderIRQ(void);
      updateEncorderIRQ();
    }
  }
}