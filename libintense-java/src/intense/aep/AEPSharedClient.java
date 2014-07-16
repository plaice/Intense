// ****************************************************************************
//
// AEPSharedClient.java : AEP 2.0 shared client.
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


public class AEPSharedClient
  extends AEPClient {

  private AEPSharedServer server;

  private LinkedList<Token> incoming;

  private Condition receptionCondition;

  private void construct
  ()
  {
    server = null;
    incoming = new LinkedList<Token>();
    receptionCondition = receptionMutex.newCondition();
  }

  private void construct
  (AEPSharedServer server)
    throws AEPException
  {
    this.server = server;
    incoming = new LinkedList<Token>();
    receptionCondition = receptionMutex.newCondition();
    attach(server);
  }

  public AEPSharedClient
  ()
    throws AEPException
  {
    super(null, Log.NOTICE, true, false, 0);
    construct();
  }

  public AEPSharedClient
  (Log log)
    throws AEPException
  {
    super(log, Log.NOTICE, true, false, 0);
    construct();
  }

  public AEPSharedClient
  (Log log, int logLevel)
    throws AEPException
  {
    super(log, logLevel, true, false, 0);
    construct();
  }

  public AEPSharedClient
  (Log log, int logLevel, boolean errorTolerant)
    throws AEPException
  {
    super(log, logLevel, errorTolerant, false, 0);
    construct();
  }

  public AEPSharedClient
  (Log log, int logLevel, boolean errorTolerant, boolean useReceiverThread)
    throws AEPException
  {
    super(log, logLevel, errorTolerant, useReceiverThread, 0);
    construct();
  }

  public AEPSharedClient
  (Log log, int logLevel, boolean errorTolerant, boolean useReceiverThread,
   int maxReceiveSize)
    throws AEPException
  {
    super(log, logLevel, errorTolerant, useReceiverThread, maxReceiveSize);
    construct();
  }

  public AEPSharedClient
  (AEPSharedServer server)
    throws AEPException
  {
    super(null, Log.NOTICE, true, false, 0);
    construct(server);
  }

  public AEPSharedClient
  (AEPSharedServer server, Log log)
    throws AEPException
  {
    super(log, Log.NOTICE, true, false, 0);
    construct(server);
  }

  public AEPSharedClient
  (AEPSharedServer server, Log log, int logLevel)
    throws AEPException
  {
    super(log, logLevel, true, false, 0);
    construct(server);
  }

  public AEPSharedClient
  (AEPSharedServer server, Log log, int logLevel, boolean errorTolerant)
    throws AEPException
  {
    super(log, logLevel, errorTolerant, false, 0);
    construct(server);
  }

  public AEPSharedClient
  (AEPSharedServer server, Log log, int logLevel, boolean errorTolerant,
   boolean useReceiverThread)
    throws AEPException
  {
    super(log, logLevel, errorTolerant, useReceiverThread, 0);
    construct(server);
  }

  public AEPSharedClient
  (AEPSharedServer server, Log log, int logLevel, boolean errorTolerant,
   boolean useReceiverThread, int maxReceiveSize)
    throws AEPException
  {
    super(log, logLevel, errorTolerant, useReceiverThread, maxReceiveSize);
    construct(server);
  }

  public static AEPSharedClient connect
  (AEPSharedServer server)
    throws IOException, AEPException
  {
    return connect(server, null, Log.NOTICE, true, false, 0);
  }

  public static AEPSharedClient connect
  (AEPSharedServer server, Log log)
    throws IOException, AEPException
  {
    return connect(server, log, Log.NOTICE, true, false, 0);
  }

  public static AEPSharedClient connect
  (AEPSharedServer server, Log log, int logLevel)
    throws IOException, AEPException
  {
    return connect(server, log, logLevel, true, false, 0);
  }

  public static AEPSharedClient connect
  (AEPSharedServer server, Log log, int logLevel, boolean errorTolerant)
    throws IOException, AEPException
  {
    return connect(server, log, logLevel, errorTolerant, false, 0);
  }

  public static AEPSharedClient connect
  (AEPSharedServer server, Log log, int logLevel, boolean errorTolerant,
   boolean useReceiverThread)
    throws IOException, AEPException
  {
    return connect(server, log, logLevel, errorTolerant, useReceiverThread, 0);
  }

  public static AEPSharedClient connect
  (AEPSharedServer server, Log log, int logLevel, boolean errorTolerant,
   boolean useReceiverThread, int maxReceiveSize)
    throws IOException, AEPException
  {
    AEPSharedClient client = new AEPSharedClient(
      server, log, logLevel, errorTolerant, useReceiverThread, maxReceiveSize
    );

    client.start();
    return client;
  }

  protected void attach
  (AEPSharedServer server)
    throws AEPException
  {
    server.lock();
    try {
      if (server.client != null) {
        throw new AEPException(
          "Server already attached to client in AEPSharedClient::attach"
        );
      }
      server.client = this;
    } finally {
      server.unlock();
    }
  }

  protected void send
  (AEPServer.Token token)
    throws IOException, AEPException
  {
    if (server == null) {
      throw new AEPException(
        "INTERNAL ERROR: NULL server in AEPSharedClient::send", Log.FATAL
      );
    }
    server.append((AEPServer.Token)token);
  }

  protected void append
  (Token token)
  {
    receptionMutex.lock();
    incoming.addLast(token);
    receptionCondition.signal();
    receptionMutex.unlock();
  }

  protected boolean locksReception
  ()
  {
    return true;
  }

  protected Token receive
  ()
    throws IOException, IntenseException, AEPException
  {
    if (incoming.size() == 0) {
      throw new AEPException(
        "INTERNAL ERROR: incoming queue empty in AEPSharedClient::receive",
        Log.FATAL
      );
    }
    return (Token)incoming.removeFirst();
  }

  protected boolean blockForAvailableData
  ()
    throws IOException
  {
    receptionMutex.lock();
    while (incoming.size() == 0) {
      receptionCondition.awaitUninterruptibly();
      if (stopped) {
        return true;
      }
    }
    return true;
  }

  protected String getName
  ()
  {
    return "AEPSharedClient";
  }

  protected void closeConnection
  ()
    throws IOException
  {
    server = null;
    receptionMutex.lock();
    receptionCondition.signal();
    receptionMutex.unlock();
  }

}
