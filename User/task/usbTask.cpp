/// @file      task/usbTask.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#include "main.h"
#include "message/msgdef.h"
#include "resource.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

constexpr int32_t SIG_TX_END = 1;

extern "C"
{
  void usbTaskProc(void *argument)
  {
    MX_USB_DEVICE_Init();
    msg::registerThread(4);
    for (;;)
    {
      auto res = msg::recv();
      auto *msg = res.msg();
      if (!msg || msg->type != msg::USB_TX_REQ)
      {
        continue;
      }
      if (CDC_Transmit_FS(const_cast<uint8_t *>(msg->bytes), msg->size) != USBD_OK)
      {
        continue;
      }
      osSignalWait(SIG_TX_END, 10);
    }
  }

  void USB_RxIRQ(uint8_t const *data, uint32_t size)
  {
    UNUSED(data);
    UNUSED(size);
    // do nothing;
  }

  void USB_TxCpltIRQ(uint8_t const *data, uint32_t size, uint8_t ep)
  {
    UNUSED(data);
    UNUSED(size);
    UNUSED(ep);
    osSignalSet(usbTaskHandle, SIG_TX_END);
  }
}