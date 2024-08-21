/// @file      task/resource.h
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include <cmsis_os.h>

extern osThreadId usbTaskHandle;
extern osThreadId keyTaskHandle;
extern osThreadId i2cTaskHandle;
extern osThreadId appTaskHandle;
extern osThreadId i2cOledTaskHandle;
