// ****************************************************************************
//
// ByteCountFileStream.cpp - A FileStream with bytecount accounting.  Code is
// paraphrased from was Nicolai Josuttis' "The C++ Standard Library", Addison
// Weseley, 1999, pp. 672-673.
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
// Copyright notice of Nicolai Josuttis (note, changes have been made - not the
// original, which, if you are looking for it, can be found at
// http://www.josuttis.com/cppcode/fdstream.html):
//
// (C) Copyright Nicolai M. Josuttis 2001.
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
// ****************************************************************************




using namespace std;


#include <unistd.h>
#include <typeinfo>
#include <streambuf>
#include <cstdio>
#include <cstring>
#include <iostream>
#include "ByteCountFileStream.hpp"


using namespace intense::io;


std::streambuf::int_type ByteCountFileInBuffer::underflow
()
{
  int numPutback;
  int bytesRead;

  if (fd < 0) {
    return -1;
  }
  if (gptr() < egptr()) {
    return *gptr();
  }
  numPutback = gptr() - eback();
  if (numPutback > putbackSize) {
    numPutback = putbackSize;
  }
  std::memmove(
    buffer + (putbackSize - numPutback), gptr() - numPutback, numPutback
  );
  bytesRead = ::read(fd, buffer + putbackSize, bufferSize);
  if (bytesRead <= 0) {
    return EOF;
  }
  (*incomingBytecount) += bytesRead;
  setg(
    buffer + (putbackSize - numPutback), buffer + putbackSize,
    buffer + putbackSize + bytesRead
  );
  return *gptr();
}
