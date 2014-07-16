// ****************************************************************************
//
// LogStream.cpp - Simple, threadsafe logging with timestamps.
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


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <errno.h>
#include "LogStream.hpp"


using namespace std;
using namespace intense::io;


const char* Log::priorityStrings[] = {
  "CATASTROPHIC", "FATAL", "CRITICAL", "ERROR", "WARN", "NOTICE",
  "DEBUG", "DEBUG1", "DEBUG2", "EVEN_LOGGING_THE_LOGGING"
};


map<const char*, Log::Priority, LessThanCharPtr>* Log::priorityStringMap =
  Log::initPriorityStringMap();


map<const char*, Log::Priority, LessThanCharPtr>* Log::initPriorityStringMap
()
{
  map<const char*, Log::Priority, LessThanCharPtr>* newMap =
    new map<const char*, Log::Priority, LessThanCharPtr>;

  for (int i = Log::CATASTROPHIC; i <= Log::EVEN_LOGGING_THE_LOGGING; i++) {
    (*newMap)[priorityStrings[i]] = (Log::Priority)i;
  }
  return newMap;
}


Log::Priority Log::toPriority
(const char* priorityString)
{
  map<const char*, Log::Priority, LessThanCharPtr>::iterator itr =
    priorityStringMap->find(priorityString);

  if (itr == priorityStringMap->end()) {
    ostringstream oss;

    oss << "Invalid log priority string \"" << priorityString << "\"";
    throw LogException(oss.str());
  } else {
    return itr->second;
  }
}


MultiMutex::MultiMutex
()
{
#if HAVE_PTHREAD_MUTEX_ERRORCHECK
  // We want an error-checking mutex so the same caller can lock it
  // multiple times:
  pthread_mutexattr_init(&mutexAttribute);
  pthread_mutexattr_settype(&mutexAttribute, PTHREAD_MUTEX_ERRORCHECK_NP);
  pthread_mutex_init(&mutex, &mutexAttribute);
#else // HAVE_PTHREAD_MUTEX_ERRORCHECK
  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_init(&mutexOwnerMutex, NULL);
  // Hopefully, the os won't ever use 0 to represent a live thread:
  mutexOwner = 0;
#endif // HAVE_PTHREAD_MUTEX_ERRORCHECK
}


MultiMutex::~MultiMutex
()
{
#if HAVE_PTHREAD_MUTEX_ERRORCHECK
  pthread_mutexattr_destroy(&mutexAttribute);
#else // HAVE_PTHREAD_MUTEX_ERRORCHECK
  pthread_mutex_destroy(&mutexOwnerMutex);
#endif // HAVE_PTHREAD_MUTEX_ERRORCHECK
  pthread_mutex_destroy(&mutex);
}


void MultiMutex::lock
()
{
#if HAVE_PTHREAD_MUTEX_ERRORCHECK
  pthread_mutex_lock(&mutex);
#else //HAVE_PTHREAD_MUTEX_ERRORCHECK
  pthread_mutex_lock(&mutexOwnerMutex);
  if (mutexOwner == pthread_self()) {
    pthread_mutex_unlock(&mutexOwnerMutex);
    return;
  }
  pthread_mutex_unlock(&mutexOwnerMutex);
  pthread_mutex_lock(&mutex);
  pthread_mutex_lock(&mutexOwnerMutex);
  mutexOwner = pthread_self();
  pthread_mutex_unlock(&mutexOwnerMutex);
#endif //HAVE_PTHREAD_MUTEX_ERRORCHECK
}


void MultiMutex::unlock
()
{
#if HAVE_PTHREAD_MUTEX_ERRORCHECK
  pthread_mutex_unlock(&mutex);
#else //HAVE_PTHREAD_MUTEX_ERRORCHECK
  pthread_mutex_lock(&mutexOwnerMutex);
  if (mutexOwner != pthread_self()) {
    ostringstream oss;

    oss << "INTERNAL ERROR: thread:" << pthread_self() <<
      ": Attept to unlock MultiMutex not owned by self";
    throw LogException(oss.str());
  }
  mutexOwner = 0;
  pthread_mutex_unlock(&mutexOwnerMutex);
  pthread_mutex_unlock(&mutex);
#endif //HAVE_PTHREAD_MUTEX_ERRORCHECK
}


void Log::lock
()
{
  mutex.lock();
}


void Log::unlock
()
{
  mutex.unlock();
}


void Log::setDefaultPriority
(Priority defaultPriority_)
{
  lock();
  defaultPriority = defaultPriority_;
  unlock();
}


void Log::stampTime
()
{
  lock();
  updateTimestamp();
  autoTimestamp = false;
}


Log::Log
(LogStream& logStream_)
  : logStream(&logStream_), message(), defaultPriority(NOTICE),
    messageStarted(false), autoTimestamp(true), mutex()
{
  priority = defaultPriority;
}


Log::~Log
()
{
}


void Log::updateTimestamp
()
{
  struct timeval tv;
  struct tm *time;

  gettimeofday(&tv, 0);
  timestamp.milliseconds = (unsigned short)(tv.tv_usec / 1000);
  time = localtime((time_t *)&(tv.tv_sec));
  timestamp.years = (unsigned short)(time->tm_year + 1900);
  timestamp.months = (unsigned short)(time->tm_mon + 1);
  timestamp.days = (unsigned short)(time->tm_mday);
  timestamp.hours = (unsigned short)(time->tm_hour);
  timestamp.minutes = (unsigned short)(time->tm_min);
  timestamp.seconds = (unsigned short)(time->tm_sec);
}


void Log::operator<<
(EndLogType)
{
  lock();
  logStream->lock();
  if (!(logStream->isOpen())) {
    unlock();
    logStream->unlock();
    return;
  }
  if (((int)logStream->logLevel) >= ((int)priority)) {
    if (logStream->useTimestamps) {
      if (autoTimestamp) {
        updateTimestamp();
      }
      logStream->streamRef()
        << "[" << timestamp.years << "/" << timestamp.months << "/"
        << timestamp.days << " " << timestamp.hours << ":"
        << timestamp.minutes << ":" << timestamp.seconds << ":"
        << timestamp.milliseconds << "] ";
    }
    if (logStream->usePriorities) {
      logStream->streamRef() << '[' << priorityStrings[priority] << "] ";
    }
    logStream->streamRef() << message.str() << "\n";
    logStream->streamRef().flush();
  }
  logStream->unlock();
  autoTimestamp = true;
  message.str("");
  priority = defaultPriority;
  unlock();
}


Log& Log::operator<<
(Priority newPriority)
{
  lock();
  priority = newPriority;
  return *this;
}


LogStream::LogStream
(Log::Priority logLevel_, bool useTimestamps_,
 bool usePriorities_)
  : os(NULL), logLevel(logLevel_), useTimestamps(useTimestamps_),
    usePriorities(usePriorities_), mutex()
{}


LogStream::LogStream
(ostream& os_, Log::Priority logLevel_, bool useTimestamps_,
 bool usePriorities_)
  : os(&os_), logLevel(logLevel_), useTimestamps(useTimestamps_),
    usePriorities(usePriorities_), mutex()
{}


LogStream::~LogStream
()
{}


// Assumes lock is acquired:
bool LogStream::isOpen
()
{
  return os != NULL;
}


void LogStream::close
()
{
  lock();
  os = NULL;
  unlock();
}


void LogStream::lock
()
{
  mutex.lock();
}


void LogStream::unlock
()
{
  mutex.unlock();
}


ostream& LogStream::streamRef
()
{
  if (os != NULL) {
    return *os;
  } else {
    throw runtime_error("Attempt to get reference to NULL log stream");
  }
}


void LogStream::setLogLevel
(Log::Priority logLevel_)
{
  lock();
  logLevel = logLevel_;
  unlock();
}


FileLogStream::FileLogStream
(Log::Priority logLevel, bool useTimestamps_, bool usePriorities_)
  : LogStream(logLevel), path(), ofs(NULL)
{}


FileLogStream::FileLogStream
(const string& filepath, Log::Priority logLevel, ios_base::openmode mode)
  : LogStream(logLevel), path(filepath), ofs(new ofstream)
{
  ofs->open(path.data(), mode);
  if (!(ofs->is_open())) {
    // Might at well spew an error message, at this point:
    cerr << "ERROR: Can't open logfile " << filepath << "\n";
    // And we might as well just bind the log output to cerr:
    os = &cerr;
  } else {
    os = ofs;
  }
}


FileLogStream::~FileLogStream
()
{
  if (ofs != NULL) {
    ofs->close();
    delete ofs;
  }
}


void FileLogStream::setFile
(const string& filepath, ios_base::openmode mode)
{
  lock();
  // First, we need to close any open files:
  if (ofs != NULL) {
    ofs->close();
  } else {
    ofs = new ofstream;
  }
  path = filepath;
  ofs->open(path.data(), mode);
  if (!(ofs->is_open())) {
    ostringstream oss;

    // We might as well just bind the log output to cerr:
    os = &cerr;
    oss << "Couldn't open FileLogStream path \"" << path << "\".";
    unlock();
    throw runtime_error(oss.str());
  } else {
    os = ofs;
  }
  unlock();
}


void FileLogStream::close
()
{
  lock();
  if (ofs != NULL) {
    ofs->close();
    ofs = NULL;
    os = NULL;
  }
  unlock();
}
