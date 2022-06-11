/// @file      device/fonts.h
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include <cstdint>

namespace mik
{
struct FontDef
{
  uint8_t width;        /* Font width in pixels */
  uint8_t height;       /* Font height in pixels */
  uint16_t const *data; /* Pointer to data font data array */
};
extern const FontDef Font_7x10;
extern const FontDef Font_11x18;
extern const FontDef Font_16x26;
} // namespace mik
