// ****************************************************************************
//
// FileStream.hpp - Since methods dealing with raw POSIX file descriptors
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


#ifndef __FILESTREAM_HPP__
#define __FILESTREAM_HPP__


#include <iostream>
#include <streambuf>
#include <cstdio>


namespace intense {


  namespace io {


    class FileBufferBase
      : public std::streambuf {

    protected:

      int fd;

    public:

      FileBufferBase
      ()
        : fd(-1)
      {
      }

      FileBufferBase
      (int _fd)
        : fd(_fd)
      {
      }

      virtual int close
      ();

      virtual void attach
      (int _fd)
      {
        fd = _fd;
      }

      virtual void detach
      ()
      {
        fd = 0;
      }

    };


    class FileInBuffer
      : public virtual FileBufferBase {

    protected:

      static const int bufferSize = 1024;

      static const int putbackSize = 4;

      char buffer[bufferSize + putbackSize];

      virtual void resetBuffer();

    public:

      FileInBuffer
      ()
      {
        resetBuffer();
      }

      FileInBuffer
      (int _fd)
        : FileBufferBase(_fd)
      {
        resetBuffer();
      }

      virtual int close();

    protected:

      // Read one character:
      virtual int_type underflow
      ();

    };


    class FileInStream
      : public std::istream {

    protected:

      FileInBuffer buf;

    public:

      FileInStream
      ()
        : std::istream(0), buf(0)
      {
        rdbuf(&buf);
      }

      FileInStream
      (int fd)
        : std::istream(0), buf(fd)
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


    class FileOutBuffer
      : public virtual FileBufferBase {

    public:

      FileOutBuffer
      ()
      {
      }

      FileOutBuffer
      (int _fd)
        : FileBufferBase(_fd)
      {
      }

    protected:

      // Write one character:
      virtual int_type overflow
      (int_type c);

      // write multiple characters
      virtual std::streamsize xsputn
      (const char* s, std::streamsize num);

    };


    class FileOutStream
      : public std::ostream {

    protected:

      FileOutBuffer buf;

    public:

      FileOutStream
      ()
        : std::ostream(0), buf(0)
      {
        rdbuf(&buf);
      }

      FileOutStream
      (int fd)
        : std::ostream(0), buf(fd)
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


    class FileInOutBuffer
      : public FileInBuffer, public FileOutBuffer {

    public:

      FileInOutBuffer
      (int _fd)
        : FileInBuffer(_fd), FileOutBuffer(_fd)
      {}

      virtual int close
      ()
      {
        return FileInBuffer::close();
      }

    };


    class FileStream
      : public std::iostream {

    protected:

      FileInOutBuffer buf;

    public:

      FileStream
      ()
        : std::iostream(0), buf(0)
      {
        rdbuf(&buf);
      }

      FileStream
      (int fd)
        : std::iostream(0), buf(0)
      {
        rdbuf(&buf);
        attach(fd);
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


#endif // __FILESTREAM_HPP__
