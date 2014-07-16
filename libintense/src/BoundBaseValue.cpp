// ****************************************************************************
//
// BoundBaseValue.cpp : BaseValues to represent bound entities.
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
#include "BoundBaseValue.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;


BoundBaseValue::~BoundBaseValue
()
{
  if (length > 0) {
    delete (char*)bound;
  } else if (length < 0) {
    if (manager != NULL) {
      manager->destruct(bound);
    }
  }
}


void BoundBaseValue::serialise
(BaseSerialiser& baseSerialiser, ostream& os,
 BoundManager* boundSerialiser) const
{
  if ((boundSerialiser != NULL)&&(length < 0)) {
    char* packed;
    int packedLength;

    boundSerialiser->pack(packed, packedLength, bound);
    if (packed != NULL) {
      baseSerialiser.out(packed, packedLength, os);
      delete packed;
    } else {
      baseSerialiser.out(NULL, 0, os);
    }
  } else {
    if (bound != NULL) {
      baseSerialiser.out((char*)bound, length, os);
    } else {
      baseSerialiser.out(NULL, length, os);
    }
  }
}


void BoundBaseValue::deserialise
(BaseSerialiser& baseSerialiser, istream& is,
 BoundManager* boundSerialiser)
{
  int packedLength;
  char* packed;

  baseSerialiser.in(packed, packedLength, is, maxLength);
  bound = NULL;
  if (boundSerialiser != NULL) {
    boundSerialiser->unpack(bound, packed, packedLength);
  }
  if (bound == NULL) {
    // We just store the data (good for middleman-type Participants, etc.):
    bound = packed;
    length = packedLength;
  } else {
    length = -1;
    // NOTE: We do NOT delete packed.  It is up to the BaseSerialiser to
    // delete it if necessary.
  }
}
