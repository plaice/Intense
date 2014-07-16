// ****************************************************************************
//
// BoundBaseValue.hpp : BaseValues to represent bound entities.
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


#ifndef __BOUNDBASEVALUE_HPP__
#define __BOUNDBASEVALUE_HPP__


namespace intense {


  class BoundBaseValue
    : public BaseValue {

    const static int maxLength = 1024*1024;

  public:

    void* bound;

    // A length of -1 indicates that bound points to unpacked object data.
    int length;

    BoundManager* manager;

    BoundBaseValue
    ()
      : BaseValue(), bound(NULL), length(-1), manager(NULL)
    {}

    BoundBaseValue
    (const void* newBound)
      : BaseValue(), bound((void*)newBound), length(-1), manager(NULL)
    {}

    BoundBaseValue
    (const void* newBound, int newLength)
      : BaseValue(), bound((void*)newBound), length(newLength), manager(NULL)
    {}

    BoundBaseValue
    (const void* newBound, int newLength, BoundManager& newManager)
      : BaseValue(), bound((void*)newBound), length(newLength),
        manager(&newManager)
    {}

    virtual ~BoundBaseValue
    ();

    Type getType
    () const
    {
      return BaseValue::BOUND;
    }

    virtual std::string canonical
    () const
    {
      return std::string("");
    }

    virtual std::string toString
    () const
    {
      return std::string("");
    }

    virtual BoundBaseValue* clone
    () const
    {
      return new BoundBaseValue(bound);
    }

    virtual bool equals
    (const BaseValue& bv) const
    {
      return (bv.getType() == BOUND)&&(bound == ((BoundBaseValue&)bv).bound);
    }

    virtual bool refinesTo
    (const BaseValue& bv) const
    {
      return false;
    }

    virtual bool lessThan
    (const BaseValue& bv) const
    {
      return false;
    }

    virtual BaseValue& assign
    (const BaseValue& bv)
    {
      if (bv.getType() == BOUND) {
        bound = ((BoundBaseValue&)bv).bound;
      }
      return *this;
    }

    virtual void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os) const
    {
      serialise(baseSerialiser, os, NULL);
    }

    virtual void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is)
    {
      deserialise(baseSerialiser, is, NULL);
    }

    void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os,
     BoundManager* boundSerialiser) const;

    void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is,
     BoundManager* boundSerialiser);

  };


}


#endif // __BOUNDBASEVALUE_HPP__
