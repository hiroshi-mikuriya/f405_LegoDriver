/// @file      device/ssd1306.cpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2024 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#include "ssd1306.h"
#include "constants.h"
#include "fonts.h"
#include <cmsis_os.h>
#include <cstdio>  // to use 'memset'
#include <cstring> // to use 'memcpy' 'memset'

using namespace mik;

namespace
{
constexpr uint8_t WIDTH = 128;                  ///< 横ピクセル数
constexpr uint8_t NUM_PAGE = 8;                 ///< ページ数
constexpr uint8_t HEIGHT = NUM_PAGE * 8;        ///< 縦ピクセル数
constexpr uint32_t BUF_SIZE = WIDTH * NUM_PAGE; ///< 画面の全ピクセルデータをバッファするのに必要なサイズ

constexpr uint8_t SSD1306_CONFIG_MUX_RATIO_CMD = 0xA8;
constexpr uint8_t SSD1306_CONFIG_MUX_RATIO_A = 0x3F;
constexpr uint8_t SSD1306_CONFIG_DISPLAY_OFFSET_CMD = 0xD3;
constexpr uint8_t SSD1306_CONFIG_DISPLAY_OFFSET_A = 0x0;
constexpr uint8_t SSD1306_CONFIG_DISPLAY_START_LINE = 0x40;
constexpr uint8_t SSD1306_CONFIG_SEGMENT_REMAP = 0xA1;
constexpr uint8_t SSD1306_CONFIG_COM_OUT_DIRECTION = 0xC8;
constexpr uint8_t SSD1306_CONFIG_COM_PIN_CONFIG_CMD = 0xDA;
constexpr uint8_t SSD1306_CONFIG_COM_PIN_CONFIG_A = 0x12;
constexpr uint8_t SSD1306_CONFIG_CONTRAST_CMD = 0x81;
constexpr uint8_t SSD1306_CONFIG_CONTRAST_A = 0x7F;
constexpr uint8_t SSD1306_CONFIG_ENTIRE_DISPLAY_ON = 0xA4;
constexpr uint8_t SSD1306_CONFIG_DISPLAY_PIX_MODE = 0xA6;
constexpr uint8_t SSD1306_CONFIG_DISPLAY_FREQ_CMD = 0xD5;
constexpr uint8_t SSD1306_CONFIG_DISPLAY_FREQ_A = 0xF0;
constexpr uint8_t SSD1306_CONFIG_ADDRESSING_MODE_CMD = 0x20;
constexpr uint8_t SSD1306_CONFIG_ADDRESSING_MODE_A = 0x0;
constexpr uint8_t SSD1306_CONFIG_CHARGE_PUMP_CMD = 0x8D;
constexpr uint8_t SSD1306_CONFIG_CHARGE_PUMP_A = 0x14;
constexpr uint8_t SSD1306_CONFIG_DISPLAY_ON_OFF = 0xAF;

///< 1つのコマンドセットのみ
constexpr uint8_t SSD1306_CTRL_BYTE_CMD_SINGLE = 0b00000000;
///< コマンドの後ろに続けて複数のコントロールバイト＆コマンドor描画データを場合
constexpr uint8_t SSD1306_CTRL_BYTE_CMD_STREAM = 0b10000000;
///< 描画データのWriteのみ
constexpr uint8_t SSD1306_CTRL_BYTE_DATA_SINGLE = 0b01000000;
///< 描画データの後ろに続けて複数のコントロールバイト＆コマンドor描画データを場合
constexpr uint8_t SSD1306_CTRL_BYTE_DATA_STREAM = 0b11000000;

/// @brief １ピクセル書き込む
/// @param [in] color 色
/// @param [in] x X位置
/// @param [in] y Y位置
/// @param [out] dst 書き込み先
inline void drawPixel(uint8_t color, uint8_t x, uint8_t y, uint8_t *dst) noexcept
{
  if (x < WIDTH && y < HEIGHT)
  {
    if (color)
    {
      dst[x + (y / 8) * WIDTH] |= 1 << (y % 8);
    }
    else
    {
      dst[x + (y / 8) * WIDTH] &= ~(1 << (y % 8));
    }
  }
}
/// @brief １文字書き込む
/// @param [in] ch １文字
/// @param [in] font フォントデータ
/// @param [in] invert 反転有無
/// @param [in] x X位置
/// @param [in] y Y位置
/// @param [out] dst 書き込み先
inline void drawChar(char ch, mik::FontDef const &font, bool invert, uint8_t x, uint8_t y, uint8_t *dst)
{
  for (uint8_t dy = 0; dy < font.height; ++dy)
  {
    uint16_t b = font.data[(ch - 32) * font.height + dy];
    for (uint8_t dx = 0; dx < font.width; ++dx)
    {
      bool on = (static_cast<uint16_t>(0) != ((0x8000 >> dx) & b)) ? true : false;
      if (invert)
      {
        on = !on;
      }
      drawPixel(on, x + dx, y + dy, dst);
    }
  }
}

/// @brief 文字列を書き込む
/// @param [in] str 文字列
/// @param [in] font フォントデータ
/// @param [in] invert 反転有無
/// @param [in] x X位置
/// @param [in] y Y位置
/// @param [out] dst 書き込み先
inline void drawString(char const *str, mik::FontDef const &font, bool invert, uint8_t x, uint8_t y, uint8_t *dst)
{
  for (char const *p = str; *p; ++p)
  {
    drawChar(*p, font, invert, x, y, dst);
    x += font.width;
  }
}
/// @brief モータ制御モード文字列を取得する
/// @param [in] m モータ制御モード
/// @return モータ制御モード文字列
inline char const *const modeText(mik::MotorMode m)
{
  switch (m)
  {
  case mik::VELOCITY:
    return "VELOCITY";
  case mik::POSITION:
    return "POSITION";
  default:
    return "NONE";
  }
}
} // namespace

I2C::Result SSD1306::init()
{
  // see. https://monoedge.net/raspi-ssd1306/
  // 1	Set MUX Ratio	使用する行数。
  // default値0x3F=64行	0xA8, 0x3F
  // 2	Set Display Offset	画面垂直方向のオフセット(ずらし)
  // オフセットは設けないので0に設定	0xD3, 0x00
  // 3	Set Display Start Line	画面スタートライン
  // 全領域使用するので0x40を指定(0x40~0x7F)	0x40
  // 4	Set Segment Re-map	水平方向の反転
  // デフォルトは右下スタートだが、今回は左上スタートにするので反転有効=0xA1	0xA1
  // 5	Set COM Output Scan Direction	垂直方向の反転
  // デフォルトは右下スタートだが、今回は左上スタートにするので反転有効=0xC8	0xC8
  // 6	Set COM Pins Hardware Configuration	COM signals pinの設定のようだが値を変えても挙動に変化無し。デフォルト値0x12でOK	0xDA, 0x12
  // 7	Set Contrast Control	256階調のコントラスト(明るさ)設定
  // デフォルトは0x7F。最大は0xFF。お好みでOK	0x81, 0xFF
  // 8	Disable Entire Display On	0xA4で画面メモリの内容を画面表示
  // 0xA5でテスト用(と思われる)の全画面表示機能なので使用しない	0xA4
  // 9	Set Normal Display	白黒反転設定
  // 通常は1が発光だが、その逆の設定も可能
  // 今回は通常設定	0xA6
  // 10	Set Osc Frequency	ディスプレイのクロック設定
  // デフォルト値0x80でOK	0xD5, 0x80
  // 11	Set Addressing Mode	アドレスモードの設定
  // 0x00：水平方向アドレッシング←今回はこれ
  // 0x01：垂直方向アドレッシング
  // 0x10：ページアドレッシング(default)	0x20, 0x00
  // 12	Enable Charge Pump Regulator	ディスプレイをONにするにはチャージポンプレギュレータをONにしておく必要があるので設定します	0x8D, 0x14
  // 13	Display On	0xAEは画面OFF
  // 0xAFで画面ONです	0xAF

  constexpr uint8_t v[] = {
      SSD1306_CTRL_BYTE_CMD_SINGLE,       //
      SSD1306_CONFIG_MUX_RATIO_CMD,       //
      SSD1306_CONFIG_MUX_RATIO_A,         //
      SSD1306_CONFIG_DISPLAY_OFFSET_CMD,  //
      SSD1306_CONFIG_DISPLAY_OFFSET_A,    //
      SSD1306_CONFIG_DISPLAY_START_LINE,  //
      SSD1306_CONFIG_SEGMENT_REMAP,       //
      SSD1306_CONFIG_COM_OUT_DIRECTION,   //
      SSD1306_CONFIG_COM_PIN_CONFIG_CMD,  //
      SSD1306_CONFIG_COM_PIN_CONFIG_A,    //
      SSD1306_CONFIG_CONTRAST_CMD,        //
      SSD1306_CONFIG_CONTRAST_A,          //
      SSD1306_CONFIG_ENTIRE_DISPLAY_ON,   //
      SSD1306_CONFIG_DISPLAY_PIX_MODE,    //
      SSD1306_CONFIG_DISPLAY_FREQ_CMD,    //
      SSD1306_CONFIG_DISPLAY_FREQ_A,      //
      SSD1306_CONFIG_ADDRESSING_MODE_CMD, //
      SSD1306_CONFIG_ADDRESSING_MODE_A,   //
      SSD1306_CONFIG_CHARGE_PUMP_CMD,     //
      SSD1306_CONFIG_CHARGE_PUMP_A,       //
      SSD1306_CONFIG_DISPLAY_ON_OFF,      //
  };
  return i2c_->writeWithDma(slaveAddr_, v, sizeof(v));
}

I2C::Result SSD1306::sendBufferToDevice()
{
  I2C::Result res = I2C::OK;
  for (uint8_t page = 0; page < NUM_PAGE; ++page)
  {
    uint8_t v0[] = {
        SSD1306_CTRL_BYTE_CMD_STREAM, //
        (uint8_t)(0xB0 + page),       // set page start address
        SSD1306_CTRL_BYTE_CMD_SINGLE, //
        0x21,                         // set column address
        0,                            // column start address(0-127)
        WIDTH - 1,                    // column stop address(0-127)
    };
    res = i2c_->writeWithDma(slaveAddr_, v0, sizeof(v0));
    if (res != I2C::OK)
    {
      return res;
    }
    uint8_t v1[WIDTH + 1] = {0};
    v1[0] = SSD1306_CTRL_BYTE_DATA_SINGLE;
    memcpy(v1 + 1, buffer_ + page * WIDTH, WIDTH);
    res = i2c_->writeWithDma(slaveAddr_, v1, sizeof(v1));
    if (res != I2C::OK)
    {
      return res;
    }
  }
  return res;
}

void SSD1306::writeBuffer(Motor const &motor, uint32_t i)
{
  uint8_t y = static_cast<uint8_t>(HEIGHT / 2 * i);
  char c[24] = {0};
  uint8_t *buf = buffer_ + 1;
  drawString(modeText(motor.mode()), Font_7x10, false, 0, y, buf);
  snprintf(c, sizeof(c), "%5.1fmA %.1fV", motor.getCurrent(), motor.getBusVoltage());
  drawString(c, Font_7x10, false, 0, y + 11, buf);
  snprintf(c, sizeof(c), "E:%ld V:%ld R:%ld", motor.encoder().get(), motor.getVelocity(), motor.nob().get());
  drawString(c, Font_7x10, false, 0, y + 22, buf);
}

SSD1306::SSD1306(I2C *i2c, uint8_t slaveAddr)                 //
    : i2c_(i2c),                                              //
      slaveAddr_(slaveAddr),                                  //
      buffer_(static_cast<uint8_t *>(pvPortMalloc(BUF_SIZE))) //
{
  memset(buffer_, 0, BUF_SIZE);
}

SSD1306::~SSD1306()
{
  vPortFree(buffer_);
}

I2C::Result SSD1306::black()
{
  memset(buffer_, 0, BUF_SIZE);
  return sendBufferToDevice();
}

I2C::Result SSD1306::white()
{
  memset(buffer_, 0xFF, BUF_SIZE);
  return sendBufferToDevice();
}

I2C::Result SSD1306::showText(const char *txt)
{
  memset(buffer_, 0, BUF_SIZE);
  drawString(txt, Font_11x18, false, 0, 0, buffer_);
  return sendBufferToDevice();
}

I2C::Result SSD1306::update(Application const *app)
{
  memset(buffer_, 0, BUF_SIZE);
  if (app)
  {
    for (uint32_t i = 0; i < MOTOR_COUNT; ++i)
    {
      writeBuffer(app->motor(i), i);
    }
  }
  return sendBufferToDevice();
}
