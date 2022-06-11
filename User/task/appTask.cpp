/// @file      task/appTask.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#include "application.h"
#include "common/alloc.hpp"
#include "main.h"
#include "message/msgdef.h"

extern "C"
{
  void appTaskProc(void *argument)
  {
    msg::registerThread(4);
    auto app = mik::makeUnique<mik::Application>();
    {
      msg::AppPointer d{app.get()};
      msg::send(i2cOledTaskHandle, msg::APP_POINTER_NOTIFY, d);
    }
    for (;;)
    {
      auto res = msg::recv();
      auto *msg = res.msg();
      if (msg)
      {
        app->update(msg);
      }
    }
  }
}