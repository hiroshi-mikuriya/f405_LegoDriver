/// @file      message/msglib.h
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include "cmsis_os.h"

#ifndef MAX_MAIL_DATA_SIZE
#define MAX_MAIL_DATA_SIZE 32 ///< 付随データの最大長
#endif

namespace msg
{
struct Message;
class Result;
using ID = uint16_t; ///< メッセージID型

/// @brief 自スレッドをメッセージ送信先に登録
/// @param [in] msgCount 格納できる最大メッセージ数
/// @retval osOK 成功
/// @retval それ以外 失敗理由
osStatus registerThread(uint32_t msgCount) noexcept;
/// @brief メッセージ送信
/// @param [in] threadId 送信先スレッドID
/// @param [in] type メッセージ種別
/// @retval osOK 送信成功
/// @retval osErrorParameter 送信先が未登録
/// @retval osEventTimeout タイムアウト発生
/// @retval osErrorOS 送信失敗
osStatus send(osThreadId threadId, ID type) noexcept;
/// @brief メッセージ送信
/// @param [in] threadId 送信先スレッドID
/// @param [in] type メッセージ種別
/// @param [in] bytes 付随データ先頭ポインタ
/// @param [in] size 付随データサイズ
/// @retval osOK 送信成功
/// @retval osErrorParameter 送信先が未登録
/// @retval osErrorValue 付随データサイズが送信可能な最大長を超えている
/// @retval osEventTimeout タイムアウト発生
/// @retval osErrorOS 送信失敗
osStatus send(osThreadId threadId, ID type, void const *bytes, uint16_t size) noexcept;
/// @brief メッセージ送信
/// @tparam T 付随データ型
/// @param [in] threadId 送信先スレッドID
/// @param [in] type メッセージ種別
/// @param [in] data 付随データ
/// @retval osOK 送信成功
/// @retval osErrorParameter 送信先が未登録
/// @retval osErrorValue 付随データサイズが送信可能な最大長を超えている
/// @retval osEventTimeout タイムアウト発生
/// @retval osErrorOS 送信失敗
template <typename T>
osStatus send(osThreadId threadId, ID type, T const &data) noexcept
{
  return send(threadId, type, &data, sizeof(data));
}
/// @brief メッセージ受信
/// @param [in] millisec タイムアウト時間
/// @return 受信結果
Result recv(uint32_t millisec = osWaitForever) noexcept;
} // namespace msg

/// @brief メッセージ型
struct msg::Message
{
  ID type;                           ///< メッセージ種別
  uint16_t size;                     ///< 付随データサイズ
  uint8_t bytes[MAX_MAIL_DATA_SIZE]; ///< 付随データ
};

/// @brief 受信結果型
class msg::Result
{
  /// @brief デフォルトコンストラクタ削除
  Result() = delete;
  /// @brief コピーコンストラクタ削除
  Result(Result const &) = delete;
  /// @brief 代入演算子削除
  Result &operator=(Result const &) = delete;

  osStatus status_; ///< 受信ステータス
  Message *msg_;    ///< メッセージ
  osMailQId mail_;  ///< メールID

public:
  /// @brief コンストラクタ
  /// @param [in] status 受信ステータス
  /// @param [in] msg メッセージ
  /// @param [in] mail メールID
  explicit Result(osStatus status, Message *msg, osMailQId mail) noexcept;
  /// @brief コンストラクタ
  /// @param [in] status 受信ステータス
  explicit Result(osStatus status) noexcept;
  /// @brief デストラクタ
  virtual ~Result();
  /// @brief moveコンストラクタ
  Result(Result &&that) noexcept;
  /// @brief move代入演算子
  /// @param [in] that 移動元
  /// @return 自身の参照
  Result &operator=(Result &&that) noexcept;
  /// @brief msg_のメモリを開放する
  void reset() noexcept;
  /// @brief ステータス取得
  /// @return ステータス
  osStatus status() const noexcept;
  /// @brief メッセージ取得
  /// @return メッセージ
  Message const *msg() const noexcept;
};