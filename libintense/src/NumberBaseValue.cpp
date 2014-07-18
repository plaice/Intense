// ****************************************************************************
//
// NumberBaseValue.cpp : Number-valued BaseValues.
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


#include <stdlib.h>
#include <list>
#include <map>
#include <string>
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <intense-io.hpp>
#include "intense_config.hpp"
#include "BoundManager.hpp"
#include "IntenseException.hpp"
#include "Context.hpp"
#include "StringBaseValue.hpp"
#include "NumberBaseValue.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;


string NumberBaseValue::canonical
() const
{
  ostringstream oss;

  oss << number;
  return oss.str();
}


NumberBaseValue* NumberBaseValue::clone
() const
{
  return new NumberBaseValue(*this);
}


bool NumberBaseValue::equals
(const BaseValue& bv) const
{
  return (bv.getType() == NUMBER)&&(number == ((NumberBaseValue&)bv).number);
}


bool NumberBaseValue::refinesTo
(const BaseValue& bv) const
{
  return (bv.getType() == OMEGA)||
    ((bv.getType() == NUMBER)&&(number <= ((NumberBaseValue&)bv).number));
}


bool NumberBaseValue::lessThan
(const BaseValue& bv) const
{
  return (bv.getType() == NUMBER)&&(number < ((NumberBaseValue&)bv).number);
}


BaseValue& NumberBaseValue::assign
(const BaseValue& bv)
{
  if (bv.getType() != NUMBER) {
    throw IntenseException(
      "Incompatible types in BaseValue assignment"
    );
  }
  number = ((NumberBaseValue&)bv).number;
  return *this;
}


BaseValue& NumberBaseValue::assign
(const char* s)
{
  char* endPtr;

  number = strtod(s, &endPtr);
  if (endPtr == s) {
    ostringstream oss;

    oss << "Attempt to assign non-long-double-convertable string \""
        << s << "\" as value to NumberBaseValue";
    throw IntenseException(oss.str());
  }
  return *this;
}


void NumberBaseValue::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  baseSerialiser.out(number, os);
}


void NumberBaseValue::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  baseSerialiser.in(number, is);
}
