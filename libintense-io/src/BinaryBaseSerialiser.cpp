// ****************************************************************************
//
// BinaryBaseSerialiser.cpp - A class for sending and receiving stdc++
// strings and c++ base types through straight binary serialisation, a
// subclass of BaseSerialiser.
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


using namespace std;


#include <cstring>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include "BaseSerialiser.hpp"
#include "BinaryBaseSerialiser.hpp"


using namespace intense::io;


const char* BinaryBaseSerialiser::archTypeStrings[] = {
  "UNKNOWN", "xdr", "sparc_sunos56_gcc3", "i586_linux_gcc3"
};


const BinaryBaseSerialiser::CompatibleArchitecture
BinaryBaseSerialiser::stringToCompatibleArchitecture
(const char* type)
{
  for (int i = 0; i <= ARCH_i586_linux_gcc3; i++) {
    if (strncmp(archTypeStrings[i], type, 100) == 0) {
      return (CompatibleArchitecture)i;
    }
  }
  {
    ostringstream oss;
    char buffer[64];

    buffer[63] = 0;
    strncpy(buffer, type, 63);
    oss << "Invalid BinaryBaseSerialiser::CompatibleArchitecture type string \""
        << buffer << "\"";
    throw SerialException(oss.str());
  }
}


void BinaryBaseSerialiser::out
(const string& s, ostream& os)
{
  unsigned int i = s.length();

  os.write((char*)&i, sizeof(i));
  os.write(s.data(), i);
}


void BinaryBaseSerialiser::in
(string& s, istream& is, int maxLength)
{
  unsigned int i;

  if (maxLength < 0) {
    maxLength = maxStringLength;
  }
  is.read((char*)&i, sizeof(i));
  if (i <= maxLength) {
    char *buffer;

    if (!(buffer = new char[i + 1])) {
      throw SerialException("String buffer allocation failed");
    }
    buffer[i] = 0;
    is.read(buffer, i);
    s = buffer;
    delete[] buffer;
  } else {
    ostringstream oss;

    oss << "Attempt to deserialise a string of length "
        << i << " > " << maxLength;
    throw SerialException(oss.str());
  }
}


void BinaryBaseSerialiser::out
(char* src, int length, ostream& os)
{
  if (length < 0) {
    ostringstream oss;
    oss << "Attempt to serialise a binary string of negative length "
        << length;
    throw SerialException(oss.str());
  }
  os.write((char*)&length, sizeof(length));
  if (length > 0) {
    os.write((char*)src, length);
  }
}


void BinaryBaseSerialiser::in
(char*& dest, int& length, istream& is, int maxLength)
{
  int i;

  if (maxLength < 0) {
    maxLength = maxStringLength;
  }
  is.read((char*)&i, sizeof(i));
  if (i < 0) {
    ostringstream oss;

    oss << "Attempt to deserialise a binary string of negative length " << i;
    throw SerialException(oss.str());
  } else if (i == 0) {
    length = i;
  } else if (i <= maxLength) {
    length = i;
    dest = new char[i];
    is.read(dest, i);
  } else {
    ostringstream oss;

    oss << "Attempt to deserialise a binary string of length " << i << " > "
        << maxLength;
    throw SerialException(oss.str());
  }
}


void BinaryBaseSerialiser::out
(int i, ostream& os)
{
  os.write((char*)&i, sizeof(i));
}


void BinaryBaseSerialiser::in
(int& i, istream& is)
{
  is.read((char*)&i, sizeof(i));
}


void BinaryBaseSerialiser::out
(unsigned int i, ostream& os)
{
  os.write((char*)&i, sizeof(i));
}


void BinaryBaseSerialiser::in
(unsigned int& i, istream& is)
{
  is.read((char*)&i, sizeof(i));
}


void BinaryBaseSerialiser::out
(short int i, ostream& os)
{
  os.write((char*)&i, sizeof(i));
}


void BinaryBaseSerialiser::in
(short int& i, istream& is)
{
  is.read((char*)&i, sizeof(i));
}


void BinaryBaseSerialiser::out
(short unsigned int i, ostream& os)
{
  os.write((char*)&i, sizeof(i));
}


void BinaryBaseSerialiser::in
(short unsigned int& i, istream& is)
{
  is.read((char*)&i, sizeof(i));
}


void BinaryBaseSerialiser::out
(long long int i, ostream& os)
{
  os.write((char*)&i, sizeof(i));
}


void BinaryBaseSerialiser::in
(long long int& i, istream& is)
{
  is.read((char*)&i, sizeof(i));
}


void BinaryBaseSerialiser::out
(unsigned long long int i, ostream& os)
{
  os.write((char*)&i, sizeof(i));
}


void BinaryBaseSerialiser::in
(unsigned long long int& i, istream& is)
{
  is.read((char*)&i, sizeof(i));
}


void BinaryBaseSerialiser::out
(char c, ostream& os)
{
  os.write(&c, sizeof(c));
}


void BinaryBaseSerialiser::in
(char& c, istream& is)
{
  is.read(&c, sizeof(c));
}


void BinaryBaseSerialiser::out
(float f, ostream& os)
{
  os.write((char*)&f, sizeof(f));
}


void BinaryBaseSerialiser::in
(float& f, istream& is)
{
  is.read((char*)&f, sizeof(f));
}


void BinaryBaseSerialiser::out
(double d, ostream& os)
{
  os.write((char*)&d, sizeof(d));
}


void BinaryBaseSerialiser::in
(double& d, istream& is)
{
  is.read((char*)&d, sizeof(d));
}


void BinaryBaseSerialiser::out
(bool b, ostream& os)
{
  char bAsChar = (char)b;

  os.write(&bAsChar, sizeof(char));
}


void BinaryBaseSerialiser::in
(bool& b, istream& is)
{
  char bAsChar;

  is.read(&bAsChar, sizeof(char));
  b = (bool)bAsChar;
}
