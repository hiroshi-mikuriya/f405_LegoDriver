/// @file      peripheral/i2c.h
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2024 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include "cmsis_os.h"
#include "main.h"

namespace mik
{
class I2C;
} // namespace mik

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
  I2C() = delete;                       ///< デフォルトコンストラクタ削除
  I2C(I2C const &) = delete;            ///< コピーコンストラクタ削除
  I2C &operator=(I2C const &) = delete; ///< 代入演算子削除
  I2C(I2C &&) = delete;                 ///< moveコンストラクタ削除
  I2C &operator=(I2C &&) = delete;      ///< move演算子削除

  I2C_TypeDef *i2cx_;           ///< I2Cペリフェラル
  mutable osThreadId threadId_; ///< I2C通信を実施するスレッドのID
  DMA_TypeDef *dma_;            ///< 送受信DMA
  uint32_t txStream_;           ///< 送信DMAストリーム
  uint8_t slaveAddr_;           ///< 通信対象デバイスのスレーブアドレス

  enum Mode
  {
    TX_DMA = 0, ///< 送信モード（DMA使用）
    TX,         ///< 送信モード（DMA未使用）
    RX_TX,      ///< 受信前の送信モード（DMA未使用）
    RX          ///< 受信モード（DMA未使用）
  };

  volatile struct
  {
    Mode mode;           ///< 通信モードの指定
    uint8_t const *tbuf; ///< 次の送信データを指すポインタ
    uint16_t tsize;      ///< 残りの送信サイズ
    uint8_t *rbuf;       ///< 次の受信データの格納先
    uint16_t rsize;      ///< 残りの受信サイズ
  } inter_;              ///< 割り込み内部で使用する変数群

  /// @brief 関連する割り込みを全て禁止にする
  void disableInterrupts();

public:
  /// @brief コンストラクタ（DMA使用）
  /// @param [in] i2cx I2Cペリフェラル
  /// @param [in] dma 送受信DMA
  /// @param [in] txStream 送信DMAストリーム
  explicit I2C(I2C_TypeDef *const i2cx, DMA_TypeDef *const dma, uint32_t txStream);
  /// @brief コンストラクタ（DMA未使用）
  /// @param [in] i2cx I2Cペリフェラル
  explicit I2C(I2C_TypeDef *const i2cx);
  /// @brief デストラクタ
  virtual ~I2C();
  /// @brief イベント割り込みが発生したら呼び出す関数
  /// @par SB
  ///      Start Bit
  /// @par ADDR
  ///      Address sent, Address matched
  /// @par BTF
  ///      Byte transfer finished
  /// @par RXNE
  ///      Receive buffer not empty
  /// @par TXE
  ///      Transmit buffer empty
  void notifyEvIRQ();
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
  void notifyErIRQ();
  /// @brief 送信完了割り込みが発生したら呼び出す関数
  void notifyTxEndIRQ();
  /// @brief 送信エラー割り込みが発生したら呼び出す関数
  void notifyTxErrorIRQ();
  /// @brief データを書き込む（DMA使用する）
  /// @param [in] slaveAddr スレーブアドレス
  /// @param [in] bytes 書き込みデータの先頭ポインタ
  /// @param [in] size 書き込むバイト数
  /// @retval OK      成功
  /// @retval BUSY    ビジー
  /// @retval TIMEOUT タイムアウト
  /// @retval ERROR   エラー
  Result writeWithDma(uint8_t slaveAddr, void const *bytes, uint16_t size);
  /// @brief データを書き込む（DMA使用しない）
  /// @param [in] slaveAddr スレーブアドレス
  /// @param [in] bytes 書き込みデータの先頭ポインタ
  /// @param [in] size 書き込むバイト数
  /// @retval OK      成功
  /// @retval BUSY    ビジー
  /// @retval TIMEOUT タイムアウト
  /// @retval ERROR   エラー
  Result write(uint8_t slaveAddr, void const *bytes, uint16_t size);
  /// @brief レジスタを指定してからデータを読み込む（DMA使用しない）
  /// @param [in] slaveAddr スレーブアドレス
  /// @param [in] reg レジスタ番号
  /// @param [in] buffer 読み込んだデータを格納するバッファ
  /// @param [in] size 読み込むバイト数
  /// @retval OK      成功
  /// @retval BUSY    ビジー
  /// @retval TIMEOUT タイムアウト
  /// @retval ERROR   エラー
  Result readReg(uint8_t slaveAddr, uint8_t reg, void *buffer, uint16_t size);
};