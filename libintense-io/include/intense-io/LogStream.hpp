// ****************************************************************************
// LogStream.hpp - Simple, threadsafe logging with timestamps.
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


#ifndef __LOGSTREAM_HPP__
#define __LOGSTREAM_HPP__


#include "intense_io_config.hpp"


#include <pthread.h>
#include <sys/timeb.h>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <map>


namespace intense {


  namespace io {


    class LogException
      : public std::runtime_error {

    public:

      LogException
      (const std::string& what)
        : std::runtime_error(what)
      {}

    };


    typedef enum { ENDLOG } EndLogType;


    // This should _definitely_ be elsewhere...
    class MultiMutex {

#if HAVE_PTHREAD_MUTEX_ERRORCHECK

      pthread_mutexattr_t mutexAttribute;

#else // HAVE_PTHREAD_MUTEX_ERRORCHECK

      pthread_t mutexOwner;

      pthread_mutex_t mutexOwnerMutex;

#endif // HAVE_PTHREAD_MUTEX_ERRORCHECK

      pthread_mutex_t mutex;

    public:

      MultiMutex
      ();

      ~MultiMutex
      ();

      void lock
      ();

      void unlock
      ();

    };


    class LogStream;


    struct LessThanCharPtr {

      bool operator()
        (const char* c1, const char* c2)
      {
        return strcmp(c1, c2) < 0;
      }

    };


    class Log {

    public:

      typedef enum {
        CATASTROPHIC, FATAL, CRITICAL, ERROR, WARN, NOTICE,
        DEBUG, DEBUG1, DEBUG2, EVEN_LOGGING_THE_LOGGING
      } Priority;

      static Priority toPriority
      (const char* priorityString);

    protected:

      static const char* priorityStrings[];


      static std::map<const char*, Priority, LessThanCharPtr>*
      priorityStringMap;

      static std::map<const char*, Priority, LessThanCharPtr>*
      initPriorityStringMap
      ();

      struct {
        unsigned short years;
        unsigned short months;
        unsigned short days;
        unsigned short hours;
        unsigned short minutes;
        unsigned short seconds;
        unsigned short milliseconds;
      } timestamp;

      LogStream* logStream;

      std::ostringstream message;

      Priority priority;

      Priority defaultPriority;

      bool messageStarted;

      bool autoTimestamp;

      MultiMutex mutex;

      Log
      ()
      {}

      void updateTimestamp
      ();

      void lock
      ();

      void unlock
      ();

    public:

      Log
      (LogStream& logStream_);

      ~Log
      ();

      // Change the default message priority for this Log:
      void setDefaultPriority
      (Priority defaultPriority_);

      // Overrides auto-timestamping for the current message (done at the end
      // of the message), setting the timestamp for the message to the time of
      // call to stampTime():
      void stampTime
      ();

      template <class Type>
      Log& operator<<
      (const Type& moreMessage)
      {
        lock();
        message << moreMessage;
        return *this;
      }

      // This terminates the current log message and writes it to the
      // underlying ostream - if stampTime() was not called during the message,
      // a timestamp is generated.
      //
      // Usage:
      //
      // log << "The value is " << value << "!" << ENDLOG;
      //
      void operator<<
      (EndLogType);

      // This changes the priority of the current log message, which is changed
      // back to the default priority at the start of the next message.
      Log& operator<<
      (Priority newPriority);

    };


    class LogStream {

      friend class Log;

    protected:

      std::ostream* os;

      Log::Priority logLevel;

      bool useTimestamps;

      bool usePriorities;

      MultiMutex mutex;

      LogStream
      (Log::Priority logLevel_ = Log::DEBUG2,
       bool useTimestamps_ = true, bool usePriorities_ = true);

      void lock
      ();

      void unlock
      ();

      std::ostream& streamRef
      ();

    public:

      LogStream
      (std::ostream& os_, Log::Priority logLevel_ = Log::NOTICE,
       bool useTimestamps_ = true, bool usePriorities_ = true);

      virtual ~LogStream
      ();

      // Assumes lock is acquired:
      bool isOpen
      ();

      virtual void close
      ();

      void setLogLevel
      (Log::Priority logLevel_);

    };


    class FileLogStream
      : public LogStream {

    protected:

      std::string path;

      std::ofstream *ofs;

    public:

      FileLogStream
      (Log::Priority logLevel = Log::NOTICE,
       bool useTimestamps_ = true, bool usePriorities_ = true);

      FileLogStream
      (const std::string& filepath, Log::Priority logLevel = Log::NOTICE,
       std::ios_base::openmode mode = std::ios::app);

      ~FileLogStream();

      void setFile
      (const std::string& filepath,
       std::ios_base::openmode mode = std::ios::app);

      virtual void close
      ();

    };


  }


}


#endif // __LOGSTREAM_HPP__
