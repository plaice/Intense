// ****************************************************************************
//
// IntenseAEPTest.java : Base for tests of libintense-aep.
//
// Copyright 2004 Paul Swoboda.
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


import java.io.*;
import java.util.*;
import java.util.regex.*;
import intense.*;
import intense.log.*;
import intense.aep.*;


abstract class IntenseAEPTest
  extends Test {


  protected class TestParticipant
    extends AEPClient.Participant {

    protected String name;

    protected ByteArrayOutputStream outBytes;

    protected PrintStream out;

    protected Context localContext;

    public TestParticipant
    (String name, Log log, int logLevel)
    {
      super(log, logLevel);
      this.name = name;
      this.outBytes = new ByteArrayOutputStream();
      this.out = new PrintStream(outBytes);
      this.localContext = new Context();
    }

    TestParticipant
    (AEPClient client, String name, Log log, int logLevel)
    {
      super(client, log, logLevel);
      this.name = name;
      this.outBytes = new ByteArrayOutputStream();
      this.out = new PrintStream(outBytes);
      this.localContext = new Context();
    }

    public void assignNotify
    (Context newValue, CompoundDimension dim, intense.Origin origin)
      throws IntenseException
    {
      if (log != null) {
        log.priority(logLevel);
      }
      if (dim != null) {
        if (log != null) {
          log.add(name + ": " + dim.canonical() + ":" + newValue);
        }
        localContext.value(dim).assign(newValue);
      } else { 
        if (log != null) {
          log.add(name + ": " + newValue);
        }
        localContext.assign(newValue);
      }
      if (origin == null) {
        if (log != null) {
          log.add(" origin: void");
        }
      } else {
        if (log != null) {
          log.add(
            " origin: server sequence: " +
            ((AEPClient.NotifyToken)origin).getServerSequence()
          );
        }
      }
      if (log != null) {
        log.end();
      }
    }

    public void applyNotify
    (ContextOp o, CompoundDimension dim, intense.Origin origin)
      throws IntenseException
    {
      if (log != null) {
        log.priority(logLevel);
      }
      if (dim != null) {
        if (log != null) {
          log.add(name + ": " + dim.canonical() + ":" + o);
        }
        localContext.value(dim).apply(o);
      } else { 
        if (log != null) {
          log.add(name + ": " + o);
        }
        localContext.apply(o);
      }
      if (origin == null) {
        if (log != null) {
          log.add(" origin: void");
        }
      } else {
        if (log != null) {
          log.add(
            " origin: server sequence: " +
            ((AEPClient.NotifyToken)origin).getServerSequence()
          );
        }
      }
      if (log != null) {
        log.end();
      }
    }

    public void clearNotify
    (CompoundDimension dim, intense.Origin origin)
      throws IntenseException
    {
      if (log != null) {
        log.priority(logLevel);
      }
      if (dim != null) {
        if (log != null) {
          log.add(name + ": clear(" + dim.canonical() + ").");
        }
        localContext.value(dim).clear();
      } else {
        if (log != null) {
          log.add(name + ": clear.");
        }
        localContext.clear();
      }
      if (origin == null) {
        if (log != null) {
          log.add(" origin: void");
        }
      } else {
        if (log != null) {
          log.add(
            " origin: server sequence: " +
            ((AEPClient.NotifyToken)origin).getServerSequence()
          );
        }
      }
      if (log != null) {
        log.end();
      }
    }

    public void kickNotify
    (intense.Origin origin)
      throws IntenseException
    {
      if (log != null) {
        log.priority(logLevel);
        log.add(name + ": kicked.");
        if (origin == null) {
          log.add(" origin: void");
        } else {
          log.add(
            " origin: server sequence: " +
            ((AEPClient.NotifyToken)origin).getServerSequence()
          );
        }
        log.end();
      }
    }

    public void checkContext
    (int testId, String expectedCanonical)
      throws TestException
    {
      if (!localContext.canonical().equals(expectedCanonical)) {
        throw new TestException(
          testId,
          "IntenseAEPTest::TestParticipant::checkContext: " +
          "unexpected context:\n" + localContext.canonical() +
          "\nexpected: " + expectedCanonical +
          "\nbasecount: " + localContext.baseCount()
        );
      }
    }

    public String getName
    ()
    {
      return name;
    }

  }


  protected class ClientServerPair {

    protected String name;

    protected AEPClient client;

    protected AEPServer server;

    protected Log log;

    protected int logLevel;

    HashMap<Long, TestParticipant> participantMap;

    ClientServerPair
    (String name, Log log, int logLevel)
    {
      this.name = name;
      this.client = null;
      this.server = null;
      this.log = log;
      this.logLevel = logLevel;
      this.participantMap = new HashMap<Long, TestParticipant>();
    }

    void setClient
    (AEPClient client)
    {
      this.client = client;
    }

    void setServer
    (AEPServer server)
    {
      this.server = server;
    }

    public void stop
    ()
      throws InterruptedException, IOException, AEPException
    {
      client.disconnect();
      client.stop();
      server.stop();
    }

    public TestParticipant newParticipant
    (int id)
    {
      TestParticipant participant = new TestParticipant(
        client, "Participant " + id, log, Log.DEBUG
      );
      participantMap.put(new Long(id), participant);
      return participant;
    }

    TestParticipant get
    (long id)
    {
      return participantMap.get(new Long(id));
    }

  }


  protected class SharedClientServerPair
    extends ClientServerPair {

    public SharedClientServerPair
    (String name, AEPServer.AEther aether, Log log, int logLevel)
      throws InterruptedException, IOException, IntenseException, AEPException
    {
      super(name, log, logLevel);
      AEPSharedServer sharedServer = new AEPSharedServer(
        aether, log, logLevel, true
      );
      AEPSharedClient sharedClient =
        new AEPSharedClient(sharedServer, log, logLevel);

      setServer(sharedServer);
      setClient(sharedClient);
      sharedServer.start();
      sharedClient.start();
    }

  }


  protected class TeeOutputStream
    extends OutputStream
  {
    OutputStream tee = null, out = null;
  
    public TeeOutputStream
    (OutputStream chainedStream, OutputStream teeStream)
    {
      out = chainedStream;
  
      if (teeStream == null) {
        tee = System.out;
      } else {
        tee = teeStream;
      }
    }
      
    /**
     * Implementation for parent's abstract write method.  
     * This writes out the passed in character to the both,
     * the chained stream and "tee" stream.
     */
    public void write
    (int c)
      throws IOException
    {
      out.write(c);
      tee.write(c);
      tee.flush();
    }
  
    /**
     * Closes both, chained and tee, streams.
     */
    public void close
    ()
      throws IOException
    {
      flush();
      out.close();
      tee.close();
    }
  
    /**
     * Flushes chained stream; the tee stream is flushed 
     * each time a character is written to it.
     */
    public void flush
    ()
      throws IOException
    {
      out.flush();
    }
  
  }


  protected abstract class StreamClientServerPair
    extends ClientServerPair {

    protected PipedOutputStream serverOut;

    protected PipedInputStream serverIn;

    protected PipedOutputStream clientOut;

    protected PipedInputStream clientIn;

    public StreamClientServerPair
    (String name, Log log, int logLevel)
      throws IOException
    {
      super(name, log, logLevel);
      serverOut = new PipedOutputStream();
      clientIn = new PipedInputStream(serverOut);
      clientOut = new PipedOutputStream();
      serverIn = new PipedInputStream(clientOut);
    }

  }


  protected class XDRClientServerPair
    extends StreamClientServerPair {

    public XDRClientServerPair
    (String name, AEPServer.AEther aether, Log log, int logLevel)
      throws InterruptedException, IOException, IntenseException, AEPException
    {
      super(name, log, logLevel);

      AEPXDRServer xdrServer = new AEPXDRServer(
        aether, serverIn, serverOut, log, logLevel, true
      );
      AEPXDRClient xdrClient = new AEPXDRClient(
        clientIn, clientOut, log, logLevel
      );
      setServer(xdrServer);
      setClient(xdrClient);
      xdrServer.start();
      xdrClient.start();
    }

  }


  protected class AETPClientServerPair
    extends StreamClientServerPair {

    public AETPClientServerPair
    (String name, AEPServer.AEther aether, Log log, int logLevel)
      throws InterruptedException, IOException, IntenseException, AEPException
    {
      super(name, log, logLevel);

      AETPServer aetpServer = new AETPServer(
        aether, serverIn, serverOut, log, logLevel, true
      );
      AETPClient aetpClient = new AETPClient(
        clientIn, clientOut, log, logLevel
      );
      setServer(aetpServer);
      setClient(aetpClient);
      aetpServer.start();
      aetpClient.start();
    }

  };


  protected Log log;

  protected Vector<ClientServerPair> pairs;

  public IntenseAEPTest
  (String execName)
  {
    super(execName);
    this.log = null;
    this.pairs = new Vector<ClientServerPair>();
  }

  protected String getUsageString
  ()
  {
    return "\nusage:\n\n" + execName +
      "[-m (infinite loop mem leak test)]\n" +
      "\t[-h (help - you're looking at it...)]\n";
  }

  protected boolean permitMultiThreadedTest
  ()
  {
    return false;
  }

  private ClientServerPair getPair
  (int i)
  {
    return (ClientServerPair)pairs.get(i);
  }

  protected void addPair
  (ClientServerPair pair)
  {
    pairs.add(pair);
  }

  protected void setLog
  (Log log)
  {
    this.log = log;
  }

  protected void deletePairs
  ()
    throws InterruptedException, IOException, AEPException
  {
    Iterator itr = pairs.iterator();

    while (itr.hasNext()) {
      ((ClientServerPair)itr.next()).stop();
    }
    pairs.clear();
  }

  protected void aepTest
  ()
    throws InterruptedException, IOException, IntenseException, AEPException,
           TestException
  {
    int i;
    int j;
    int flags =
      //    AEPServer.preFenceBit|
      //    AEPServer.postFenceBit|
      AEPServer.notifySelfBit|
      AEPServer.notifyClientBit;
    CompoundDimension dimension = new CompoundDimension();
    long serverSequence;

    if (log != null) {
      log.add("Starting aepTest...").end();
    }
    for (i = 0; i < pairs.size(); i++) {
      for (j = 1; j <= 10; j++) {
        TestParticipant participant = getPair(i).newParticipant(j);

        if (log != null) {
          log.add("Created new participant " + participant.getName()).end();
        }
      }
      getPair(i).get(1).join(true);
      getPair(i).get(1).leave();
      // participant 1 joined at root:
      getPair(i).get(1).join(true);
      getPair(i).get(2).join(true);
      // participant 2 joined at root:
      getPair(i).get(2).join(true);
      getPair(i).get(3).join(true, "bogus");
      // participant 3 joined at reactor:
      getPair(i).get(3).join(true, "reactor");
      // participant 4 joined at reactor:core:
      getPair(i).get(4).join(true, "reactor:core");
      // participant 5 joined at reactor:core:temp:
      getPair(i).get(5).join(true, "reactor:core:temp");
      // participant 6 joined at reactor:core:temp:
      getPair(i).get(6).join(true, "reactor:core:temp");
      // participant 7 joined at reactor:core:pressure:
      getPair(i).get(7).join(true, "reactor:core:pressure");
      // participant 8 joined at reactor:output:power:
      getPair(i).get(8).join(true, "reactor:output:power");
      // participant 9 joined at some:bogus:dimension:
      getPair(i).get(9).join(true, "some:bogus:dimension");
      // participant 10 joined at someother:bogus:dimension:
      getPair(i).get(10).join(true, "someother:bogus:dimension");
    }
    for (i = 0; i < pairs.size(); i++) {
      serverSequence = getPair(i).get(1).apply(
        "[reactor:core:[--+temp:[10+--]]]",
        (CompoundDimension)null, flags
      );
      getPair(i).get(1).synch();
      getPair(i).get(1).checkContext(100 + i, "<reactor:<core:<temp:<10.0>>>>");
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(200 + i, "<reactor:<core:<temp:<10.0>>>>");
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(300 + i, "<core:<temp:<10.0>>>");
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(400 + i, "<temp:<10.0>>");
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(500 + i, "<10.0>");
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(600 + i, "<10.0>");
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(700 + i, "<>");
      getPair(i).get(8).checkContext(800 + i, "<>");
      getPair(i).get(9).checkContext(900 + i, "<>");
      getPair(i).get(10).checkContext(1000 + i, "<>");
      // **********************************************************************
      serverSequence = getPair(i).get(1).apply(
        "[core:temp:20]", "reactor", flags
      );
      getPair(i).get(1).synch();
      getPair(i).get(1).checkContext(1100 + i, "<reactor:<core:<temp:<20.0>>>>");
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(1200 + i, "<reactor:<core:<temp:<20.0>>>>");
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(1300 + i, "<core:<temp:<20.0>>>");
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(1400 + i, "<temp:<20.0>>");
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(1500 + i, "<20.0>");
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(1600 + i, "<20.0>");
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(1700 + i, "<>");
      getPair(i).get(8).checkContext(1800 + i, "<>");
      getPair(i).get(9).checkContext(1900 + i, "<>");
      getPair(i).get(10).checkContext(2000 + i, "<>");
      // **********************************************************************
      serverSequence = getPair(i).get(1).apply(
        "[30]", "reactor:core:temp", flags
      );
      getPair(i).get(1).synch();
      getPair(i).get(1).checkContext(2100 + i, "<reactor:<core:<temp:<30.0>>>>");
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(2200 + i, "<reactor:<core:<temp:<30.0>>>>");
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(2300 + i, "<core:<temp:<30.0>>>");
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(2400 + i, "<temp:<30.0>>");
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(2500 + i, "<30.0>");
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(2600 + i, "<30.0>");
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(2700 + i, "<>");
      getPair(i).get(8).checkContext(2800 + i, "<>");
      getPair(i).get(9).checkContext(2900 + i, "<>");
      getPair(i).get(10).checkContext(3000 + i, "<>");
      // **********************************************************************
      serverSequence = getPair(i).get(5).apply(
        "[40]", (CompoundDimension)null, flags
      );
      getPair(i).get(5).synch();
      getPair(i).get(1).synch(serverSequence);
      getPair(i).get(1).checkContext(3100 + i, "<reactor:<core:<temp:<40.0>>>>");
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(3200 + i, "<reactor:<core:<temp:<40.0>>>>");
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(3300 + i, "<core:<temp:<40.0>>>");
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(3400 + i, "<temp:<40.0>>");
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(3500 + i, "<40.0>");
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(3600 + i, "<40.0>");
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(3700 + i, "<>");
      getPair(i).get(8).checkContext(3800 + i, "<>");
      getPair(i).get(9).checkContext(3900 + i, "<>");
      getPair(i).get(10).checkContext(4000 + i, "<>");
      // **********************************************************************
      serverSequence = getPair(i).get(7).apply(
        "[50]", (CompoundDimension)null, flags
      );
      getPair(i).get(7).synch();
      getPair(i).get(1).synch(serverSequence);
      getPair(i).get(1).checkContext(
        4100 + i, "<reactor:<core:<pressure:<50.0>+temp:<40.0>>>>"
      );
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(
        4200 + i, "<reactor:<core:<pressure:<50.0>+temp:<40.0>>>>"
      );
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(
        4300 + i, "<core:<pressure:<50.0>+temp:<40.0>>>"
      );
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(
        4400 + i, "<pressure:<50.0>+temp:<40.0>>"
      );
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(
        4500 + i, "<40.0>"
      );
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(
        4600 + i, "<40.0>"
      );
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(
        4700 + i, "<50.0>"
      );
      getPair(i).get(8).checkContext(
        4800 + i, "<>"
      );
      getPair(i).get(9).checkContext(
        4900 + i, "<>"
      );
      getPair(i).get(10).checkContext(
        5000 + i, "<>"
      );
      // **********************************************************************
      serverSequence = getPair(i).get(1).apply(
        "[reactor:core:temp:[60+level:\"CRITICAL\"]+reactor:core:pressure:70]",
        (CompoundDimension)null, flags
      );
      getPair(i).get(1).synch();
      getPair(i).get(1).checkContext(
        5100 + i,
        "<reactor:<core:<pressure:<70.0>+temp:<60.0+level:<\"CRITICAL\">>>>>"
      );
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(
        5200 + i,
        "<reactor:<core:<pressure:<70.0>+temp:<60.0+level:<\"CRITICAL\">>>>>"
      );
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(
        5300 + i, "<core:<pressure:<70.0>+temp:<60.0+level:<\"CRITICAL\">>>>"
      );
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(
        5400 + i, "<pressure:<70.0>+temp:<60.0+level:<\"CRITICAL\">>>"
      );
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(
        5500 + i, "<60.0+level:<\"CRITICAL\">>"
      );
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(
        5600 + i, "<60.0+level:<\"CRITICAL\">>"
      );
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(
        5700 + i, "<70.0>"
      );
      getPair(i).get(8).checkContext(
        5800 + i, "<>"
      );
      getPair(i).get(9).checkContext(
        5900 + i, "<>"
      );
      getPair(i).get(10).checkContext(
        6000 + i, "<>"
      );
    }
    for (i = 0; i < pairs.size(); i++) {
      serverSequence = getPair(i).get(1).assign(
        "<reactor:core:temp:10>", (CompoundDimension)null, flags
      );
      getPair(i).get(1).synch();
      getPair(i).get(1).checkContext(7100 + i, "<reactor:<core:<temp:<10.0>>>>");
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(7200 + i, "<reactor:<core:<temp:<10.0>>>>");
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(7300 + i, "<core:<temp:<10.0>>>");
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(7400 + i, "<temp:<10.0>>");
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(7500 + i, "<10.0>");
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(7600 + i, "<10.0>");
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(7700 + i, "<>");
      getPair(i).get(8).checkContext(7800 + i, "<>");
      getPair(i).get(9).checkContext(7900 + i, "<>");
      getPair(i).get(10).checkContext(8000 + i, "<>");
      // **********************************************************************
      serverSequence = getPair(i).get(1).assign(
        "<core:temp:20>", "reactor", flags
      );
      getPair(i).get(1).synch();
      getPair(i).get(1).checkContext(8100 + i, "<reactor:<core:<temp:<20.0>>>>");
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(8200 + i, "<reactor:<core:<temp:<20.0>>>>");
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(8300 + i, "<core:<temp:<20.0>>>");
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(8400 + i, "<temp:<20.0>>");
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(8500 + i, "<20.0>");
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(8600 + i, "<20.0>");
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(8700 + i, "<>");
      getPair(i).get(8).checkContext(8800 + i, "<>");
      getPair(i).get(9).checkContext(8900 + i, "<>");
      getPair(i).get(10).checkContext(9000 + i, "<>");
      // **********************************************************************
      serverSequence = getPair(i).get(1).assign(
        "<30.0>", "reactor:core:temp", flags
      );
      getPair(i).get(1).synch();
      getPair(i).get(1).checkContext(
        10100 + i, "<reactor:<core:<temp:<30.0>>>>"
      );
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(
        10200 + i, "<reactor:<core:<temp:<30.0>>>>"
      );
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(10300 + i, "<core:<temp:<30.0>>>");
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(10400 + i, "<temp:<30.0>>");
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(10500 + i, "<30.0>");
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(10600 + i, "<30.0>");
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(10700 + i, "<>");
      getPair(i).get(8).checkContext(10800 + i, "<>");
      getPair(i).get(9).checkContext(10900 + i, "<>");
      getPair(i).get(10).checkContext(11000 + i, "<>");
      // **********************************************************************
      serverSequence = getPair(i).get(5).assign(
        "<40.0>", (CompoundDimension)null, flags
      );
      getPair(i).get(5).synch();
      getPair(i).get(1).synch(serverSequence);
      getPair(i).get(1).checkContext(
        11100 + i, "<reactor:<core:<temp:<40.0>>>>"
      );
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(
        11200 + i, "<reactor:<core:<temp:<40.0>>>>"
      );
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(11300 + i, "<core:<temp:<40.0>>>");
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(11400 + i, "<temp:<40.0>>");
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(11500 + i, "<40.0>");
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(11600 + i, "<40.0>");
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(11700 + i, "<>");
      getPair(i).get(8).checkContext(11800 + i, "<>");
      getPair(i).get(9).checkContext(11900 + i, "<>");
      getPair(i).get(10).checkContext(12000 + i, "<>");
      // **********************************************************************
      serverSequence = getPair(i).get(7).assign(
        "<50.0>", (CompoundDimension)null, flags
      );
      getPair(i).get(7).synch();
      getPair(i).get(1).synch(serverSequence);
      getPair(i).get(1).checkContext(
        12100 + i, "<reactor:<core:<pressure:<50.0>+temp:<40.0>>>>"
      );
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(
        12200 + i, "<reactor:<core:<pressure:<50.0>+temp:<40.0>>>>"
      );
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(
        12300 + i, "<core:<pressure:<50.0>+temp:<40.0>>>"
      );
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(
        12400 + i, "<pressure:<50.0>+temp:<40.0>>"
      );
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(
        12500 + i, "<40.0>"
      );
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(
        12600 + i, "<40.0>"
      );
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(
        12700 + i, "<50.0>"
      );
      getPair(i).get(8).checkContext(
        12800 + i, "<>"
      );
      getPair(i).get(9).checkContext(
        12900 + i, "<>"
      );
      getPair(i).get(10).checkContext(
        13000 + i, "<>"
      );
      // **********************************************************************
      serverSequence = getPair(i).get(1).assign(
        "<reactor:core:temp:<60.0+level:\"CRITICAL\">+reactor:core:pressure:70.0>",
        (CompoundDimension)null, flags
      );
      getPair(i).get(1).synch();
      getPair(i).get(1).checkContext(
        13100 + i,
        "<reactor:<core:<pressure:<70.0>+temp:<60.0+level:<\"CRITICAL\">>>>>"
      );
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(
        13200 + i,
        "<reactor:<core:<pressure:<70.0>+temp:<60.0+level:<\"CRITICAL\">>>>>"
      );
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(
        13300 + i, "<core:<pressure:<70.0>+temp:<60.0+level:<\"CRITICAL\">>>>"
      );
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(
        13400 + i, "<pressure:<70.0>+temp:<60.0+level:<\"CRITICAL\">>>"
      );
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(
        13500 + i, "<60.0+level:<\"CRITICAL\">>"
      );
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(
        13600 + i, "<60.0+level:<\"CRITICAL\">>"
      );
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(
        13700 + i, "<70.0>"
      );
      getPair(i).get(8).checkContext(
        13800 + i, "<>"
      );
      getPair(i).get(9).checkContext(
        13900 + i, "<>"
      );
      getPair(i).get(10).checkContext(
        14000 + i, "<>"
      );
    }
    for (i = 0; i < pairs.size(); i++) {
      serverSequence = getPair(i).get(1).clear("reactor:core", flags);
      getPair(i).get(1).synch();
      getPair(i).get(1).checkContext(14100 + i, "<>");
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(14200 + i, "<>");
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(14300 + i, "<>");
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(14400 + i, "<>");
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(14500 + i, "<>");
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(14600 + i, "<>");
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(14700 + i, "<>");
      getPair(i).get(8).checkContext(14800 + i, "<>");
      getPair(i).get(9).checkContext(14900 + i, "<>");
      getPair(i).get(10).checkContext(15000 + i, "<>");
      serverSequence = getPair(i).get(1).assign(
        "<reactor:core:temp:<80.0>+reactor:core:pressure:90>",
        (CompoundDimension)null, flags
      );
      getPair(i).get(1).synch();
      getPair(i).get(1).checkContext(
        15100 + i,
        "<reactor:<core:<pressure:<90.0>+temp:<80.0>>>>"
      );
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(
        15200 + i,
        "<reactor:<core:<pressure:<90.0>+temp:<80.0>>>>"
      );
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(
        15300 + i, "<core:<pressure:<90.0>+temp:<80.0>>>"
      );
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(
        15400 + i, "<pressure:<90.0>+temp:<80.0>>"
      );
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(
        15500 + i, "<80.0>"
      );
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(
        15600 + i, "<80.0>"
      );
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(
        15700 + i, "<90.0>"
      );
      getPair(i).get(8).checkContext(
        15800 + i, "<>"
      );
      getPair(i).get(9).checkContext(
        15900 + i, "<>"
      );
      getPair(i).get(10).checkContext(
        16000 + i, "<>"
      );
      // **********************************************************************
      serverSequence = getPair(i).get(5).clear(
        (CompoundDimension)null, flags
      );
      getPair(i).get(5).synch();
      getPair(i).get(1).synch(serverSequence);
      getPair(i).get(1).checkContext(
        16100 + i,
        "<reactor:<core:<pressure:<90.0>>>>"
      );
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(
        16200 + i,
        "<reactor:<core:<pressure:<90.0>>>>"
      );
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(
        16300 + i, "<core:<pressure:<90.0>>>"
      );
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(
        16400 + i, "<pressure:<90.0>>"
      );
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(
        16500 + i, "<>"
      );
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(
        16600 + i, "<>"
      );
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(
        16700 + i, "<90.0>"
      );
      getPair(i).get(8).checkContext(
        16800 + i, "<>"
      );
      getPair(i).get(9).checkContext(
        16900 + i, "<>"
      );
      getPair(i).get(10).checkContext(
        17000 + i, "<>"
      );
      // **********************************************************************
      // Now a (non-deterministic / race-dependent) test of accumulation:
      // **********************************************************************
      serverSequence = getPair(i).get(1).assign(
        "<reactor:core:temp:<60.0>+reactor:core:pressure:70.0>",
        (CompoundDimension)null, flags
      );
      dimension.parse("reactor:core");
      serverSequence = getPair(i).get(1).apply(
        "[temp:1+pressure:11]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:2+pressure:12]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:3+pressure:13]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:4+pressure:14]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:5+pressure:15]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:6+pressure:16]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:7+pressure:17]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:8+pressure:18]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:9+pressure:19]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:10+pressure:20]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:11+pressure:21]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:12+pressure:22]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:13+pressure:23]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:14+pressure:24]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:15+pressure:25]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:16+pressure:26]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:17+pressure:27]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:18+pressure:28]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:19+pressure:29]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:20+pressure:30]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:21+pressure:31]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:22+pressure:32]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:23+pressure:33]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:24+pressure:34]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:25+pressure:35]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:26+pressure:36]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:27+pressure:37]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:28+pressure:38]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:29+pressure:39]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:30+pressure:40]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:31+pressure:41]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:32+pressure:42]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:33+pressure:43]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:34+pressure:44]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:35+pressure:45]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:36+pressure:46]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:37+pressure:47]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:38+pressure:48]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:39+pressure:49]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:40+pressure:50]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:41+pressure:51]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:42+pressure:52]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:43+pressure:53]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:44+pressure:54]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:45+pressure:55]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:46+pressure:56]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:47+pressure:57]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:48+pressure:58]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:49+pressure:59]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:50+pressure:60]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:51+pressure:61]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:52+pressure:62]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:53+pressure:63]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:54+pressure:64]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:55+pressure:65]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:56+pressure:66]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:57+pressure:67]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:58+pressure:68]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:59+pressure:69]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:60+pressure:70]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:61+pressure:71]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:62+pressure:72]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:63+pressure:73]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:64+pressure:74]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:65+pressure:75]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:66+pressure:76]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:67+pressure:77]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:68+pressure:78]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:69+pressure:79]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:70+pressure:80]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:71+pressure:81]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:72+pressure:82]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:73+pressure:83]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:74+pressure:84]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:75+pressure:85]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:76+pressure:86]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:77+pressure:87]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:78+pressure:88]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:79+pressure:89]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:80+pressure:90]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:81+pressure:91]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:82+pressure:92]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:83+pressure:93]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:84+pressure:94]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:85+pressure:95]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:86+pressure:96]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:87+pressure:97]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:88+pressure:98]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:89+pressure:99]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:90+pressure:100]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:91+pressure:101]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:92+pressure:102]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:93+pressure:103]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:94+pressure:104]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:95+pressure:105]", dimension, flags
      );
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).clear(dimension, flags);
      serverSequence = getPair(i).get(1).apply(
        "[temp:96+pressure:106]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:97+pressure:107]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:98+pressure:108]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:99+pressure:109]", dimension, flags
      );
      serverSequence = getPair(i).get(1).apply(
        "[temp:100+pressure:110]", dimension, flags
      );
  
      getPair(i).get(1).synch();
      getPair(i).get(1).checkContext(
        18100 + i,
        "<reactor:<core:<pressure:<110.0>+temp:<100.0>>>>"
      );
      getPair(i).get(2).synch(serverSequence);
      getPair(i).get(2).checkContext(
        18200 + i,
        "<reactor:<core:<pressure:<110.0>+temp:<100.0>>>>"
      );
      getPair(i).get(3).synch(serverSequence);
      getPair(i).get(3).checkContext(
        18300 + i, "<core:<pressure:<110.0>+temp:<100.0>>>"
      );
      getPair(i).get(4).synch(serverSequence);
      getPair(i).get(4).checkContext(
        18400 + i, "<pressure:<110.0>+temp:<100.0>>"
      );
      getPair(i).get(5).synch(serverSequence);
      getPair(i).get(5).checkContext(
        18500 + i, "<100.0>"
      );
      getPair(i).get(6).synch(serverSequence);
      getPair(i).get(6).checkContext(
        18600 + i, "<100.0>"
      );
      getPair(i).get(7).synch(serverSequence);
      getPair(i).get(7).checkContext(
        181100 + i, "<110.0>"
      );
      getPair(i).get(8).checkContext(
        18800 + i, "<>"
      );
      getPair(i).get(9).checkContext(
        18900 + i, "<>"
      );
      getPair(i).get(10).checkContext(
        19000 + i, "<>"
      );
  
    }
    for (i = 0; i < pairs.size(); i++) {
      // Leave() the even-numbered participants only, before the client (and all
      // participants with it) gets deleted in ~ClientServerPair:
      for (j = 1; j <= 5; j++) {
        getPair(i).get(j*2).leave();
      }
    }
    if (log != null) {
      log.add("Finished aepTest.").end();
    }
  }

}
