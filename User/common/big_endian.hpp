/// @file      common/big_endian.hpp
/// @author    Hiroshi Mikuriya
/// @copyright Copyright© 2022 Hiroshi Mikuriya
///
/// DO NOT USE THIS SOFTWARE WITHOUT THE SOFTWARE LICENSE AGREEMENT.

#pragma once

#include <cstdint>

namespace mik
{
namespace impl
{
/// @brief Nバイトコピー
/// @tparam N コピーバイト数
/// @param [in] src コピー元
/// @param [out] dst コピー先
template <uint32_t N>
void rcopy(void const *src, void *dst);
/// @brief 2バイトコピー
/// @param [in] src コピー元
/// @param [out] dst コピー先
template <>
inline void rcopy<2>(void const *src, void *dst)
{
  static_cast<uint8_t *>(dst)[0] = static_cast<uint8_t const *>(src)[1];
  static_cast<uint8_t *>(dst)[1] = static_cast<uint8_t const *>(src)[0];
}
/// @brief 4バイトコピー
/// @param [in] src コピー元
/// @param [out] dst コピー先
template <>
inline void rcopy<4>(void const *src, void *dst)
{
  rcopy<2>(&static_cast<uint8_t const *>(src)[0], &static_cast<uint8_t *>(dst)[2]);
  rcopy<2>(&static_cast<uint8_t const *>(src)[2], &static_cast<uint8_t *>(dst)[0]);
}
/// @brief 8バイトコピー
/// @param [in] src コピー元
/// @param [out] dst コピー先
template <>
inline void rcopy<8>(void const *src, void *dst)
{
  rcopy<4>(&static_cast<uint8_t const *>(src)[0], &static_cast<uint8_t *>(dst)[4]);
  rcopy<4>(&static_cast<uint8_t const *>(src)[4], &static_cast<uint8_t *>(dst)[0]);
}
/// @brief 16バイトコピー
/// @param [in] src コピー元
/// @param [out] dst コピー先
template <>
inline void rcopy<16>(void const *src, void *dst)
{
  rcopy<8>(&static_cast<uint8_t const *>(src)[0], &static_cast<uint8_t *>(dst)[8]);
  rcopy<8>(&static_cast<uint8_t const *>(src)[8], &static_cast<uint8_t *>(dst)[0]);
}
/// @tparam T 読み書きする型
template <typename T>
class BE
{
  /// @brief 型とバイナリ列を変換する共用体
  union U
  {
    T v;                  ///< 型
    uint8_t a[sizeof(v)]; ///< バイナリ列
  };

public:
  /// @brief バイト列から値を読み出す
  /// @param [in] bytes バイト列
  /// @return 値
  static T get(void const *bytes)
  {
    U u;
    rcopy<sizeof(u)>(bytes, u.a);
    return u.v;
  }
  /// @brief バッファへ値を書き込む
  /// @param [in] buffer バッファ
  /// @param [in] v 値
  static void set(void *buffer, T v)
  {
    U u;
    u.v = v;
    rcopy<sizeof(u)>(u.a, buffer);
  }
};
} // namespace impl

/// @brief ビッグエンディアン変換クラスのテンプレート
template <typename T>
class BE;
/// @brief ビッグエンディアン変換クラス（bool版）
template <>
class BE<bool>
{
public:
  static bool get(void const *src) { return !!static_cast<uint8_t const *>(src)[0]; }
  static void set(void *dst, bool src) { static_cast<uint8_t *>(dst)[0] = src ? 1 : 0; }
};
/// @brief ビッグエンディアン変換クラス（uint8_t版）
template <>
class BE<uint8_t>
{
public:
  static uint8_t get(void const *src) { return static_cast<uint8_t const *>(src)[0]; }
  static void set(void *dst, uint8_t src) { static_cast<uint8_t *>(dst)[0] = src; }
};
/// @brief ビッグエンディアン変換クラス（int8_t版）
template <>
class BE<int8_t>
{
public:
  static int8_t get(void const *src) { return static_cast<int8_t>(BE<uint8_t>::get(src)); }
  static void set(void *dst, int8_t src) { BE<uint8_t>::set(dst, static_cast<uint8_t>(src)); }
};
/// @brief ビッグエンディアン変換クラス（uint16_t版）
template <>
class BE<uint16_t> : public impl::BE<uint16_t>
{
};
/// @brief ビッグエンディアン変換クラス（int16_t版）
template <>
class BE<int16_t> : public impl::BE<int16_t>
{
};
/// @brief ビッグエンディアン変換クラス（uint32_t版）
template <>
class BE<uint32_t> : public impl::BE<uint32_t>
{
};
/// @brief ビッグエンディアン変換クラス（int32_t版）
template <>
class BE<int32_t> : public impl::BE<int32_t>
{
};
/// @brief ビッグエンディアン変換クラス（uint64_t版）
template <>
class BE<uint64_t> : public impl::BE<uint64_t>
{
};
/// @brief ビッグエンディアン変換クラス（int64_t版）
template <>
class BE<int64_t> : public impl::BE<int64_t>
{
};
/// @brief ビッグエンディアン変換クラス（float版）
template <>
class BE<float> : public impl::BE<float>
{
};
/// @brief ビッグエンディアン変換クラス（double版）
template <>
class BE<double> : public impl::BE<double>
{
};
/// @brief ビッグエンディアン変換クラス（long double版）
template <>
class BE<long double> : public impl::BE<long double>
{
};
} // namespace mik
