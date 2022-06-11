/// @file      peripheral/i2c.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#include "i2c.h"
#include <cstring> // memcpy

namespace
{
constexpr int32_t SIG_ADDR = (1 << 1) & mik::I2C::SIG_MASK;
constexpr int32_t SIG_DMAEND = (1 << 2) & mik::I2C::SIG_MASK;
constexpr int32_t SIG_DMAERR = (1 << 3) & mik::I2C::SIG_MASK;
constexpr int32_t SIG_ERROR = (1 << 7) & mik::I2C::SIG_MASK;

constexpr uint8_t I2C_REQUEST_WRITE = 0x00;
constexpr uint8_t I2C_REQUEST_READ = 0x01;

/// @brief osEvent を I2C::Result へ変換する
/// @param [in] ev osEvent
/// @return I2C::Result
mik::I2C::Result makeResult(osEvent const &ev)
{
  if (ev.status & osEventTimeout)
  {
    return mik::I2C::TIMEOUT;
  }
  if (ev.value.signals & SIG_DMAERR)
  {
    return mik::I2C::ERROR;
  }
  if (ev.value.signals & SIG_ERROR)
  {
    return mik::I2C::ERROR;
  }
  return mik::I2C::OK;
}
} // namespace

mik::I2C::I2C(I2C_TypeDef *const i2cx,    //
              DMA_TypeDef *const dma,     //
              uint32_t rxStream,          //
              uint32_t txStream) noexcept //
    : i2cx_(i2cx),                        //
      threadId_(0),                       //
      dma_(dma),                          //
      rxStream_(rxStream),                //
      txStream_(txStream),                //
      slaveAddr_(0),                      //
      mutex_()                            //
{
  LL_I2C_Enable(i2cx_);
  LL_DMA_SetDataTransferDirection(dma_, txStream_, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  LL_DMA_SetPeriphAddress(dma_, txStream_, LL_I2C_DMA_GetRegAddr(i2cx_));
  LL_DMA_SetDataTransferDirection(dma_, rxStream_, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetPeriphAddress(dma_, rxStream_, LL_I2C_DMA_GetRegAddr(i2cx_));
}

mik::I2C::~I2C()
{
  LL_DMA_DisableIT_TC(dma_, rxStream_);
  LL_DMA_DisableIT_TE(dma_, rxStream_);
  LL_DMA_DisableStream(dma_, rxStream_);
  LL_DMA_DisableIT_TC(dma_, txStream_);
  LL_DMA_DisableIT_TE(dma_, txStream_);
  LL_DMA_DisableStream(dma_, txStream_);
  LL_I2C_DisableDMAReq_RX(i2cx_);
  LL_I2C_DisableDMAReq_TX(i2cx_);
  LL_I2C_DisableIT_EVT(i2cx_);
  LL_I2C_DisableIT_ERR(i2cx_);
  LL_I2C_DisableIT_BUF(i2cx_);
  LL_I2C_DisableIT_TX(i2cx_);
  LL_I2C_DisableIT_RX(i2cx_);
  LL_I2C_Disable(i2cx_);
}

void mik::I2C::notifyEvIRQ() noexcept
{
  // Start Bit (master mode).
  if (LL_I2C_IsActiveFlag_SB(i2cx_))
  {
    LL_I2C_TransmitData8(i2cx_, slaveAddr_);
  }
  // Address sent (master mode) or Address matched flag (slave mode)
  if (LL_I2C_IsActiveFlag_ADDR(i2cx_))
  {
    LL_I2C_ClearFlag_ADDR(i2cx_);
    osSignalSet(threadId_, SIG_ADDR);
  }
  /*
  // 10-bit header sent (master mode)
  if (LL_I2C_IsActiveFlag_ADD10(i2cx_))
  {
  }
  // Stop detection flag (slave mode)
  if (LL_I2C_IsActiveFlag_STOP(i2cx_))
  {
    LL_I2C_ClearFlag_STOP(i2cx_);
  }
  // Transmit data register empty flag
  if (LL_I2C_IsActiveFlag_TXE(i2cx_))
  {
  }
  // Byte Transfer Finished flag
  if (LL_I2C_IsActiveFlag_BTF(i2cx_))
  {
  }
  // Receive data register not empty flag
  if (LL_I2C_IsActiveFlag_RXNE(i2cx_))
  {
  }
  */
}

void mik::I2C::notifyErIRQ() noexcept
{
  // Bus error flag
  if (LL_I2C_IsActiveFlag_BERR(i2cx_))
  {
    LL_I2C_ClearFlag_BERR(i2cx_);
  }
  // Arbitration lost flag
  if (LL_I2C_IsActiveFlag_ARLO(i2cx_))
  {
    LL_I2C_ClearFlag_ARLO(i2cx_);
  }
  // Acknowledge failure flag
  if (LL_I2C_IsActiveFlag_AF(i2cx_))
  {
    LL_I2C_ClearFlag_AF(i2cx_);
  }
  // Overrun/Underrun flag
  if (LL_I2C_IsActiveFlag_OVR(i2cx_))
  {
    LL_I2C_ClearFlag_OVR(i2cx_);
  }
  // SMBus Timeout detection flag
  if (LL_I2C_IsActiveSMBusFlag_TIMEOUT(i2cx_))
  {
    LL_I2C_ClearSMBusFlag_TIMEOUT(i2cx_);
  }
  // SMBus PEC error flag in reception
  if (LL_I2C_IsActiveSMBusFlag_PECERR(i2cx_))
  {
    LL_I2C_ClearSMBusFlag_PECERR(i2cx_);
  }
  // SMBus alert flag
  if (LL_I2C_IsActiveSMBusFlag_ALERT(i2cx_))
  {
    LL_I2C_ClearSMBusFlag_ALERT(i2cx_);
  }
  osSignalSet(threadId_, SIG_ERROR);
}

void mik::I2C::notifyRxEndIRQ() noexcept
{
  osSignalSet(threadId_, SIG_DMAEND);
}

void mik::I2C::notifyRxErrorIRQ() noexcept
{
  osSignalSet(threadId_, SIG_DMAERR);
}

void mik::I2C::notifyTxEndIRQ() noexcept
{
  osSignalSet(threadId_, SIG_DMAEND);
}

void mik::I2C::notifyTxErrorIRQ() noexcept
{
  osSignalSet(threadId_, SIG_DMAERR);
}

mik::I2C::Result mik::I2C::write(uint8_t slaveAddr, void const *bytes, uint32_t size, bool waitReady) noexcept
{
  LockGuard<Mutex> lock(mutex_);
  if (!i2cx_)
  {
    return Result::ERROR;
  }
  if (waitReady)
  {
    while (LL_I2C_IsActiveFlag_BUSY(i2cx_))
    {
      osThreadYield();
    }
  }
  else if (LL_I2C_IsActiveFlag_BUSY(i2cx_))
  {
    return Result::BUSY;
  }
  threadId_ = osThreadGetId();
  if (threadId_ == 0)
  {
    return Result::ERROR;
  }
  osSignalWait(mik::I2C::SIG_MASK, 0); // フラグクリア
  slaveAddr_ = slaveAddr | I2C_REQUEST_WRITE;

  // 割り込み有効化
  LL_DMA_EnableIT_TC(dma_, txStream_);
  LL_DMA_EnableIT_TE(dma_, txStream_);
  LL_I2C_EnableIT_EVT(i2cx_); // SB ADDR ADD10 STOPF BTF
  LL_I2C_EnableIT_ERR(i2cx_); // BERR ARLO AF OVR TIMEOUT PECERR SMBALERT

  // スレーブアドレスの送信はDMAを使用しないので、一時的にDMAを無効化する
  LL_I2C_DisableDMAReq_TX(i2cx_);

  // I2C通信実施
  LL_I2C_AcknowledgeNextData(i2cx_, LL_I2C_ACK);
  LL_I2C_GenerateStartCondition(i2cx_);
  osEvent ev = osSignalWait(SIG_ADDR, 1);
  if (ev.status == osEventSignal && ev.value.signals & SIG_ADDR)
  {
    LL_I2C_EnableDMAReq_TX(i2cx_);
    LL_DMA_SetMemoryAddress(dma_, txStream_, reinterpret_cast<uint32_t>(bytes));
    LL_DMA_SetDataLength(dma_, txStream_, size + 1); // TODO サイズを加算しないと最後の１バイトが送信されない
    LL_DMA_EnableStream(dma_, txStream_);
    ev = osSignalWait(SIG_DMAEND | SIG_ERROR | SIG_DMAERR, 100);
  }
  LL_I2C_GenerateStopCondition(i2cx_);

  // 割り込み無効化
  LL_DMA_DisableIT_TC(dma_, txStream_);
  LL_DMA_DisableIT_TE(dma_, txStream_);
  LL_I2C_DisableIT_EVT(i2cx_);
  LL_I2C_DisableIT_ERR(i2cx_);

  return makeResult(ev);
}

mik::I2C::Result mik::I2C::read(uint8_t slaveAddr, void *buffer, uint32_t size, bool waitReady) noexcept
{
  LockGuard<Mutex> lock(mutex_);
  if (!i2cx_)
  {
    return Result::ERROR;
  }
  threadId_ = osThreadGetId();
  if (threadId_ == 0)
  {
    return Result::ERROR;
  }
  if (waitReady)
  {
    while (LL_I2C_IsActiveFlag_BUSY(i2cx_))
    {
      osThreadYield();
    }
  }
  else if (LL_I2C_IsActiveFlag_BUSY(i2cx_))
  {
    return Result::BUSY;
  }
  osSignalWait(mik::I2C::SIG_MASK, 0); // フラグクリア
  slaveAddr_ = slaveAddr | I2C_REQUEST_READ;

  // 割り込み有効化
  LL_DMA_EnableIT_TC(dma_, rxStream_);
  LL_DMA_EnableIT_TE(dma_, rxStream_);
  LL_I2C_EnableIT_EVT(i2cx_); // SB ADDR ADD10 STOPF BTF
  LL_I2C_EnableIT_ERR(i2cx_); // BERR ARLO AF OVR TIMEOUT PECERR SMBALERT

  // スレーブアドレスの送信はDMAを使用しないので、一時的にDMAを無効化する
  LL_I2C_DisableDMAReq_RX(i2cx_);

  // I2C通信実施
  LL_I2C_AcknowledgeNextData(i2cx_, LL_I2C_ACK);
  LL_I2C_GenerateStartCondition(i2cx_);
  osEvent ev = osSignalWait(SIG_ADDR, 1);
  if (ev.status == osEventSignal && ev.value.signals & SIG_ADDR)
  {
    LL_I2C_EnableDMAReq_RX(i2cx_);
    LL_DMA_SetMemoryAddress(dma_, rxStream_, reinterpret_cast<uint32_t>(buffer));
    LL_DMA_SetDataLength(dma_, rxStream_, size - 1); // TODO サイズを減算しないと１バイト余計に受信してしまう
    LL_DMA_EnableStream(dma_, rxStream_);
    ev = osSignalWait(SIG_DMAEND | SIG_ERROR | SIG_DMAERR, 10);
  }
  LL_I2C_GenerateStopCondition(i2cx_);

  // 割り込み無効化
  LL_DMA_DisableIT_TC(dma_, rxStream_);
  LL_DMA_DisableIT_TE(dma_, rxStream_);
  LL_I2C_DisableIT_EVT(i2cx_);
  LL_I2C_DisableIT_ERR(i2cx_);

  return makeResult(ev);
}
