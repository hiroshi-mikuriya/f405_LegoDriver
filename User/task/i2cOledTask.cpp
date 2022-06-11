/// @file      task/i2cOledTask.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#include "application.h"
#include "common/alloc.hpp"
#include "device/ssd1306.h"
#include "main.h"
#include "message/msgdef.h"

namespace
{
mik::UniquePtr<mik::I2C> s_i2c;
}

extern "C"
{
  void i2cOledTaskProc(void *argument)
  {
    msg::registerThread(1);
    s_i2c = mik::makeUnique<mik::I2C>(I2C1, DMA1, LL_DMA_STREAM_0, LL_DMA_STREAM_6);
    auto ssd1306 = mik::makeUnique<mik::SSD1306>(s_i2c.get(), mik::SSD1306_SLAVE_ADDR0);
    mik::Application const *app = 0;
    for (;;)
    {
      auto res = msg::recv(100);
      auto *msg = res.msg();
      if (msg && msg->type == msg::APP_POINTER_NOTIFY)
      {
        auto data = *reinterpret_cast<msg::AppPointer const *>(msg->bytes);
        app = static_cast<mik::Application const *>(data.app);
      }
      if (app)
      {
        ssd1306->update(app);
      }
    }
  }

  /// @brief I2C1 イベント割り込み
  void i2c1EvIrqHandler(void) { s_i2c->notifyEvIRQ(); }
  /// @brief I2C1 エラー割り込み
  void i2c1ErIrqHandler(void) { s_i2c->notifyErIRQ(); }
  /// @brief I2C1 DMA受信割り込み
  void i2c1RxIrqHandler(void) { s_i2c->notifyRxEndIRQ(); }
  /// @brief I2C1 DMA受信エラー割り込み
  void i2c1RxErrIrqHandler(void) { s_i2c->notifyRxErrorIRQ(); }
  /// @brief I2C1 DMA送信割り込み
  void i2c1TxIrqHandler(void) { s_i2c->notifyTxEndIRQ(); }
  /// @brief I2C1 DMA送信エラー割り込み
  void i2c1TxErrIrqHandler(void) { s_i2c->notifyTxErrorIRQ(); }
}