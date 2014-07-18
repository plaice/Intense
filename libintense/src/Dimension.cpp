// ****************************************************************************
//
// Dimension.cpp : Abstract Dimension.
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
#include "intense_config.hpp"
#include "IntenseException.hpp"
#include "BoundManager.hpp"
#include "Context.hpp"
#include "container_stringtok.hpp"
#include "StringBaseValue.hpp"
#include "BinaryBaseValue.hpp"
#include "BoundBaseValue.hpp"
#include "correct_FlexLexer.h"
#include "ContextOpLexer.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;


Dimension* Dimension::factory
(Type objectType)
{
  switch (objectType) {
  case VOID:
    return NULL;
  case STRING:
    return new StringDimension();
  case INTEGER:
    return new IntegerDimension();
  default:
    ostringstream oss;

    oss << "bad Dimension type " << objectType;
    throw IntenseException(oss.str());
  }
}


void Dimension::factorySerialise
(Dimension* dimension, BaseSerialiser& baseSerialiser, ostream& os)
{
  int type;

  if (dimension != NULL) {
    type = dimension->getType();
  } else {
    type = Dimension::VOID;
  }
  baseSerialiser.out(type, os);
  if (dimension != NULL) {
    dimension->serialise(baseSerialiser, os);
  }
}


Dimension* Dimension::factoryDeserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  int type;
  Dimension* dimension;

  // The basevalue flag:
  baseSerialiser.in(type, is);
  dimension = Dimension::factory((Dimension::Type)type);
  if (dimension != NULL) {
    dimension->deserialise(baseSerialiser, is);
  }
  return dimension;
}


string CompoundDimension::canonical
() const
{
  ostringstream oss;
  list<DimensionRef>::iterator itr = ((list<DimensionRef>*)this)->begin();

  if (itr != ((list<DimensionRef>*)this)->end()) {
    oss << itr++->canonical();
  }
  while (itr != ((list<DimensionRef>*)this)->end()) {
    oss << ':' << itr++->canonical();
  }
  return oss.str();
}


void CompoundDimension::parse
(istream& is)
{
  ContextOpLexer lexer(is);
  Context::Token token;
  bool done = false;

  clear();
  while (!done) {
    switch (lexer.getToken(token)) {
    case Context::Token::DIMENSION:
      {
        DimensionRef ref(*(token.value.dimension));

        push_back(ref);
      }
      break;
    case Context::Token::ERROR:
      {
        ostringstream oss;

        oss << "Bad token \"" << *(token.value.errorText)
            << "\" in compound dimension";
        throw IntenseException(oss.str());
      }
    case Context::Token::ENDOFFILE:
      done = true;
      break;
    default:
      {
        ostringstream oss;

        oss << "Bad token type " << token.getType()
            << " in compound dimension";
        throw IntenseException(oss.str());
      }
    }
  }
  token.setVoid();
}


void CompoundDimension::parse
(const char* src)
{
  stringstream srcStream;

  srcStream << src << ':';
  srcStream.flush();
  parse(srcStream);
}


void CompoundDimension::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  int sizeInt = size();
  list<DimensionRef>::iterator itr;

  baseSerialiser.out(sizeInt, os);
  for (itr = ((CompoundDimension*)this)->begin();
       itr != ((CompoundDimension*)this)->end(); itr++) {
    itr->serialise(baseSerialiser, os);
  }
}


void CompoundDimension::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  int sizeInt;

  clear();
  baseSerialiser.in(sizeInt, is);
  if (sizeInt < 0) {
    throw IntenseException(
      "Attempt to deserialise CompoundDimension with negative size"
    );
  } else if (sizeInt > maxCompoundDimensionSize) {
    ostringstream oss;

    oss << "Attempt to deserialise CompoundDimension with size " << sizeInt
        << " > max allowed size " << maxCompoundDimensionSize;
    throw IntenseException(oss.str());
  }
  for (int i = 0; i < sizeInt; i++) {
    DimensionRef ref(baseSerialiser, is);

    push_back(ref);
  }
}
