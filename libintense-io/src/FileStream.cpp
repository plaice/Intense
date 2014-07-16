// ****************************************************************************
//
// FileStream.cpp - Since methods dealing with raw POSIX file descriptors
// are no longer part of the iostream hierarchy in the stdc++ standard,
// this should cure a few related issues.  Code is paraphrased from
// was Nicolai Josuttis' "The C++ Standard Library", Addison
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
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <errno.h>
#include "FileStream.hpp"


using namespace intense::io;


#define FDSTREAM_DEBUG 0


int FileBufferBase::close
()
{
  int rc;

  if (fd >= 0) {
    rc = ::close(fd);
    fd = -1;
    return rc;
  } else {
    return 1;
  }
}


void FileInBuffer::resetBuffer
()
{
  setg(buffer + putbackSize, buffer + putbackSize, buffer + putbackSize);
}


int FileInBuffer::close
()
{
  resetBuffer();
  return FileBufferBase::close();
}


// Read one character:
std::streambuf::int_type FileInBuffer::underflow
()
{
  int numPutback;
  int bytesRead;

#if FDSTREAM_DEBUG
  cerr << "FileInBuffer underflow().\n";
#endif
  if (fd < 0) {
#if FDSTREAM_DEBUG
    cerr << "FileInBuffer underflow() returning -1.\n";
#endif
    return -1;
  }
  if (gptr() < egptr()) {
    return std::char_traits<char>::not_eof(*gptr());
  }
  numPutback = gptr() - eback();
  if (numPutback > putbackSize) {
    numPutback = putbackSize;
  }
  std::memmove(
    buffer + (putbackSize - numPutback), gptr() - numPutback, numPutback
  );
  bytesRead = ::read(fd, buffer + putbackSize, bufferSize);
  if (bytesRead < 0) {
    ostringstream oss;

    oss << "read failed on descriptor " << fd << " with errno " << errno
        << ": \"" << strerror(errno) << "\"";
    throw runtime_error(oss.str());
  } else if (bytesRead == 0) {
    return EOF;
  }
  setg(
    buffer + (putbackSize - numPutback), buffer + putbackSize,
    buffer + putbackSize + bytesRead
  );
  return std::char_traits<char>::not_eof(*gptr());
}


// Write one character:
std::streambuf::int_type FileOutBuffer::overflow
(int_type c)
{
  char z = c;

#if FDSTREAM_DEBUG
  cerr << "FileOutBuffer overflow('" << c << "').\n";
#endif
  if (fd < 0) {
    return -1;
  }
  if (::write(fd, &z, 1) != 1) {
    throw runtime_error(
      "Failed to write single character in FileOutBuffer::overflow"
    );
  }
  return c;
}


// write multiple characters
std::streamsize FileOutBuffer::xsputn
(const char* s, std::streamsize num)
{
  int rc;

  if (fd < 0) {
    return -1;
  }
  rc = ::write(fd, s, num);
  if (rc != num) {
    ostringstream oss;

    oss << "Failed to write " << num << " bytes in FileOutBuffer::overflow";
    throw runtime_error(oss.str());
  }
#if FDSTREAM_DEBUG
  cerr << "FileOutBuffer (fd = " << fd << ") xsputn("
       << num << ") = " << rc << ".\n";
#endif
  return rc;
}
