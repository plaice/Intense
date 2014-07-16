// ****************************************************************************
//
// StringUtil.cpp : Static utility methods for string handling and conversion
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


#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include "StringUtil.hpp"
#include "IntenseException.hpp"


using namespace std;
using namespace intense;


char StringUtil::nybbleToChar
(int nybble)
{
  if ((nybble >= 0)&&(nybble <= 9)) {
    return '0' + nybble;
  } else if ((nybble >= 10)&&(nybble <= 15)) {
    return 'a' + nybble - 10;
  } else {
    ostringstream oss;

    oss << "Bad nybble value " << nybble;
    throw runtime_error(oss.str());
  }
}


int StringUtil::hexStringToBinaryString
(char* dest, const char* src, int maxDestBytes)
{
  char* srcMarker = (char*)src;
  unsigned char* destMarker = (unsigned char*)dest;
  int destByteCount = 0;

  while ((*srcMarker != 0)&&(destByteCount < maxDestBytes)) {
    unsigned char character;

    if ((*srcMarker >= '0')&&(*srcMarker <= '9')) {
      character = (*srcMarker - '0') << 4;
    } else if ((*srcMarker >= 'a')&&(*srcMarker <= 'f')) {
      character = (*srcMarker - 'a' + 10) << 4;
    } else if ((*srcMarker >= 'A')&&(*srcMarker <= 'F')) {
      character = (*srcMarker - 'A' + 10) << 4;
    } else {
      ostringstream oss;

      oss << "Invalid char '" << *srcMarker
          << "' as first nybble of hex string at byte " << destByteCount;
      throw IntenseException(oss.str());
    }
    srcMarker++;
    if ((*srcMarker >= '0')&&(*srcMarker <= '9')) {
      character += *srcMarker - '0';
    } else if ((*srcMarker >= 'a')&&(*srcMarker <= 'f')) {
      character += *srcMarker - 'a' + 10;
    } else if ((*srcMarker >= 'A')&&(*srcMarker <= 'F')) {
      character += *srcMarker - 'A' + 10;
    } else {
      ostringstream oss;

      oss << "Invalid char '" << *srcMarker
          << "' as second nybble of hex string at byte " << destByteCount;
      throw IntenseException(oss.str());
    }
    srcMarker++;
    *destMarker++ = character;
    destByteCount++;
  }
  return destByteCount;
}


int StringUtil::utf8EncodeHex
(char* dest, const char* src, int nSrcBytes)
{
  int bytesLength;
  unsigned long character = 0;
  unsigned char bytes[4] = { 0, 0, 0, 0};
  unsigned long byteMultiplier = 1;
  unsigned char* destMarker = (unsigned char*)dest;
  int destBytesWritten;
  int i;

  // 2, 4, 6 or 8 hex chars:
  if ((i = hexStringToBinaryString((char*)(bytes + 4 - nSrcBytes), src, 4)) !=
      nSrcBytes) {
    ostringstream oss;

    oss << "Unexpected byte count " << i << " != " << nSrcBytes
        << " in UTF8-encode hex conversion of hex src \"" << src << "\"";
    throw runtime_error(oss.str());
  }
  for (i = 0; i < nSrcBytes; i++) {
    character += ((unsigned long)(bytes[3 - i]))*byteMultiplier;
    byteMultiplier *= 256;
  }
  if (character <= 0x7f) {
    // 0xxxxxxx
    *destMarker++ = 0x7f&(bytes[3]);
  } else if (character <= 0x7ff) {
    // 110xxxxx  10xxxxxx
    *destMarker++ = 0xc0|((0x07&(bytes[2])) << 2)|((0xc0&(bytes[3])) >> 6);
    *destMarker++ = 0x80|(0x3f&(bytes[3]));
  } else if (character <= 0xffff) {
    // 1110xxxx  10xxxxxx  10xxxxxx
    *destMarker++ = 0xe0|((0xf0&(bytes[2])) >> 4);
    *destMarker++ = 0x80|((0x0f&(bytes[2])) << 2)|((0xc0&(bytes[3])) >> 6);
    *destMarker++ = 0x80|(0x3f&(bytes[3]));
  } else if (character <= 0x1fffff) {
    // 11110xxx  10xxxxxx  10xxxxxx  10xxxxxx
    *destMarker++ = 0xf0|((0x1c&(bytes[1])) >> 2);
    *destMarker++ = 0x80|((0x03&(bytes[1])) << 4)|((0xf0&(bytes[2])) >> 4);
    *destMarker++ = 0x80|((0x0f&(bytes[2])) << 2)|((0xc0&(bytes[3])) >> 6);
    *destMarker++ = 0x80|(0x3f&(bytes[3]));
  } else if (character <= 0x3ffffff) {
    // 111110xx  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx
    *destMarker++ = 0xf8|(0x03&(bytes[0]));
    *destMarker++ = 0x80|((0xfc&(bytes[1])) >> 2);
    *destMarker++ = 0x80|((0x03&(bytes[1])) << 4)|((0xf0&(bytes[2])) >> 4);
    *destMarker++ = 0x80|((0x0f&(bytes[2])) << 2)|((0xc0&(bytes[3])) >> 6);
    *destMarker++ = 0x80|(0x3f&(bytes[3]));
  } else if (character <= 0x7fffffff) {
    // 1111110x  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx  10xxxxxx
    *destMarker++ = 0xfc|((0x40&(bytes[0])) >> 6);
    *destMarker++ = 0x80|(0x3f&(bytes[0]));
    *destMarker++ = 0x80|((0xfc&(bytes[1])) >> 2);
    *destMarker++ = 0x80|((0x03&(bytes[1])) << 4)|((0xf0&(bytes[2])) >> 4);
    *destMarker++ = 0x80|((0x0f&(bytes[2])) << 2)|((0xc0&(bytes[3])) >> 6);
    *destMarker++ = 0x80|(0x3f&(bytes[3]));
  } else {
    throw runtime_error("Attempt to UTF8-encode 4-byte sequence > 0x7fffffff");
  }
  return destMarker - (unsigned char*)dest;
}
