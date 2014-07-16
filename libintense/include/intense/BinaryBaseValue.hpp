// ****************************************************************************
//
// BinaryBaseValue.hpp : BaseValues with binary values.
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


#ifndef __BINARYBASEVALUE_HPP__
#define __BINARYBASEVALUE_HPP__


namespace intense {


  // A basic binary-valued BaseValue subclass:
  class BinaryBaseValue
    : public BaseValue {

    const static int maxLength = 1024*1024;

  public:

    unsigned char* content;

    int contentLength;

    BinaryBaseValue
    (int newContentLength);

    BinaryBaseValue
    (const void* srcContent, int srcContentLength);

    BinaryBaseValue
    (const BinaryBaseValue& src);

    BinaryBaseValue
    ()
      : BaseValue(), content(NULL), contentLength(0)
    {}

    virtual ~BinaryBaseValue
    ();

    Type getType
    () const
    {
      return BaseValue::BINARYSTRING;
    }

    virtual std::string canonical
    () const;

    virtual std::string toString
    () const
    {
      return std::string((const char*)content, contentLength);
    }

    virtual BinaryBaseValue* clone
    () const;

    virtual bool equals
    (const BaseValue& bv) const;

    virtual bool refinesTo
    (const BaseValue& bv) const;

    virtual bool lessThan
    (const BaseValue& bv) const;

    virtual BaseValue& assign
    (const BaseValue& bv);

    virtual BaseValue& assign
    (const char*);

    virtual BaseValue& operator=
    (const char* s)
    {
            return assign(s);
    }

    virtual BaseValue& assign
    (const std::string& s);

    virtual BaseValue& operator=
    (const std::string& s)
    {
      return assign(s);
    }

    virtual void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

    virtual void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is);

  private:

    char nybbleToChar
    (int nybble) const;

  };


}


#endif // __BINARYBASEVALUE_HPP__
