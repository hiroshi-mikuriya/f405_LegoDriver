/// @file      peripheral/i2c.h
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include "common/mutex.hpp"
#include "main.h"

namespace mik
{
class I2C;
}

/// @brief I2C通信クラス
class mik::I2C
{
public:
  /// @brief シグナルマスク
  static constexpr int32_t SIG_MASK = 0x0000FFFF;
  /// @brief I2Cクラス関数戻り値定義
  enum Result
  {
    OK = 0,  ///< 成功
    BUSY,    ///< ビジー
    TIMEOUT, ///< タイムアウト
    ERROR,   ///< エラー
  };

private:
  /// @brief デフォルトコンストラクタ削除
  I2C() = delete;
  /// @brief コピーコンストラクタ削除
  I2C(I2C const &) = delete;
  /// @brief 代入演算子削除
  I2C &operator=(I2C const &) = delete;
  /// @brief moveコンストラクタ削除
  I2C(I2C &&) = delete;
  /// @brief move演算子削除
  I2C &operator=(I2C &&) = delete;

  I2C_TypeDef *i2cx_;           ///< I2Cペリフェラル
  mutable osThreadId threadId_; ///< I2C通信を実施するスレッドのID
  DMA_TypeDef *dma_;            ///< 送受信DMA
  uint32_t rxStream_;           ///< 受信DMAストリーム
  uint32_t txStream_;           ///< 送信DMAストリーム
  uint8_t slaveAddr_;           ///< 通信対象デバイスのスレーブアドレス
  Mutex mutex_;                 ///< 通信中にロックするミューテックス

public:
  /// @brief コンストラクタ
  /// @param [in] i2cx I2Cペリフェラル
  /// @param [in] threadId イベント通知先のスレッドID
  /// @param [in] dma 送受信DMA
  /// @param [in] rxStream 受信DMAストリーム
  /// @param [in] txStream 送信DMAストリーム
  explicit I2C(I2C_TypeDef *const i2cx, DMA_TypeDef *const dma, uint32_t rxStream, uint32_t txStream) noexcept;
  /// @brief デストラクタ
  virtual ~I2C();
  /// @brief イベント割り込みが発生したら呼び出す関数
  /// @par SB
  ///      Start Bit
  /// @par ADDR
  ///      Address sent, Address matched
  /// @par ADD10
  ///      10-bit header sent
  /// @par STOP
  ///      Stop detection
  /// @par BTF
  ///      Byte transfer finished
  /// @par RXNE
  ///      Receive buffer not empty
  /// @par TXE
  ///      Transmit buffer empty
  void notifyEvIRQ() noexcept;
  /// @brief エラー割り込みが発生したら呼び出す関数
  /// @par BERR
  ///      Bus Error detection
  /// @par ARLO
  ///      Arbitration Loss
  /// @par AF
  ///      Acknowledge Failur
  /// @par OVR
  ///      Overrun/Underrun
  /// @par TIMEOUT
  ///      SMBus Timeout detection
  /// @par PECERR
  ///      SMBus PEC error detection
  /// @par SMBALERT
  ///      SMBus Alert pin event detection
  void notifyErIRQ() noexcept;
  /// @brief 受信完了割り込みが発生したら呼び出す関数
  void notifyRxEndIRQ() noexcept;
  /// @brief 受信エラー割り込みが発生したら呼び出す関数
  void notifyRxErrorIRQ() noexcept;
  /// @brief 送信完了割り込みが発生したら呼び出す関数
  void notifyTxEndIRQ() noexcept;
  /// @brief 送信エラー割り込みが発生したら呼び出す関数
  void notifyTxErrorIRQ() noexcept;
  /// @brief データを書き込む
  /// @param [in] slaveAddr スレーブアドレス
  /// @param [in] bytes 書き込みデータの先頭ポインタ
  /// @param [in] size 書き込むバイト数
  /// @param [in] waitReady I2CがReadyになるまで待機する
  /// @retval OK      成功
  /// @retval BUSY    ビジー
  /// @retval TIMEOUT タイムアウト
  /// @retval ERROR   エラー
  Result write(uint8_t slaveAddr, void const *bytes, uint32_t size, bool waitReady = true) noexcept;
  /// @brief データを読み込む
  /// @param [in] slaveAddr スレーブアドレス
  /// @param [in] buffer 読み込んだデータを格納するバッファ
  /// @param [in] size 読み込むバイト数
  /// @param [in] waitReady I2CがReadyになるまで待機する
  /// @retval OK      成功
  /// @retval BUSY    ビジー
  /// @retval TIMEOUT タイムアウト
  /// @retval ERROR   エラー
  Result read(uint8_t slaveAddr, void *buffer, uint32_t size, bool waitReady = true) noexcept;
};