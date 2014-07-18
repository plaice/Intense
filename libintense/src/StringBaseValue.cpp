// ****************************************************************************
//
// StringBaseValue.cpp : Byte-array BaseValues.
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
#include "intense_config.hpp"
#include "IntenseException.hpp"
#include "BoundManager.hpp"
#include "Context.hpp"
#include "StringBaseValue.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;


string StringBaseValue::canonical
() const
{
  ostringstream oss;
  string::iterator itr;
  bool requiresQuotes = false;

  oss << '"';
  for (itr = ((string*)this)->begin(); itr != ((string*)this)->end(); itr++) {
    switch (*itr) {
    case '\\':
      oss << "\\\\";
      break;
    case '\n':
      oss << "\\n";
      break;
    case '\t':
      oss << "\\t";
      break;
    case '\v':
      oss << "\\v";
      break;
    case '\b':
      oss << "\\b";
      break;
    case '\r':
      oss << "\\r";
      break;
    case '\f':
      oss << "\\f";
      break;
    case '\a':
      oss << "\\a";
      break;
    case '\"':
      oss << "\\\"";
      break;
    default:
      oss << *itr;
      break;
    }
  }
  oss << '"';
  return oss.str();
}


bool StringBaseValue::equals
(const BaseValue& bv) const
{
  // I suppose this could be a dynamic cast, but what the heck:
  if (bv.getType() != STRING) {
    return false;
  }
  return canonical() == bv.canonical();
}


bool StringBaseValue::lessThan
(const BaseValue& bv) const
{
  if (bv.getType() != STRING) {
    // No comparing incompatible types:
    return false;
  }
  return canonical() < bv.canonical();
}


void StringBaseValue::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  baseSerialiser.out(*this, os);
}


void StringBaseValue::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  baseSerialiser.in(*this, is, maxLength);
}
