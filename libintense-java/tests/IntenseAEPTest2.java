// ****************************************************************************
//
// IntenseAEPTest2.java : A basic sanity test for AETP (text-mode) AEP 2.
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


public class IntenseAEPTest2
  extends IntenseAEPTest {

  public static final int logLevel = Log.ERROR;

  public IntenseAEPTest2
  ()
  {
    super("IntenseAEPTest2");
  }

  protected void test
  ()
    throws Exception
  {
    LogStream logStream = new LogStream(System.err, logLevel, true, true);
    Log log = new Log(logStream);
    AEPServer.AEther aether = new AEPServer.AEther(10, log, logLevel);

    aether.start();
    setLog(log);
    addPair(new AETPClientServerPair("t1", aether, log, logLevel));
    addPair(new AETPClientServerPair("t2", aether, log, logLevel));
    addPair(new AETPClientServerPair("t3", aether, log, logLevel));
    addPair(new AETPClientServerPair("t4", aether, log, logLevel));
    addPair(new AETPClientServerPair("t5", aether, log, logLevel));
    addPair(new AETPClientServerPair("t6", aether, log, logLevel));
    aepTest();
    deletePairs();
    aether.stop();
  }

  public static void main(String[] argv)
  {
    IntenseAEPTest2 test = new IntenseAEPTest2();
    test.testMain(argv);
  }

}
