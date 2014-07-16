// ****************************************************************************
//
// BinaryBaseValue.cpp : Byte-array BaseValues.
//
// Copyright 2001, 2002 Paul Swoboda.
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


#include <list>
#include <map>
#include <string>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <intense-io.hpp>
#include "IntenseException.hpp"
#include "BoundManager.hpp"
#include "Context.hpp"
#include "StringBaseValue.hpp"
#include "BinaryBaseValue.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;


BinaryBaseValue::BinaryBaseValue
(int newContentLength)
  : BaseValue()
{
  contentLength = newContentLength;
  content = new unsigned char[contentLength];
}


BinaryBaseValue::BinaryBaseValue
(const void* srcContent, int srcContentLength)
  : BaseValue()
{
  contentLength = srcContentLength;
  content = new unsigned char[contentLength];
  memcpy(content, srcContent, contentLength);
}


BinaryBaseValue::BinaryBaseValue
(const BinaryBaseValue& src)
  : BaseValue()
{
  if (src.content != NULL) {
    contentLength = src.contentLength;
    content = new unsigned char[contentLength];
    memcpy(content, src.content, contentLength);
  }
}


BinaryBaseValue::~BinaryBaseValue
()
{
  if (content != NULL) {
    delete content;
  }
}


string BinaryBaseValue::canonical
() const
{
  ostringstream oss;
  unsigned char* marker = (unsigned char*)content;

  oss << "#";
  for (int i = 0; i < contentLength; i++) {
    oss << nybbleToChar(((*marker)&0xf0) >> 4)
        << nybbleToChar((*marker)&0x0f);
    marker++;
  }
  return oss.str();
}


char BinaryBaseValue::nybbleToChar
(int nybble) const
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


BinaryBaseValue* BinaryBaseValue::clone
() const
{
  return new BinaryBaseValue(*this);
}


bool BinaryBaseValue::equals
(const BaseValue& bv) const
{
  if (bv.getType() != BINARYSTRING) {
    return false;
  } else {
    BinaryBaseValue* bvPtr = (BinaryBaseValue*)(&bv);
    if (bvPtr->contentLength != contentLength) return false;
    if (memcmp(bvPtr->content, content, contentLength)) return false;
  }
  return true;
}


bool BinaryBaseValue::refinesTo
(const BaseValue& bv) const
{
  if (bv.getType() == OMEGA) {
    return true;
  }
  return equals(bv);
}


bool BinaryBaseValue::lessThan
(const BaseValue& bv) const
{
  if (bv.getType() != BINARYSTRING) {
    // No comparing incompatible types:
    return false;
  } else {
    int maxLength;
    BinaryBaseValue* bvPtr = (BinaryBaseValue*)(&bv);
    if (bvPtr->contentLength > contentLength) {
      maxLength = contentLength;
    } else {
      maxLength = bvPtr->contentLength;
    }
    if ((memcmp(content, bvPtr->content, contentLength)) < 0) {
      return true;
    } else {
      return false;
    }
  }
}


BaseValue& BinaryBaseValue::assign
(const BaseValue& bv)
{
  if (content != NULL) {
    delete[] content;
  }
  if (bv.getType() == STRING) {
    StringBaseValue* bvPtr = (StringBaseValue*)(&bv);
    contentLength = bvPtr->length();
    content = new unsigned char[contentLength];
    memcpy(content, bvPtr->string::data(), contentLength);
  } else if (bv.getType() == BINARYSTRING) {
    BinaryBaseValue* bvPtr = (BinaryBaseValue*)(&bv);
    contentLength = bvPtr->contentLength;
    content = new unsigned char[contentLength];
    memcpy(content, bvPtr->content, contentLength);
  } else {
    string bvCanonical = bv.canonical();
    contentLength = bvCanonical.length();
    content = new unsigned char[contentLength];
    memcpy(content, bvCanonical.data(), contentLength);
  }
  return *this;
}


BaseValue& BinaryBaseValue::assign
(const char* s)
{
  contentLength = strlen(s);
  content = new unsigned char[contentLength];
  memcpy(content, (void*)s, contentLength);
  return *this;
}


BaseValue& BinaryBaseValue::assign
(const string& s)
{
  contentLength = s.length();
  content = new unsigned char[contentLength];
  memcpy(content, s.data(), contentLength);
  return *this;
}


void BinaryBaseValue::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  baseSerialiser.out((char*)content, contentLength, os);
}


void BinaryBaseValue::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  baseSerialiser.in((char*&)content, contentLength, is, maxLength);
}
