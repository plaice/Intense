// ****************************************************************************
//
// AEPXDRServer.java : AEP 2.0 XDR streamed server, compatible with the both
// the C++ and Java AEPXDRClient.
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


public class AEPXDRServer
  extends AEPStreamServer {

  protected DataInputStream is;

  protected DataOutputStream os;

  protected void construct
  ()
  {
    this.is = new DataInputStream(inputStream);
    this.os = new DataOutputStream(outputStream);
  }

  public AEPXDRServer
  (AEther aether, InputStream is, OutputStream os)
  {
    super(aether, is, os, null, Log.NOTICE, true);
    construct();
  }

  public AEPXDRServer
  (AEther aether, InputStream is, OutputStream os, Log log)
  {
    super(aether, is, os, log, Log.NOTICE, true);
    construct();
  }

  public AEPXDRServer
  (AEther aether, InputStream is, OutputStream os, Log log,
   int logLevel)
  {
    super(aether, is, os, log, logLevel, true);
    construct();
  }

  public AEPXDRServer
  (AEther aether, InputStream is, OutputStream os, Log log,
   int logLevel, boolean errorTolerant)
  {
    super(aether, is, os, log, logLevel, errorTolerant);
    construct();
  }

  protected void send
  (AEPClient.Token token)
    throws IOException, AEPException
  {
    AEPClient.Token.factorySerialise(token, os);
    os.flush();
    if ((log != null)&&(logLevel >= Log.DEBUG2)) {
      log.priority(Log.ERROR).add("AEPXDRServer.send: sent token: ");
      token.print(log, (logLevel > Log.DEBUG2) ? logLevel : Log.DEBUG);
      log.end();
    }
  }

  protected Token receive
  ()
    throws IOException, IntenseException, AEPException
  {
    return AEPServer.Token.factoryDeserialise(is);
  }

  public String getName
  ()
  {
    return "AEPXDRServer";
  }

  protected boolean locksReception
  ()
  {
    return true;
  }

}
