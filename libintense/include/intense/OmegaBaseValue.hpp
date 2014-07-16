// ****************************************************************************
//
// OmegaBaseValue.h++ : Maximally defined base values (in the intensional
// sense).
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


#ifndef __OMEGABASEVALUE_HPP__
#define __OMEGABASEVALUE_HPP__


namespace intense {


  // A minimally-defined base value.
  class OmegaBaseValue
    : public BaseValue {

  public:

    OmegaBaseValue
    ()
      : BaseValue()
    {}

    virtual ~OmegaBaseValue
    ()
    {}

    Type getType
    () const
    {
      return BaseValue::OMEGA;
    }

    virtual std::string canonical
    () const
    {
      return std::string("^");
    }

    virtual std::string toString
    () const
    {
      return std::string("^");
    }

    virtual OmegaBaseValue* clone
    () const
    {
      return new OmegaBaseValue();
    }

    virtual bool equals
    (const BaseValue& bv) const
    {
      if (bv.getType() == OMEGA) {
        return true;
      }
      return false;
    }

    virtual bool refinesTo
    (const BaseValue& bv) const
    {
      if (bv.getType() == OMEGA) {
        return true;
      }
      return false;
    }

    virtual bool lessThan
    (const BaseValue& bv) const
    {
      if (bv.getType() == OMEGA) {
        return false;
      }
      return true;
    }

    virtual BaseValue& assign
    (const BaseValue& bv)
    {
      // bogus:
      return *this;
    }

    virtual BaseValue& operator=
    (const char *s)
    {
      // bogus:
      return *this;
    }

    virtual BaseValue& operator=
    (const std::string& s)
    {
    }

    virtual void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os) const
    {}

    virtual void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is)
    {}

  };


}


#endif // __OMEGABASEVALUE_HPP__
