// ****************************************************************************
//
// AEPXDRClient.java : AEP 2.0 XDR streamed client, compatible with the both
// the C++ and Java AEPXDRServer.
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


public class AEPXDRClient
  extends AEPStreamClient {

  protected DataInputStream is;

  protected DataOutputStream os;

  protected void construct
  ()
  {
    this.is = new DataInputStream(inputStream);
    this.os = new DataOutputStream(outputStream);
  }

  public AEPXDRClient
  (InputStream is, OutputStream os)
    throws AEPException
  {
    super(is, os, null, Log.NOTICE, true, false, 0);
    construct();
  }

  public AEPXDRClient
  (InputStream is, OutputStream os, Log log)
    throws AEPException
  {
    super(is, os, log, Log.NOTICE, true, false, 0);
    construct();
  }

  public AEPXDRClient
  (InputStream is, OutputStream os, Log log, int logLevel)
    throws AEPException
  {
    super(is, os, log, logLevel, true, false, 0);
    construct();
  }

  public AEPXDRClient
  (InputStream is, OutputStream os, Log log, int logLevel,
   boolean errorTolerant)
    throws AEPException
  {
    super(is, os, log, logLevel, errorTolerant, false, 0);
    construct();
  }

  public AEPXDRClient
  (InputStream is, OutputStream os, Log log, int logLevel,
   boolean errorTolerant, boolean useReceiverThread)
    throws AEPException
  {
    super(is, os, log, logLevel, errorTolerant, useReceiverThread, 0);
    construct();
  }

  public AEPXDRClient
  (InputStream is, OutputStream os, Log log, int logLevel,
   boolean errorTolerant, boolean useReceiverThread, int maxReceiveSize)
    throws AEPException
  {
    super(
      is, os, log, logLevel, errorTolerant, useReceiverThread, maxReceiveSize
    );
    construct();
  }

  protected void send
  (AEPServer.Token token)
    throws IOException, AEPException
  {
    AEPServer.Token.factorySerialise((AEPServer.Token)token, os);
    os.flush();
  }

  protected Token receive
  ()
    throws IOException, IntenseException, AEPException
  {
    return AEPClient.Token.factoryDeserialise(is);
  }

  protected String getName
  ()
  {
    return "AEPXDRClient";
  }

  protected boolean locksReception
  ()
  {
    return true;
  }

}
