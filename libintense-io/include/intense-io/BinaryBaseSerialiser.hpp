// ****************************************************************************
//
// BinaryBaseSerialiser.hpp - A class for sending and receiving stdc++ strings
// and c++ base types through straight binary serialisation, subclass of
// BaseSerial.
//
// Copyright 2002 by Paul Swoboda.  All Rights Reserved.
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


#ifndef __BINARYBASESERIALISER_HPP__
#define __BINARYBASESERIALISER_HPP__


namespace intense {


  namespace io {


#include "CompatibleArchitecture.hpp"


    class BinaryBaseSerialiser
      : public virtual BaseSerialiser {

    public:

      typedef enum {
        ARCH_unknown, ARCH_xdr, ARCH_sparc_sunos56_gcc3, ARCH_i586_linux_gcc3
      } CompatibleArchitecture;

    protected:

      // Defined in BinaryBaseSerialiser.cpp:
      static const char* archTypeStrings[];

    public:

      static const char* compatibleArchitectureToString
      (CompatibleArchitecture type)
      {
        return archTypeStrings[type];
      }

      static const CompatibleArchitecture stringToCompatibleArchitecture
      (const char* type);

      static const CompatibleArchitecture compatibleArchitecture
      ()
      {
        return INTENSE_IO_COMPATIBLE_ARCHITECTURE;
      }

      static const char* compatibleArchitectureString
      ()
      {
        return archTypeStrings[INTENSE_IO_COMPATIBLE_ARCHITECTURE];
      }

    public:

      BinaryBaseSerialiser
      (int initMaxStringLength = defaultMaxStringLength)
        :BaseSerialiser(initMaxStringLength)
      {}

      virtual ~BinaryBaseSerialiser
      ()
      {}

      void out
      (const std::string& s, std::ostream& os);

      void in
      (std::string& s, std::istream& is, int maxLength = -1);

      void out
      (char* src, int length, std::ostream& os);

      void in
      (char*& dest, int& length, std::istream& is, int maxLength = -1);

      void out
      (int i, std::ostream& os);

      void in
      (int& i, std::istream& is);

      void out
      (unsigned int i, std::ostream& os);

      void in
      (unsigned int& i, std::istream& is);

      void out
      (short int i, std::ostream& os);

      void in
      (short int& i, std::istream& is);

      void out
      (short unsigned int i, std::ostream& os);

      void in
      (short unsigned int& i, std::istream& is);

      virtual void out
      (long long int i, std::ostream& os);

      virtual void in
      (long long int& i, std::istream& is);

      virtual void out
      (unsigned long long int i, std::ostream& os);

      virtual void in
      (unsigned long long int& i, std::istream& is);

      void out
      (char c, std::ostream& os);

      void in
      (char& c, std::istream& is);

      void out
      (float f, std::ostream& os);

      void in
      (float& f, std::istream& is);

      void out
      (double d, std::ostream& os);

      void in
      (double& d, std::istream& is);

      void out
      (bool b, std::ostream& os);

      void in
      (bool& b, std::istream& is);

    };


  }


}


#endif // __BINARYBASESERIALISER_HPP__
