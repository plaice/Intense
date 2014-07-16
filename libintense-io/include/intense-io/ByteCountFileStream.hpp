// ****************************************************************************
//
// ByteCountFileStream.hpp - A FileStream with bytecount accounting.
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


#ifndef __BYTECOUNTFILESTREAM_HPP__
#define __BYTECOUNTFILESTREAM_HPP__


#include <iostream>
#include <streambuf>
#include <cstdio>
#include "FileStream.hpp"


namespace intense {


  namespace io {


#define FDSTREAM_DEBUG 1


    class ByteCountFileInBuffer
      : public FileInBuffer {

      unsigned int* incomingBytecount;

    public:

      ByteCountFileInBuffer
      (unsigned int& incomingBytecount_)
        : FileInBuffer(), incomingBytecount(&incomingBytecount_)
      {}

      ByteCountFileInBuffer
      (unsigned int& incomingBytecount_, int fd_)
        : FileInBuffer(fd_), incomingBytecount(&incomingBytecount_)
      {}

    protected:

      // Read at least one character:
      virtual int_type underflow
      ();

    };


    class ByteCountFileInStream
      : public std::istream {

    protected:

      ByteCountFileInBuffer buf;

    public:

      ByteCountFileInStream
      (unsigned int& incomingBytecount)
        : std::istream(0), buf(incomingBytecount, 0)
      {
        rdbuf(&buf);
      }

      ByteCountFileInStream
      (unsigned int& incomingBytecount, int fd)
        : std::istream(0), buf(incomingBytecount, fd)
      {
        rdbuf(&buf);
      }

      virtual int close
      ()
      {
        clear();
        return buf.close();
      }

      virtual void attach
      (int _fd)
      {
        buf.attach(_fd);
      }

      virtual void detach
      ()
      {
        buf.detach();
      }

    };


    class ByteCountFileOutBuffer
      : public FileOutBuffer {

      unsigned int* outgoingByteCount;

    public:

      ByteCountFileOutBuffer
      (unsigned int& outgoingByteCount_)
        : outgoingByteCount(&outgoingByteCount_)
      {}

      ByteCountFileOutBuffer
      (unsigned int& outgoingByteCount_, int _fd)
        : FileBufferBase(_fd), outgoingByteCount(&outgoingByteCount_)
      {}

    protected:

      // Write one character:
      virtual int_type overflow
      (int_type c)
      {
        (*outgoingByteCount)++;
        return FileOutBuffer::overflow(c);
      }

      // write multiple characters
      virtual std::streamsize xsputn
      (const char* s, std::streamsize num)
      {
        int bytesWritten = FileOutBuffer::xsputn(s, num);

        if (bytesWritten > 0) {
          *outgoingByteCount += bytesWritten;
        }
        return bytesWritten;
      }

    };


    class ByteCountFileOutStream
      : public std::ostream {

    protected:

      ByteCountFileOutBuffer buf;

    public:

      ByteCountFileOutStream
      (unsigned int& outgoingByteCount)
        : std::ostream(0), buf(outgoingByteCount, 0)
      {
        rdbuf(&buf);
      }

      ByteCountFileOutStream
      (unsigned int& outgoingByteCount, int fd)
        : std::ostream(0), buf(outgoingByteCount, fd)
      {
        rdbuf(&buf);
      }

      virtual int close
      ()
      {
        flush();
        clear();
        return buf.close();
      }

      virtual void attach
      (int _fd)
      {
        buf.attach(_fd);
      }

      virtual void detach()
      {
        buf.detach();
      }

    };


    class ByteCountFileInOutBuffer
      : public ByteCountFileInBuffer, public ByteCountFileOutBuffer {

    public:

      ByteCountFileInOutBuffer
      (unsigned int& incomingBytecount, unsigned int& outgoingByteCount,
       int fd_ = 0)
        : ByteCountFileInBuffer(incomingBytecount),
          ByteCountFileOutBuffer(outgoingByteCount)
      {
        fd = fd_;
      }

    };


    class ByteCountFileStream
      : public std::iostream {

    protected:

      ByteCountFileInOutBuffer buf;

    public:

      ByteCountFileStream
      (unsigned int& incomingBytecount, unsigned int& outgoingByteCount,
       int fd = 0)
        : std::iostream(0), buf(incomingBytecount, outgoingByteCount, fd)
      {
        rdbuf(&buf);
      }

      virtual int close
      ()
      {
        flush();
        clear();
        return buf.close();
      }

      virtual void attach
      (int _fd)
      {
        buf.attach(_fd);
      }

      virtual void detach
      ()
      {
        buf.detach();
      }

    };


  }


}


#endif // __BYTECOUNTFILESTREAM_HPP__
