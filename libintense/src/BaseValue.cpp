// ****************************************************************************
//
// BaseValue.cpp : Abstract BaseValue.
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
#include <string.h>
#include <sstream>
#include <stdexcept>
#include <intense-io.hpp>
#include "IntenseException.hpp"
#include "BoundManager.hpp"
#include "Context.hpp"
#include "AlphaBaseValue.hpp"
#include "OmegaBaseValue.hpp"
#include "StringBaseValue.hpp"
#include "BinaryBaseValue.hpp"
#include "NumberBaseValue.hpp"
#include "BoundBaseValue.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;


BaseValue* BaseValue::factory
(Type objectType)
{
  switch (objectType) {
  case VOID:
    return NULL;
  case ALPHA:
    return new AlphaBaseValue();
  case OMEGA:
    return new OmegaBaseValue();
  case NUMBER:
    return new NumberBaseValue();
  case STRING:
    return new StringBaseValue();
  case BINARYSTRING:
    return new BinaryBaseValue();
  case BOUND:
    return new BoundBaseValue();
  default:
    ostringstream oss;

    oss << "bad BaseValue type " << objectType;
    throw IntenseException(oss.str());
  }
}


void BaseValue::factorySerialise
(BaseValue* base, BaseSerialiser& baseSerialiser, ostream& os,
 BoundManager* boundManager)
{
  int type;

  if (base != NULL) {
    type = base->getType();
  } else {
    type = BaseValue::VOID;
  }
  baseSerialiser.out(type, os);
  if (base != NULL) {
    if (base->getType() == BaseValue::BOUND) {
      ((BoundBaseValue*)base)->serialise(baseSerialiser, os, boundManager);
    } else {
      base->serialise(baseSerialiser, os);
    }
  }
}


BaseValue* BaseValue::factoryDeserialise
(BaseSerialiser& baseSerialiser, istream& is, BoundManager* boundManager)
{
  int type;
  BaseValue* base;

  // The basevalue flag:
  baseSerialiser.in(type, is);
  base = BaseValue::factory((BaseValue::Type)type);
  if (base != NULL) {
    if (base->getType() == BaseValue::BOUND) {
      ((BoundBaseValue*)base)->
        deserialise(baseSerialiser, is, boundManager);
    } else {
      base->deserialise(baseSerialiser, is);
    }
  }
  return base;
}
