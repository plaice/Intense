// ****************************************************************************
//
// AEPStreamClient.java : AEP 2.0 intermediate abstract streamed client.
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


public abstract class AEPStreamClient
  extends AEPClient {

  protected BufferedInputStream inputStream;

  protected OutputStream outputStream;

  protected AEPStreamClient
  (InputStream is, OutputStream os, Log log, int logLevel,
   boolean errorTolerant, boolean useReceiverThread, int maxReceiveSize)
    throws AEPException
  {
    super(log, logLevel, errorTolerant, useReceiverThread, maxReceiveSize);
    this.inputStream = new BufferedInputStream(is);
    this.outputStream = os;
  }

  protected boolean blockForAvailableData
  ()
    throws IOException
  {
    inputStream.mark(1);
    if (inputStream.read() != -1) {
      inputStream.reset();
    }
    return false;
  }

  protected String getName
  ()
  {
    return "AEPStreamClient";
  }

  protected void closeConnection
  ()
    throws IOException
  {
    inputStream.close();
    outputStream.close();
  }

}
