/// @file      task/i2cOledTask.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#include "device/ssd1306.h"
#include "main.h"
#include "message/msgdef.h"
#include "resource.h"

namespace
{
mik::I2C *s_i2c = 0;
}

extern "C"
{
  void i2cOledTaskProc(void *argument)
  {
    msg::registerThread(1);
    mik::I2C i2c(I2C1, DMA1, LL_DMA_STREAM_6);
    s_i2c = &i2c;
    mik::SSD1306 oled(&i2c, mik::SSD1306_SLAVE_ADDR0);
    oled.init();
    oled.black();

    mik::Application const *app = 0;
    for (;;)
    {
      auto res = msg::recv(100);
      auto *msg = res.msg();
      if (msg && msg->type == msg::APP_POINTER_NOTIFY)
      {
        auto data = msg->get<msg::AppPointer>();
        app = static_cast<mik::Application const *>(data.app);
      }
      if (app)
      {
        oled.update(app);
      }
    }
  }

  /// @brief I2C1 イベント割り込み
  void I2C1_EV_IRQHandler(void)
  {
    s_i2c->notifyEvIRQ();
  }
  /// @brief I2C1 エラー割り込み
  void I2C1_ER_IRQHandler(void)
  {
    s_i2c->notifyErIRQ();
  }
  /// @brief I2C1 DMA送信割り込み
  void DMA1_Stream6_IRQHandler(void)
  {
    if (LL_DMA_IsActiveFlag_TC6(DMA1))
    {
      LL_DMA_ClearFlag_TC6(DMA1);
      s_i2c->notifyTxEndIRQ();
    }
    if (LL_DMA_IsActiveFlag_TE6(DMA1))
    {
      LL_DMA_ClearFlag_TE6(DMA1);
      s_i2c->notifyTxErrorIRQ();
    }
  }
}