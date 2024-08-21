/// @file      peripheral/i2c.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2024 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#include "i2c.h"

using namespace mik;

namespace
{
constexpr int32_t SIG_ADDR = (1 << 1) & I2C::SIG_MASK;
constexpr int32_t SIG_DMAEND = (1 << 2) & I2C::SIG_MASK;
constexpr int32_t SIG_DMAERR = (1 << 3) & I2C::SIG_MASK;
constexpr int32_t SIG_RXEND = (1 << 4) & I2C::SIG_MASK;
constexpr int32_t SIG_TXEND = (1 << 5) & I2C::SIG_MASK;
constexpr int32_t SIG_ERROR = (1 << 7) & I2C::SIG_MASK;

constexpr uint8_t I2C_REQUEST_WRITE = 0x00;
constexpr uint8_t I2C_REQUEST_READ = 0x01;

/// @brief osEvent を I2C::Result へ変換する
/// @param [in] ev osEvent
/// @return I2C::Result
inline I2C::Result makeResult(osEvent const &ev)
{
  if (ev.status & osEventTimeout)
  {
    return I2C::TIMEOUT;
  }
  if (ev.value.signals & SIG_DMAERR)
  {
    return I2C::ERROR;
  }
  if (ev.value.signals & SIG_ERROR)
  {
    return I2C::ERROR;
  }
  return I2C::OK;
}
/// @brief I2Cビジーが解除されるまで待機する
/// @param [in] i2cx I2Cペリフェラル
/// @retval true レディ
/// @retval false タイムアウト
inline bool waitForReady(I2C_TypeDef *i2cx)
{
  for (int i = 0; i < 100; ++i)
  {
    if (!LL_I2C_IsActiveFlag_BUSY(i2cx))
    {
      return true;
    }
    osThreadYield();
  }
  return false;
}
} // namespace

void I2C::disableInterrupts()
{
  if (dma_)
  {
    LL_DMA_DisableIT_TC(dma_, txStream_);
    LL_DMA_DisableIT_TE(dma_, txStream_);
  }
  LL_I2C_DisableIT_EVT(i2cx_); // SB, ADDR, ADD10, STOPF, BTF, RXNE, TXE,
  LL_I2C_DisableIT_ERR(i2cx_); // BERR, ARLO, AF, OVR, TIMEOUT, PECERR, SMBALERT
}

I2C::I2C(I2C_TypeDef *const i2cx, //
         DMA_TypeDef *const dma,  //
         uint32_t txStream)       //
    : i2cx_(i2cx),                //
      threadId_(0),               //
      dma_(dma),                  //
      txStream_(txStream),        //
      slaveAddr_(0),              //
      inter_{}                    //
{
  if (dma_)
  {
    LL_DMA_SetPeriphAddress(dma_, txStream_, LL_I2C_DMA_GetRegAddr(i2cx_));
  }
}

I2C::I2C(I2C_TypeDef *const i2cx) //
    : I2C(i2cx, 0, 0)             //
{
}

I2C::~I2C()
{
  if (dma_)
  {
    LL_DMA_DisableStream(dma_, txStream_);
  }
  LL_I2C_DisableDMAReq_RX(i2cx_);
  LL_I2C_DisableDMAReq_TX(i2cx_);
  disableInterrupts();
  LL_I2C_Disable(i2cx_);
}

void I2C::notifyEvIRQ()
{
  // Start Bit (master mode).
  if (LL_I2C_IsActiveFlag_SB(i2cx_))
  {
    switch (inter_.mode)
    {
    case RX:
      LL_I2C_TransmitData8(i2cx_, slaveAddr_ | I2C_REQUEST_READ);
      break;
    case RX_TX:
    case TX_DMA:
    case TX:
      LL_I2C_TransmitData8(i2cx_, slaveAddr_ | I2C_REQUEST_WRITE);
      break;
    }
  }
  // Address sent (master mode) or Address matched flag (slave mode)
  if (LL_I2C_IsActiveFlag_ADDR(i2cx_))
  {
    switch (inter_.mode)
    {
    case TX_DMA:
      LL_I2C_EnableDMAReq_TX(i2cx_);
      break;
    case RX:
      if (2 <= inter_.rsize)
      {
        LL_I2C_AcknowledgeNextData(i2cx_, LL_I2C_ACK);
      }
      else
      {
        LL_I2C_AcknowledgeNextData(i2cx_, LL_I2C_NACK);
      }
      break;
    case RX_TX:
    case TX:
      break;
    }
    LL_I2C_ClearFlag_ADDR(i2cx_);
    LL_I2C_IsActiveFlag_BUSY(i2cx_); // Read SR2 (Dummy)
  }
  // Transmit data register empty flag
  if (LL_I2C_IsActiveFlag_TXE(i2cx_))
  {
    switch (inter_.mode)
    {
    case RX:
    case TX_DMA:
      break;
    case RX_TX:
      if (0 < inter_.tsize)
      {
        LL_I2C_TransmitData8(i2cx_, *inter_.tbuf);
        inter_.tbuf++;
        inter_.tsize--;
      }
      else
      {
        inter_.mode = RX;
        LL_I2C_GenerateStartCondition(i2cx_);
      }
      break;
    case TX:
      if (0 < inter_.tsize)
      {
        LL_I2C_TransmitData8(i2cx_, *inter_.tbuf);
        inter_.tbuf++;
        inter_.tsize--;
      }
      if (inter_.tsize == 0)
      {
        LL_I2C_GenerateStopCondition(i2cx_);
        osSignalSet(threadId_, SIG_TXEND);
      }
      break;
    }
  }
  // Byte Transfer Finished flag
  if (LL_I2C_IsActiveFlag_BTF(i2cx_))
  {
  }
  // Receive data register not empty flag
  if (LL_I2C_IsActiveFlag_RXNE(i2cx_))
  {
    *inter_.rbuf = LL_I2C_ReceiveData8(i2cx_);
    inter_.rbuf++;
    inter_.rsize--;
    if (inter_.rsize == 0)
    {
      LL_I2C_GenerateStopCondition(i2cx_);
      osSignalSet(threadId_, SIG_RXEND);
    }
    else if (inter_.rsize == 1)
    {
      LL_I2C_AcknowledgeNextData(i2cx_, LL_I2C_NACK);
    }
  }
}

void I2C::notifyErIRQ()
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
  LL_I2C_GenerateStopCondition(i2cx_);
  osSignalSet(threadId_, SIG_ERROR);
}

void I2C::notifyTxEndIRQ()
{
  LL_I2C_GenerateStopCondition(i2cx_);
  osSignalSet(threadId_, SIG_DMAEND);
}

void I2C::notifyTxErrorIRQ()
{
  LL_I2C_GenerateStopCondition(i2cx_);
  osSignalSet(threadId_, SIG_DMAERR);
}

I2C::Result I2C::writeWithDma(uint8_t slaveAddr, void const *bytes, uint16_t size)
{
  if (!dma_)
  {
    return Result::ERROR;
  }
  inter_.mode = TX_DMA;
  slaveAddr_ = slaveAddr;
  if (!waitForReady(i2cx_))
  {
    return Result::BUSY;
  }
  threadId_ = osThreadGetId();
  if (threadId_ == 0)
  {
    return Result::ERROR;
  }
  LL_I2C_DisableDMAReq_RX(i2cx_);
  LL_I2C_DisableDMAReq_TX(i2cx_);
  disableInterrupts();

  // 割り込み有効化
  LL_DMA_EnableIT_TC(dma_, txStream_);
  LL_DMA_EnableIT_TE(dma_, txStream_);
  LL_I2C_EnableIT_EVT(i2cx_); // SB ADDR ADD10 STOPF BTF
  LL_I2C_EnableIT_ERR(i2cx_); // BERR ARLO AF OVR TIMEOUT PECERR SMBALERT

  // DMAを有効化した直後にDMA通信が発動するよう、通信データやサイズをセットしておく
  LL_DMA_SetMemoryAddress(dma_, txStream_, reinterpret_cast<uint32_t>(bytes));
  LL_DMA_SetDataLength(dma_, txStream_, size + 1); // サイズを加算しないと最後の１バイトが送信されない
  LL_DMA_EnableStream(dma_, txStream_);

  // I2C通信実施
  osSignalWait(I2C::SIG_MASK, 0); // フラグクリア
  LL_I2C_GenerateStartCondition(i2cx_);
  osEvent ev = osSignalWait(SIG_DMAEND | SIG_ERROR | SIG_DMAERR, 10);

  // 割り込み無効化
  disableInterrupts();

  return makeResult(ev);
}

I2C::Result I2C::write(uint8_t slaveAddr, void const *bytes, uint16_t size)
{
  inter_.mode = TX;
  inter_.tbuf = static_cast<uint8_t const *>(bytes);
  inter_.tsize = size + 1; // サイズを加算しないと最後の１バイトが送信されない
  slaveAddr_ = slaveAddr;
  if (!waitForReady(i2cx_))
  {
    return Result::BUSY;
  }
  threadId_ = osThreadGetId();
  if (threadId_ == 0)
  {
    return Result::ERROR;
  }
  LL_I2C_DisableDMAReq_RX(i2cx_);
  LL_I2C_DisableDMAReq_TX(i2cx_);
  disableInterrupts();

  LL_I2C_EnableIT_EVT(i2cx_); // SB ADDR ADD10 STOPF BTF
  LL_I2C_EnableIT_ERR(i2cx_); // BERR ARLO AF OVR TIMEOUT PECERR SMBALERT
  LL_I2C_EnableIT_BUF(i2cx_); // RXNE TXE

  osSignalWait(I2C::SIG_MASK, 0); // フラグクリア
  LL_I2C_GenerateStartCondition(i2cx_);
  auto ev = osSignalWait(SIG_ERROR | SIG_TXEND, 20);

  disableInterrupts();
  osDelay(1);
  return makeResult(ev);
}

I2C::Result I2C::readReg(uint8_t slaveAddr, uint8_t reg, void *buffer, uint16_t size)
{
  inter_.mode = RX_TX;
  inter_.tbuf = &reg;
  inter_.tsize = sizeof(reg);
  inter_.rbuf = static_cast<uint8_t *>(buffer);
  inter_.rsize = size;
  slaveAddr_ = slaveAddr;
  if (!waitForReady(i2cx_))
  {
    return Result::BUSY;
  }
  LL_I2C_DisableDMAReq_RX(i2cx_);
  LL_I2C_DisableDMAReq_TX(i2cx_);
  disableInterrupts();

  LL_I2C_EnableIT_EVT(i2cx_); // SB ADDR ADD10 STOPF BTF
  LL_I2C_EnableIT_ERR(i2cx_); // BERR ARLO AF OVR TIMEOUT PECERR SMBALERT
  LL_I2C_EnableIT_BUF(i2cx_); // RXNE TXE

  osSignalWait(I2C::SIG_MASK, 0); // フラグクリア
  LL_I2C_GenerateStartCondition(i2cx_);
  auto ev = osSignalWait(SIG_ERROR | SIG_RXEND, 20);

  disableInterrupts();
  return makeResult(ev);
}
