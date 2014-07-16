// ****************************************************************************
//
// StringBaseValue.hpp : BaseValues with stdc++ strings.
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


#ifndef __STRINGBASEVALUE_HPP__
#define __STRINGBASEVALUE_HPP__


namespace intense {


  // A basic string-valued BaseValue subclass:
  class StringBaseValue
    : public BaseValue, public std::string {

    const static int maxLength = 1024*1024;

  public:

    StringBaseValue
    (const BaseValue& bv)
      : BaseValue(), std::string()
    {
      std::string::operator=(bv.canonical());
    }

    StringBaseValue
    (const char *s)
      : BaseValue(), std::string()
    {
      std::string::operator=(s);
    }

    StringBaseValue
    (const std::string& s)
      : BaseValue(), std::string()
    {
      std::string::operator=(s);
    }

    StringBaseValue
    ()
      : BaseValue(), std::string()
    {
      std::string::operator=("");
    }

    virtual ~StringBaseValue
    ()
    {}

    Type getType
    () const
    {
      return STRING;
    }

    virtual std::string canonical
    () const;

    virtual std::string toString
    () const
    {
      return *this;
    }

    virtual StringBaseValue* clone
    () const
    {
      return new StringBaseValue(*this);
    }

    virtual bool equals
    (const BaseValue& bv) const;

    virtual bool refinesTo
    (const BaseValue& bv) const
    {
      if (bv.getType() == OMEGA) {
        return true;
      } else if (bv.getType() != STRING) {
        return false;
      } else {
        return canonical() == bv.canonical();
      }
    }

    virtual bool lessThan
    (const BaseValue& bv) const;

    virtual BaseValue& assign
    (const BaseValue& bv)
    {
      std::string::operator=(bv.canonical());
      return *this;
    }

    virtual BaseValue& assign
    (const char *s)
    {
      std::string::operator=(s);
      return *this;
    }

    virtual BaseValue& assign
    (const std::string& s)
    {
      std::string::operator=(s);
      return *this;
    }

    virtual void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

    virtual void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is);

  };


}


#endif // __STRINGBASEVALUE_HPP__
