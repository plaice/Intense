// ****************************************************************************
//
// StringUtil.java : Static utility methods for string handling and conversion
// in Intense.
//
// Copyright 2004 Paul Swoboda.
//
// This file is part of the Intense project.
//
// Intense is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Intense is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Intense; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
//
// ****************************************************************************


package intense;


import java.io.*;
import java.util.*;


/**
 * Static utility methods for string handling and conversion in Intense.
 */
public abstract class StringUtil {

  /**
   * Convert a nybble to a hex char.
   *
   * @param nybble An integer representation of the nybble.
   * @return A hex char in the range 0-9A-F representing the nybble.
   */
  public static char nybbleToChar
  (int nybble)
  {
    if ((nybble >= 0)&&(nybble <= 9)) {
      return (char)('0' + (char)nybble);
    } else {
      return (char)('a' + (char)nybble - 10);
    }
  }

  /**
   * Convert a byte to a two-character hexidecimal string.
   *
   * @param src The byte to convert.
   * @return A two-character hexidecimal string, of the form [0-9A-F][0-9A-F].
   */
  public static String byteToHexString
  (byte src)
  {
    return "" + nybbleToChar((src&0xf0) >> 4) + nybbleToChar(src&0x0f);
  }

  /**
   * Convert a byte array to a string of two-character hexidecimal byte
   * substrings.
   *
   * @param src The byte array to convert.
   * @param offset The offset in src at which to start conversion.
   * @param length The number of bytes in src to convert.
   * @param separator A substring to insert between each byte in the result.
   * @return The resulting hexadecimal string.
   */
  public static String byteArrayToHexString
  (byte[] src, int offset, int length, String separator)
  {
    StringBuffer buffer = new StringBuffer();
    int i;

    for (i = 0; (i < length - 1)&&((offset + i) < src.length - 1); i++) {
      buffer.append(byteToHexString(src[offset + i]));
      if (separator != null) {
        buffer.append(separator);
      }
    }
    buffer.append(byteToHexString(src[offset + i]));
    return buffer.toString();
  }

  /**
   * Convert a byte array to a string of two-character hexidecimal byte
   * substrings.
   *
   * @param src The byte array to convert.
   * @param separator A substring to insert between each byte in the result.
   * @return The resulting hexadecimal string.
   */
  public static String byteArrayToHexString
  (byte[] src, String separator)
  {
    return byteArrayToHexString(src, 0, src.length, separator);
  }

  /**
   * Convert a hexidecimal string to a binary string.
   *
   * @param dest An array of bytes to write the result to.
   * @param destOffset The offset in dest at which to start writing.
   * @param src An array of hexidecimal bytes in the range 0-9A-F to convert.
   * @param srcOffset The offset in src at which to start reading.
   * @param maxDestBytes The maximum number of destination bytes to write.
   * @return The number of bytes actually written.
   */
  public static int hexStringToBinaryString
  (byte[] dest, int destOffset, byte[] src, int srcOffset, int maxDestBytes)
    throws Exception
  {
    int destByteCount = 0;

    for (int i = 0; i < (src.length - srcOffset)/2; i++) {
      byte character = 0;

      if ((src[srcOffset + 2*i] >= '0')&&(src[srcOffset + 2*i] <= '9')) {
        character = (byte)((src[srcOffset + 2*i] - '0') << 4);
      } else if ((src[srcOffset + 2*i] >= 'a')&&
                 (src[srcOffset + 2*i] <= 'f')) {
        character = (byte)((src[srcOffset + 2*i] - 'a' + 10) << 4);
      } else if ((src[srcOffset + 2*i] >= 'A')&&
                 (src[srcOffset + 2*i] <= 'F')) {
        character = (byte)((src[srcOffset + 2*i] - 'A' + 10) << 4);
      } else {
        throw new Exception(
          "Invalid char '" + src[srcOffset + 2*i] +
          "' as first nybble of hex string at byte " + destByteCount
        );
      }
      if ((src[srcOffset + 2*i + 1] >= '0')&&
          (src[srcOffset + 2*i + 1] <= '9')) {
        character |= (byte)(src[srcOffset + 2*i + 1] - '0');
      } else if ((src[srcOffset + 2*i + 1] >= 'a')&&
                 (src[srcOffset + 2*i + 1] <= 'f')) {
        character |= (byte)(src[srcOffset + 2*i + 1] - (byte)'a' + (byte)10);
      } else if ((src[srcOffset + 2*i + 1] >= 'A')&&
                 (src[srcOffset + 2*i + 1] <= 'F')) {
        character |= (byte)(src[srcOffset + 2*i + 1] - (byte)'A' + (byte)10);
      } else {
        throw new Exception(
          "Invalid char '" + src[srcOffset + 2*i + 1] +
          "' as second nybble of hex string at byte " + destByteCount
        );
      }
      dest[destByteCount + destOffset] = character;
      if (++destByteCount > maxDestBytes) {
        break;
      }
    }
    return destByteCount;
  }

  /**
   * Convert a single hexadecimal character string to a UTF-8-encoded string.
   *
   * @param dest An array of bytes to write the result to.
   * @param destOffset The offset in dest at which to start writing.
   * @param src An array of 2, 4, 6 or 8 hexadecimal characters representing
   * the character to encode.
   * @param srcOffset The offset in src at which to start reading.
   * @param nSrcBytes The number of bytes (1, 2, 3 or 4) to convert and
   * UTF-8-encode from src.
   * @return The number of bytes actually written.
   */
  public static int utf8EncodeHex
  (byte[] dest, int destOffset, byte[] src, int srcOffset, int nSrcBytes)
    throws Exception
  {
    int bytesLength;
    long character = 0;
    byte[] bytes = { 0, 0, 0, 0 };
    long byteMultiplier = 1;
    int destBytesWritten;
    int i;
    int j = 0;
    int byteCount = hexStringToBinaryString(
      bytes, 4 - nSrcBytes, src, srcOffset, 4
    );
    // 2, 4, 6 or 8 hex chars:
    if (byteCount != nSrcBytes) {
      throw new Exception(
        "Unexpected byte count " + byteCount + " != " + nSrcBytes +
        " in UTF8-encode hex conversion"
      );
    }
    for (i = 0; i < nSrcBytes; i++) {
      character += ((long)(bytes[3 - i]&0xff))*byteMultiplier;
      byteMultiplier *= 256;
    }
    if (character <= 0x7f) {
      // 0xxxxxxx
      dest[destOffset + j++] =
        (byte)(0x7f&(bytes[3]));
    } else if (character <= 0x7ff) {
      // 110xxxxx  10xxxxxx
      dest[destOffset + j++] =
        (byte)(0xc0|((0x07&(bytes[2])) << 2)|((0xc0&(bytes[3])) >> 6));
      dest[destOffset + j++] =
        (byte)(0x80|(0x3f&(bytes[3])));
    } else if (character <= 0xffff) {
      // 1110xxxx  10xxxxxx  10xxxxxx
      dest[destOffset + j++] =
        (byte)(0xe0|((0xf0&(bytes[2])) >> 4));
      dest[destOffset + j++] =
        (byte)(0x80|((0x0f&(bytes[2])) << 2)|((0xc0&(bytes[3])) >> 6));
      dest[destOffset + j++] =
        (byte)(0x80|(0x3f&(bytes[3])));
    } else if (character <= 0x1fffff) {
      // 11110xxx  10xxxxxx  10xxxxxx  10xxxxxx
      dest[destOffset + j++] =
        (byte)(0xf0|((0x1c&(bytes[1])) >> 2));
      dest[destOffset + j++] =
        (byte)(0x80|((0x03&(bytes[1])) << 4)|((0xf0&(bytes[2])) >> 4));
      dest[destOffset + j++] =
        (byte)(0x80|((0x0f&(bytes[2])) << 2)|((0xc0&(bytes[3])) >> 6));
      dest[destOffset + j++] =
        (byte)(0x80|(0x3f&(bytes[3])));
    } else if (character <= 0x3ffffff) {
      // 111110xx  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx
      dest[destOffset + j++] =
        (byte)(0xf8|(0x03&(bytes[0])));
      dest[destOffset + j++] =
        (byte)(0x80|((0xfc&(bytes[1])) >> 2));
      dest[destOffset + j++] =
        (byte)(0x80|((0x03&(bytes[1])) << 4)|((0xf0&(bytes[2])) >> 4));
      dest[destOffset + j++] =
        (byte)(0x80|((0x0f&(bytes[2])) << 2)|((0xc0&(bytes[3])) >> 6));
      dest[destOffset + j++] =
        (byte)(0x80|(0x3f&(bytes[3])));
    } else if (character <= 0x7fffffff) {
      // 1111110x  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx
      dest[destOffset + j++] =
        (byte)(0xfc|((0x40&(bytes[0])) >> 6));
      dest[destOffset + j++] =
        (byte)(0x80|(0x3f&(bytes[0])));
      dest[destOffset + j++] =
        (byte)(0x80|((0xfc&(bytes[1])) >> 2));
      dest[destOffset + j++] =
        (byte)(0x80|((0x03&(bytes[1])) << 4)|((0xf0&(bytes[2])) >> 4));
      dest[destOffset + j++] =
        (byte)(0x80|((0x0f&(bytes[2])) << 2)|((0xc0&(bytes[3])) >> 6));
      dest[destOffset + j++] =
        (byte)(0x80|(0x3f&(bytes[3])));
    } else {
      throw new Exception(
        "Attempt to UTF8-encode 4-byte sequence > 0x7fffffff"
      );
    }
    return j;
  }

}
