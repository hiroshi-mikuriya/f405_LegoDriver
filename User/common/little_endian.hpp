/// @file      common/little_endian.hpp
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
void copy(void const *src, void *dst);
/// @brief 2バイトコピー
/// @param [in] src コピー元
/// @param [out] dst コピー先
template <>
inline void copy<2>(void const *src, void *dst)
{
  static_cast<uint8_t *>(dst)[0] = static_cast<uint8_t const *>(src)[0];
  static_cast<uint8_t *>(dst)[1] = static_cast<uint8_t const *>(src)[1];
}
/// @brief 4バイトコピー
/// @param [in] src コピー元
/// @param [out] dst コピー先
template <>
inline void copy<4>(void const *src, void *dst)
{
  copy<2>(&static_cast<uint8_t const *>(src)[0], &static_cast<uint8_t *>(dst)[0]);
  copy<2>(&static_cast<uint8_t const *>(src)[2], &static_cast<uint8_t *>(dst)[2]);
}
/// @brief 8バイトコピー
/// @param [in] src コピー元
/// @param [out] dst コピー先
template <>
inline void copy<8>(void const *src, void *dst)
{
  copy<4>(&static_cast<uint8_t const *>(src)[0], &static_cast<uint8_t *>(dst)[0]);
  copy<4>(&static_cast<uint8_t const *>(src)[4], &static_cast<uint8_t *>(dst)[4]);
}
/// @brief 16バイトコピー
/// @param [in] src コピー元
/// @param [out] dst コピー先
template <>
inline void copy<16>(void const *src, void *dst)
{
  copy<8>(&static_cast<uint8_t const *>(src)[0], &static_cast<uint8_t *>(dst)[0]);
  copy<8>(&static_cast<uint8_t const *>(src)[8], &static_cast<uint8_t *>(dst)[8]);
}
/// @tparam T 読み書きする型
template <typename T>
class LE
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
    copy<sizeof(u)>(bytes, u.a);
    return u.v;
  }
  /// @brief バッファへ値を書き込む
  /// @param [in] buffer バッファ
  /// @param [in] v 値
  static void set(void *buffer, T v)
  {
    U u;
    u.v = v;
    copy<sizeof(u)>(u.a, buffer);
  }
};
} // namespace impl

/// @brief リトルエンディアン変換クラスのテンプレート
template <typename T>
class LE;
/// @brief リトルエンディアン変換クラス（bool版）
template <>
class LE<bool>
{
public:
  static bool get(void const *src) { return !!static_cast<uint8_t const *>(src)[0]; }
  static void set(void *dst, bool src) { static_cast<uint8_t *>(dst)[0] = src ? 1 : 0; }
};
/// @brief リトルエンディアン変換クラス（uint8_t版）
template <>
class LE<uint8_t>
{
public:
  static uint8_t get(void const *src) { return static_cast<uint8_t const *>(src)[0]; }
  static void set(void *dst, uint8_t src) { static_cast<uint8_t *>(dst)[0] = src; }
};
/// @brief リトルエンディアン変換クラス（int8_t版）
template <>
class LE<int8_t>
{
public:
  static int8_t get(void const *src) { return static_cast<int8_t>(LE<uint8_t>::get(src)); }
  static void set(void *dst, int8_t src) { LE<uint8_t>::set(dst, static_cast<uint8_t>(src)); }
};
/// @brief リトルエンディアン変換クラス（uint16_t版）
template <>
class LE<uint16_t> : public impl::LE<uint16_t>
{
};
/// @brief リトルエンディアン変換クラス（int16_t版）
template <>
class LE<int16_t> : public impl::LE<int16_t>
{
};
/// @brief リトルエンディアン変換クラス（uint32_t版）
template <>
class LE<uint32_t> : public impl::LE<uint32_t>
{
};
/// @brief リトルエンディアン変換クラス（int32_t版）
template <>
class LE<int32_t> : public impl::LE<int32_t>
{
};
/// @brief リトルエンディアン変換クラス（uint64_t版）
template <>
class LE<uint64_t> : public impl::LE<uint64_t>
{
};
/// @brief リトルエンディアン変換クラス（int64_t版）
template <>
class LE<int64_t> : public impl::LE<int64_t>
{
};
/// @brief リトルエンディアン変換クラス（float版）
template <>
class LE<float> : public impl::LE<float>
{
};
/// @brief リトルエンディアン変換クラス（double版）
template <>
class LE<double> : public impl::LE<double>
{
};
/// @brief リトルエンディアン変換クラス（long double版）
template <>
class LE<long double> : public impl::LE<long double>
{
};
} // namespace mik
