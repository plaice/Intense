// ****************************************************************************
//
// BaseSerialiser.hpp - An interface for sending and receiving stdc++ strings
// and c++ base types through an iostream interface, using some (arbitrary)
// means of serialisation.
//
// Copyright 2000 by Paul Swoboda.  All Rights Reserved.
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


#ifndef __BASESERIALISER_HPP__
#define __BASESERIALISER_HPP__


namespace intense {


  namespace io {


    class SerialException
      : public std::runtime_error {

    public:

      SerialException
      (const std::string& what)
        : std::runtime_error(what)
      {}

    };


    class BaseSerialiser;


    class Serialisable {

    public:

      // Subclasses must define a serialiser and a deserialiser that use
      // pre-defined base-type serialisers.

      virtual void serialise
      (BaseSerialiser& s, std::ostream& os) const = 0;

      virtual void deserialise
      (BaseSerialiser& s, std::istream& os) = 0;

    };


    class BaseSerialiser {

    protected:

      // One meg should be plenty big enough:
      const static int defaultMaxStringLength = 1024*1024;

      int maxStringLength;

    public:

      BaseSerialiser
      (int newMaxStringLength = defaultMaxStringLength)
        : maxStringLength(newMaxStringLength)
      {}

      virtual ~BaseSerialiser
      ()
      {}

      int getMaxStringLength
      ()
      {
        return maxStringLength;
      }

      virtual void out
      (const std::string& s, std::ostream& os) = 0;

      virtual void in
      (std::string& s, std::istream& is, int maxLength = -1) = 0;

      virtual void out
      (char* src, int length, std::ostream& os) = 0;

      virtual void in
      (char*& dest, int& length, std::istream& is, int maxLength = -1) = 0;

      virtual void out
      (int i, std::ostream& os) = 0;

      virtual void in
      (int& i, std::istream& is) = 0;

      virtual void out
      (unsigned int i, std::ostream& os) = 0;

      virtual void in
      (unsigned int& i, std::istream& is) = 0;

      virtual void out
      (short int i, std::ostream& os) = 0;

      virtual void in
      (short int& i, std::istream& is) = 0;

      virtual void out
      (short unsigned int i, std::ostream& os) = 0;

      virtual void in
      (short unsigned int& i, std::istream& is) = 0;

      virtual void out
      (long int i, std::ostream& os) = 0;

      virtual void in
      (long int& i, std::istream& is) = 0;

      virtual void out
      (unsigned long int i, std::ostream& os) = 0;

      virtual void in
      (unsigned long int& i, std::istream& is) = 0;

      virtual void out
      (char c, std::ostream& os) = 0;

      virtual void in
      (char& c, std::istream& is) = 0;

      virtual void out
      (float f, std::ostream& os) = 0;

      virtual void in
      (float& f, std::istream& is) = 0;

      virtual void out
      (double d, std::ostream& os) = 0;

      virtual void in
      (double& d, std::istream& is) = 0;

      virtual void out
      (bool b, std::ostream& os) = 0;

      virtual void in
      (bool& b, std::istream& is) = 0;

      void out
      (Serialisable* serialisable, std::ostream& os)
      {
        int tempInt;

        if (serialisable != NULL) {
          tempInt = 1;
          this->out(tempInt, os);
          serialisable->serialise(*this, os);
        } else {
          tempInt = 0;
          this->out(tempInt, os);
        }
      }

      template <class SerialisableSubclass>
      void in
      (SerialisableSubclass*& dest, std::istream& is)
      {
        int tempInt;

        this->in(tempInt, is);
        if (tempInt != 0) {
          dest = new SerialisableSubclass;
          dest->deserialise(*this, is);
        } else {
          dest = NULL;
        }
      }

    };


  }


}


#endif // __BASESERIALISER_HPP__
