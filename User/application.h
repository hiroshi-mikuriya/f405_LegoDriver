/// @file      application.h
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include "control/motor.h"
#include "message/msglib.h"

namespace mik
{
class Application;
}

/// @brief アプリケーションクラス
class mik::Application
{
  Application(Application const &) = delete;
  Application(Application &&) = delete;
  Application &operator=(Application const &) = delete;
  Application &operator=(Application &&) = delete;

  Motor motor0_;
  Motor motor1_;

public:
  /// @brief コンストラクタ
  Application();
  /// @brief デストラクタ
  virtual ~Application() {}
  /// @brief モータを取得する
  /// @param [in] i モータID(0 or 1)
  /// @return モータ
  Motor &motor(uint32_t i) { return i ? motor1_ : motor0_; }
  /// @brief モータを取得する
  /// @param [in] i モータID(0 or 1)
  /// @return モータ
  Motor const &motor(uint32_t i) const { return const_cast<Application *>(this)->motor(i); }
  /// @brief モータ制御する
  void control();
  /// @brief RTOSメッセージを元に状態を更新する
  void update(msg::Message const *msg);
};