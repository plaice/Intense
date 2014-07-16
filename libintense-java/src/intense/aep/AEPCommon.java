// ****************************************************************************
//
// AEPCommon.java : AEP 2.0 client and server base, for the abstract/core AEP
// implementation.
//
// Copyright 2001, 2002, 2004 Paul Swoboda.
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


package intense.aep;


import java.lang.*;
import java.util.*;
import java.util.concurrent.locks.*;
import java.io.*;
import intense.*;
import intense.log.*;


abstract class AEPCommon {

  public static final int AEP_DEFAULT_PORT = 6969;


  /**
   * A wrapper for participants that associates them with an AEPServer or
   * an AEPClient.
   */
  protected interface Participant
    extends intense.Participant {

    public abstract long getId
    ();

    public abstract void setId
    (long id);

    public abstract void setLog
    (Log log);

    public abstract Log getLog
    ();

    public abstract void setLogLevel
    (int newLogLevel)
      throws LogException;

    public abstract int getLogLevel
    ();

  }


  protected abstract static class Token {

    public abstract int getType
    ();

    /**
     * Outbound serialisation, compatable with the C++ libintense-serial
     * XDRBaseSerialiser.
     */
    public abstract void serialise
    (DataOutputStream out)
      throws IOException, AEPException;

    /**
     * Inbound serialisation, compatable with the C++ libintense-serial
     * XDRBaseSerialiser.
     */
    public abstract void deserialise
    (DataInputStream in)
      throws IOException, IntenseException, AEPException;

    public abstract boolean handle
    ()
      throws IOException, IntenseException, AEPException;

    public abstract void aetpPrint
    (PrintStream os)
      throws IOException, AEPException;

    public void  print
    (PrintStream os)
      throws IOException
    {
      print(os, Log.DEBUG);
    }

    public abstract void  print
    (PrintStream os, int logLevel)
      throws IOException;

    public void print
    (Log log)
      throws IOException
    {
      print(log, Log.DEBUG);
    }

    public void print
    (Log log, int logLevel)
      throws IOException
    {
      log.add(toString(logLevel));
    }

    public String toString
    ()
    {
      return toString(Log.DEBUG2);
    }

    public String toString
    (int logLevel)
    {
      try {
        ByteArrayOutputStream bytesOut = new ByteArrayOutputStream();
        PrintStream out = new PrintStream(bytesOut);

        print(out, logLevel);
        out.flush();
        bytesOut.flush();
        return bytesOut.toString();
      } catch (IOException ignored) {
        return null;
      }
    }

  }

  protected Map<Long, Participant> participants;

  protected long nextParticipantId;

  protected Log log;

  protected int logLevel;

  protected boolean errorTolerant;

  protected ReentrantLock mutex;

  protected Condition condition;

  protected ReentrantLock sendMutex;

  protected ReentrantLock receptionMutex;

  protected AEPCommon
  (Log log, int logLevel, boolean errorTolerant)
  {
    participants = new HashMap<Long, Participant>();
    nextParticipantId = 1;
    this.log = log;
    this.logLevel = logLevel;
    this.errorTolerant = errorTolerant;
    mutex = new ReentrantLock();
    condition = mutex.newCondition();
    sendMutex = new ReentrantLock();
    receptionMutex = new ReentrantLock();
  }

  protected long addParticipant
  (Participant participant)
  {
    Long newId = new Long(nextParticipantId++);

    participants.put(newId, participant);
    participant.setId(newId.longValue());
    return newId.longValue();
  }

  protected void addParticipantWithId
  (Participant participant, String location)
    throws AEPException
  {
    if (participants.containsKey(new Long(participant.getId()))) {
      throw new AEPException(
        "Participant with id " + participant.getId() + " already exists in " +
        location,
        Log.ERROR
      );
    }
    participants.put(new Long(participant.getId()), participant);
  }

  public Participant getParticipant
  (long participantId, String location)
    throws AEPException
  {
    Participant participant =
      (Participant)participants.get(new Long(participantId));

    if (participant == null) {
      throw new AEPException(
        "Participant with id " + participantId + " does not exist in " +
        location,
        Log.ERROR
      );
    }
    return participant;
  }

  public Participant getParticipant
  (long participantId)
    throws AEPException
  {
    return getParticipant(participantId, "AEPCommon::getParticipant");
  }

  protected Participant removeParticipant
  (Participant participant, String location)
    throws AEPException
  {
    Long participantId = new Long(participant.getId());
    Participant existingParticipant =
      (Participant)participants.get(participantId);

    if (existingParticipant == null) {
      throw new AEPException(
        "Participant with id " + participant.getId() + " does not exist in " +
        location,
        Log.ERROR
      );
    }
    participant.setId(-1);
    participants.remove(participantId);
    return participant;
  }

  protected void removeAllParticipants
  ()
  {
    participants.clear();
  }

  protected void sendLock
  ()
  {
    sendMutex.lock();
  }

  protected void sendUnlock
  ()
  {
    sendMutex.unlock();
  }

  protected void condWait
  (Condition condition)
  {
    condition.awaitUninterruptibly();
  }

  protected void condWait
  ()
  {
    condition.awaitUninterruptibly();
  }

  /*
  protected abstract void send
  (AEPCommon.Token token)
    throws IOException, AEPException;
  */

  protected void lock
  ()
  {
    mutex.lock();
  }

  protected void unlock
  ()
  {
    mutex.unlock();
  }

  public Condition newCondition
    ()
  {
    return mutex.newCondition();
  }

  protected void signal
  ()
  {
    condition.signal();
  }

  public void setLogLevel
  (int logLevel)
    throws LogException
  {
    Log.checkPriority(logLevel);
    lock();
    this.logLevel = logLevel;
    unlock();
  }

  protected static String modeTypeStrings[] = {
    "BINARY", "XDR", "TEXT"
  };

  public static final int BINARY = 0;

  public static final int XDR = 1;

  public static final int TEXT = 2;

  public static void checkMode
  (int mode)
    throws AEPException
  {
    if ((mode < BINARY)||(mode > TEXT)) {
      throw new AEPException("Invalid mode " + mode);
    }
  }

  public static String getModeTypeString
  (int mode)
    throws AEPException
  {
    checkMode(mode);
    return modeTypeStrings[mode];
  }

  public class URLParseTarget {

    public int mode;

    public String host;

    public int port;

    public CompoundDimension dimension;

    public URLParseTarget
    ()
    {
      mode = -1;
      host = "";
      port = 0;
      dimension = null;
    }

  }

  /**
   * Parse AEP URLs of the form aep://some.host.name:port/dim1:dim2:...
   * Don't know if this might be useful in AEPServer, one day - it's for
   * AEPClient.
   */
  public static void parseURL
  (String url, URLParseTarget target)
    throws IntenseException, AEPException
  {
    String product = url;
    String portString;
    String dimensionString;
    int n;
    String endPtr;

    if (product.substring(0, 6).equals("aep://")) {
      target.mode = BINARY;
      product = product.substring(6, -1);
    } else if (product.substring(0, 7).equals("aetp://")) {
      target.mode = TEXT;
      product = product.substring(7, -1);
    } else if (product.substring(0, 7).equals("aebp://")) {
      target.mode = BINARY;
      product = product.substring(7, -1);
    } else if (product.substring(0, 7).equals("aexp://")) {
      target.mode = XDR;
      product = product.substring(7, -1);
    } else {
      throw new AEPException(
        "AEP URLs must start with the prefix \"aep://\", \"aetp://\", " +
        "\"aebp://\", or \"aexp://\"."
      );
    }
    if ((n = product.indexOf('/')) != -1) {
      // The dimension is everything after the first /:
      if (product.substring(n + 1).length() > 0) {
        dimensionString = product.substring(n + 1);
      } else {
        dimensionString = "";
      }
      // Remove everything from the first '/':
      product = product.substring(0, n - 1);
    } else {
      dimensionString = "";
    }
    // Now look for a ':' and if it's there, divide the host and port:
    if ((n = product.indexOf(':')) != -1) {
      portString = product.substring(n + 1);
      if (portString.length() <= 0) {
        throw new AEPException(
          "Bad port string \"" + portString + "\" in AEP URL"
        );
      }
      try {
        target.port = Integer.parseInt(portString);
      } catch (NumberFormatException e) {
        throw new AEPException(
          "Invalid port string \"" + portString + "\" in AEP URL", e
        );
      }
      product = product.substring(0, n - 1);
    } else {
      target.port = AEP_DEFAULT_PORT;
    }
    if (product.length() <= 0) {
      throw new AEPException("bad host in AEP URL");
    }
    target.host = product;
    target.dimension.clear();
    target.dimension.parse(dimensionString);
  }

  /**
   * Construct an AEP URL, with a possibly-NULL dimension.
   */
  public static String constructURL
  (int mode, String host, int port, CompoundDimension dimension)
    throws LogException, AEPException
  {
    StringBuffer buffer = new StringBuffer();

    switch (mode) {
    case BINARY:
      buffer.append("aebp://");
      break;
    case XDR:
      buffer.append("aexp://");
      break;
    case TEXT:
      buffer.append("aetp://");
      break;
    }
    buffer.append(host + ":" + port);
    if (dimension != null) {
      buffer.append("/" + dimension.canonical());
    }
    return buffer.toString();
  }

  protected abstract boolean locksReception
  ();

}
