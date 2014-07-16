// ****************************************************************************
//
// AEPSharedServer.java : AEP 2.0 shared server.
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


public class AEPSharedServer
  extends AEPServer {

  protected AEPSharedClient client;

  private LinkedList<Token> incoming;

  private Condition receptionCondition;

  public AEPSharedServer
  (AEther aether, Log log, int logLevel, boolean errorTolerant)
  {
    super(aether, log, logLevel, errorTolerant);
    client = null;
    incoming = new LinkedList<Token>();
    receptionCondition = receptionMutex.newCondition();
  }

  protected void send
  (AEPClient.Token token)
    throws AEPException
  {
    if (client == null) {
      throw new AEPException(
        "INTERNAL ERROR: NULL client in AEPSharedServer::send", Log.FATAL
      );
    }
    client.append((AEPClient.Token)token);
  }

  protected void append
  (Token token)
    throws AEPException
  {
    receptionMutex.lock();
    incoming.addLast(token);
    receptionCondition.signal();
    receptionMutex.unlock();
  }

  protected Token receive
  ()
    throws IOException, IntenseException, AEPException
  {
    if (incoming.size() == 0) {
      throw new AEPException(
        "INTERNAL ERROR: incoming queue empty in AEPSharedServer::receive",
        Log.FATAL
      );
    }
    return (Token)incoming.removeFirst();
  }

  protected boolean locksReception
  ()
  {
    return true;
  }

  protected boolean blockForAvailableData
  ()
    throws IOException
  {
    receptionMutex.lock();
    while (incoming.size() == 0) {
      receptionCondition.awaitUninterruptibly();
    }
    return true;
  }

  public String getName
  ()
  {
    return "AEPSharedServer";
  }

  protected void closeConnection
  ()
    throws IOException
  {
    client = null;
    receptionCondition.signal();
  }

}
