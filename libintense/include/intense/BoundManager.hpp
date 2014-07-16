// ****************************************************************************
//
// BoundManager.hpp : Serialisation and deserialisation of BoundBaseValues.
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


#ifndef __BOUNDMANGER_HPP__
#define __BOUNDMANGER_HPP__


namespace intense {


  class BoundManager {

  public:

    virtual void pack
    (char*& packed, int& packedLength, void* boundSource) = 0;

    virtual void unpack
    (void*& destObject, char* unpacked, int unpackedLength) = 0;

    virtual void destruct
    (void* boundObject) = 0;

  };


}


#endif // __BOUNDMANGER_HPP__
