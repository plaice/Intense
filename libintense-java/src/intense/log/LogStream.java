// ****************************************************************************
//
// LogStream.java - Simple, threadsafe logging with timestamps.
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


package intense.log;


import java.util.concurrent.locks.*;
import java.io.*;
import intense.*;


/**
 * A destination stream for log messages generated by intense.log.Log.
 */
public class LogStream {

  protected PrintStream os;

  protected int logLevel;

  protected boolean useTimestamps;

  protected boolean usePriorities;

  protected ReentrantLock mutex;

  protected void lock
  ()
  {
    if (!mutex.isHeldByCurrentThread()) {
      mutex.lock();
    }
  }

  protected void unlock
  ()
  {
    mutex.unlock();
  }

  protected PrintStream streamRef
  ()
  {
    return os;
  }

  private void construct
  (PrintStream os, int logLevel, boolean useTimestamps,
   boolean usePriorities)
    throws LogException
  {
    this.os = os;
    Log.checkPriority(logLevel);
    this.logLevel = logLevel;
    this.useTimestamps = useTimestamps;
    this.usePriorities = usePriorities;
    mutex = new ReentrantLock();
  }

  /**
   * @param os A destination PrintStream.
   */
  public LogStream
  (PrintStream os)
    throws LogException
  {
    construct(os, Log.NOTICE, true, true);
  }

  /**
   * @param os A destination PrintStream.
   * @param logLevel The priority of log messages accepted by this stream
   * (messages with less significant priorities are dropped).
   */
  public LogStream
  (PrintStream os, int logLevel)
    throws LogException
  {
    construct(os, logLevel, true, true);
  }

  /**
   * @param os A destination PrintStream.
   * @param logLevel The priority of log messages accepted by this stream
   * (messages with less significant priorities are dropped).
   * @param useTimestamps Denotes whether this LogStream will print a timestamp
   * before each log message.
   */
  public LogStream
  (PrintStream os, int logLevel, boolean useTimestamps)
    throws LogException
  {
    construct(os, logLevel, useTimestamps, true);
  }

  /**
   * @param os A destination PrintStream.
   * @param logLevel The priority of log messages accepted by this stream
   * (messages with less significant priorities are dropped).
   * @param useTimestamps Denotes whether this LogStream will print timestamps
   * before each log message.
   * @param useTimestamps Denotes whether this LogStream will print a message
   * priority (log level) before each log message.
   */
  public LogStream
  (PrintStream os, int logLevel, boolean useTimestamps,
   boolean usePriorities)
    throws LogException
  {
    construct(os, logLevel, useTimestamps, usePriorities);
  }

  /**
   * @return true if this LogStream is currently open for output.
   */
  public boolean isOpen
  ()
  {
    return os != null;
  }

  /**
   * Close this LogStream's output stream.
   */
  public void close
  ()
  {
    lock();
    os = null;
    unlock();
  }

  /**
   * Set the priority of log messages accepted by this stream
   * (messages with less significant priorities are dropped).
   * @param logLevel The new priority.
   */
  public void setLogLevel
  (int logLevel)
    throws LogException
  {
    Log.checkPriority(logLevel);
    this.logLevel = logLevel;
  }

}
