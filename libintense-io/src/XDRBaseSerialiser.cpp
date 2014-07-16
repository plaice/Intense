// ****************************************************************************
//
// XDRBaseSerialiser.cpp - A class for sending and receiving stdc++ strings and
// c++ base types through XDR binary serialisation, a subclass of
// BaseSerialiser.
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


using namespace std;


#include <rpc/types.h>
#include <rpc/xdr.h>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include "intense_io_config.hpp"
#include "BaseSerialiser.hpp"
#include "XDRBaseSerialiser.hpp"


using namespace intense::io;


static char* xdrOutError = "Outbound serialisation of base type failed";
static char* xdrInError = "Inbound deserialisation of base type failed";


XDRBaseSerialiser::XDRBaseSerialiser
(int initMaxStringLength)
  : BaseSerialiser(initMaxStringLength)
{
  xdrmem_create(&encoder, outBuffer, bufferSize, XDR_ENCODE);
  xdrmem_create(&decoder, inBuffer, bufferSize, XDR_DECODE);
}


void XDRBaseSerialiser::out
(const string& s, ostream& os)
{
  int i = s.length();

  xdr_setpos(&encoder, 0);
  if (!xdr_int(&encoder, &i)) {
    throw SerialException(xdrOutError);
  }
  os.write(outBuffer, 4);
  os.write(s.data(), i);
}


void XDRBaseSerialiser::in
(string& s, istream& is, int maxLength)
{
  char *buffer;
  int i;

  if (maxLength < 0) {
    maxLength = maxStringLength;
  }
  xdr_setpos(&decoder, 0);
  is.read(inBuffer, 4);
  if (!xdr_int(&decoder, &i)) throw SerialException(xdrInError);
  if (i <= maxLength) {
    if ((buffer = new char[i + 1]) == NULL) {
      throw SerialException("String deserialisation buffer allocation failed");
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


void XDRBaseSerialiser::out
(char* src, int length, ostream& os)
{
  int signedLength = length;

  if (length < 0) {
    ostringstream oss;
    oss << "Attempt to serialise a binary string of negative length "
        << length;
    throw SerialException(oss.str());
  }
  xdr_setpos(&encoder, 0);
  if (!xdr_int(&encoder, &signedLength)) {
    throw SerialException(xdrOutError);
  }
  os.write(outBuffer, 4);
  if (length > 0) {
    os.write(src, length);
  }
}


void XDRBaseSerialiser::in
(char*& dest, int& length, istream& is, int maxLength)
{
  int i;

  if (maxLength < 0) {
    maxLength = maxStringLength;
  }
  xdr_setpos(&decoder, 0);
  is.read(inBuffer, 4);
  if (!xdr_int(&decoder, &i)) {
    throw SerialException(xdrInError);
  }
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

    oss << "Attempt to deserialise a binary string of length "
        << i << " > " << maxLength;
    throw SerialException(oss.str());
  }
}


void XDRBaseSerialiser::out
(int i, ostream& os)
{
  xdr_setpos(&encoder, 0);
  if (!xdr_int(&encoder, &i)) {
    throw SerialException(xdrOutError);
  }
  os.write(outBuffer, 4);
}


void XDRBaseSerialiser::in
(int& i, istream& is)
{
  xdr_setpos(&decoder, 0);
  is.read(inBuffer, 4);
  if (!xdr_int(&decoder, &i)) {
    throw SerialException(xdrInError);
  }
}


void XDRBaseSerialiser::out
(unsigned int i, ostream& os)
{
  xdr_setpos(&encoder, 0);
  if (!xdr_u_int(&encoder, &i)) {
    throw SerialException(xdrOutError);
  }
  os.write(outBuffer, 4);
}


void XDRBaseSerialiser::in
(unsigned int& i, istream& is)
{
  xdr_setpos(&decoder, 0);
  is.read(inBuffer, 4);
  if (!xdr_u_int(&decoder, &i)) {
    throw SerialException(xdrInError);
  }
}


void XDRBaseSerialiser::out
(short int i, ostream& os)
{
  xdr_setpos(&encoder, 0);
  if (!xdr_short(&encoder, &i)) {
    throw SerialException(xdrOutError);
  }
  os.write(outBuffer, 4);
}


void XDRBaseSerialiser::in
(short int& i, istream& is)
{
  xdr_setpos(&decoder, 0);
  is.read(inBuffer, 4);
  if (!xdr_short(&decoder, &i)) {
    throw SerialException(xdrInError);
  }
}


void XDRBaseSerialiser::out
(short unsigned int i, ostream& os)
{
  xdr_setpos(&encoder, 0);
  if (!xdr_u_short(&encoder, &i)) {
    throw SerialException(xdrOutError);
  }
  os.write(outBuffer, 4);
}


void XDRBaseSerialiser::in
(short unsigned int& i, istream& is)
{
  xdr_setpos(&decoder, 0);
  is.read(inBuffer, 4);
  if (!xdr_u_short(&decoder, &i)) {
    throw SerialException(xdrInError);
  }
}


void XDRBaseSerialiser::out
(long long int i, ostream& os)
{
#if HAVE_XDR_HYPER
  xdr_setpos(&encoder, 0);
  if (!xdr_hyper(&encoder, &i)) {
    throw SerialException(xdrOutError);
  }
  os.write(outBuffer, 8);
#else // HAVE_XDR_HYPER
#if BIG_ENDIAN_SYSTEM
  os.write((char*)&i, 8);
#else // BIG_ENDIAN_SYSTEM
  int j;
  for (j = 0; j < 8; ++j) {
    outBuffer[j] = (i >> (j*8))&0xff;
  }
#endif // BIG_ENDIAN_SYSTEM
#endif // HAVE_XDR_HYPER
}


void XDRBaseSerialiser::in
(long long int& i, istream& is)
{
#if HAVE_XDR_HYPER
  xdr_setpos(&decoder, 0);
  is.read(inBuffer, 8);
  if (!xdr_hyper(&decoder, &i)) {
    throw SerialException(xdrInError);
  }
#else // HAVE_XDR_HYPER
#if BIG_ENDIAN_SYSTEM
  is.read((char*)&i, 8);
#else // BIG_ENDIAN_SYSTEM
  is.read(inBuffer, 8);
  int j;
  for (j = 0; j < 8; ++j) {
    ((char*)&i)[j] = inBuffer[7 - j];
  }
#endif // BIG_ENDIAN_SYSTEM
#endif // HAVE_XDR_HYPER
}


void XDRBaseSerialiser::out
(unsigned long long int i, ostream& os)
{
#if HAVE_XDR_U_HYPER
  xdr_setpos(&encoder, 0);
  if (!xdr_u_hyper(&encoder, &i)) {
    throw SerialException(xdrOutError);
  }
  os.write(outBuffer, 8);
#else // HAVE_XDR_U_HYPER
#if BIG_ENDIAN_SYSTEM
  os.write((char*)&i, 8);
#else // BIG_ENDIAN_SYSTEM
  int j;
  for (j = 0; j < 8; ++j) {
    outBuffer[j] = (i >> (j*8))&0xff;
  }
#endif // BIG_ENDIAN_SYSTEM
#endif // HAVE_XDR_U_HYPER
}


void XDRBaseSerialiser::in
(unsigned long long int& i, istream& is)
{
#if HAVE_XDR_U_HYPER
  xdr_setpos(&decoder, 0);
  is.read(inBuffer, 8);
  if (!xdr_u_hyper(&decoder, &i)) {
    throw SerialException(xdrInError);
  }
#else // HAVE_XDR_U_HYPER
#if BIG_ENDIAN_SYSTEM
  is.read((char*)&i, 8);
#else // BIG_ENDIAN_SYSTEM
  is.read(inBuffer, 8);
  int j;
  for (j = 0; j < 8; ++j) {
    ((char*)&i)[j] = inBuffer[7 - j];
  }
#endif // BIG_ENDIAN_SYSTEM
#endif // HAVE_XDR_U_HYPER
}


void XDRBaseSerialiser::out
(char c, ostream& os)
{
  xdr_setpos(&encoder, 0);
  if (!xdr_char(&encoder, &c)) {
    throw SerialException(xdrOutError);
  }
  os.write(outBuffer, 4);
}


void XDRBaseSerialiser::in
(char& c, istream& is)
{
  xdr_setpos(&decoder, 0);
  is.read(inBuffer, 4);
  if (!xdr_char(&decoder, &c)) {
    throw SerialException(xdrInError);
  }
}


void XDRBaseSerialiser::out
(float f, ostream& os)
{
  xdr_setpos(&encoder, 0);
  if (!xdr_float(&encoder, &f)) {
    throw SerialException(xdrOutError);
  }
  os.write(outBuffer, 4);
}


void XDRBaseSerialiser::in
(float& f, istream& is)
{
  xdr_setpos(&decoder, 0);
  is.read(inBuffer, 4);
  if (!xdr_float(&decoder, &f)) {
    throw SerialException(xdrInError);
  }
}


void XDRBaseSerialiser::out
(double d, ostream& os)
{
  xdr_setpos(&encoder, 0);
  if (!xdr_double(&encoder, &d)) {
    throw SerialException(xdrOutError);
  }
  os.write(outBuffer, 8);
}


void XDRBaseSerialiser::in
(double& d, istream& is)
{
  xdr_setpos(&decoder, 0);
  is.read(inBuffer, 8);
  if (!xdr_double(&decoder, &d)) {
    throw SerialException(xdrInError);
  }
}


void XDRBaseSerialiser::out
(bool b, ostream& os)
{
  char tempBool = (char)b;

  os.write(&tempBool, 1);
}


void XDRBaseSerialiser::in
(bool& b, istream& is)
{
  char tempBool;

  is.read(&tempBool, 1);
  b = (bool)tempBool;
}
