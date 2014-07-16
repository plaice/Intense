// ****************************************************************************
//
// Test.java : Generic base for simple tests, with optional multithreading and
// memory-leak testing (mirrors subset of C++ test base from libintense-aep).
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


import intense.*;
import java.util.*;
import java.util.regex.*;


public abstract class Test
  implements Runnable {

  protected String execName;

  protected int nThreads;

  protected boolean repeating;

  public Test
  (String execName)
  {
    this.execName = execName;
    nThreads = 1;
    repeating = false;
  }

  String getExecName
  ()
  {
    return execName;
  }

  protected String getUsageString
  ()
  {
    return "\nusage:\n\n" + execName + " [-t<n threads>] " +
      "[-m (infinite loop mem leak test)]\n" +
      "\t[-h (help - you're looking at it...)]\n";
  }

  protected boolean permitMultiThreadedTest
  ()
  {
    return true;
  }

  protected void usage
  ()
  {
    System.err.println(getUsageString());
    System.exit(1);
  }

  protected void getOpts
  (String[] argv)
  {
    int i;
    Pattern tPattern = Pattern.compile("-t([0-9]+)");
    Matcher matcher = null;

    for (i = 0; i < argv.length; i++) {
      if (argv[i].equals("-m")) {
        repeating = true;
      } else if ((matcher = tPattern.matcher(argv[i])).find()) {
        String nThreadsString = matcher.group(1);
        try {
          nThreads = Integer.parseInt(nThreadsString);
          if ((nThreads > 1)&&(!permitMultiThreadedTest())) {
            System.err.println(
              "Only one (main) thread permitted (-t<nThreads> not supported)"
            );
            System.exit(1);
          }
        } catch (NumberFormatException e) {
          System.err.println(
            "Invalid threads-number expression \"" + nThreadsString + "\""
          );
          System.exit(1);
        }
      } else {
        usage();
      }
    }
  }

  public void run
  ()
  {
    if (repeating) {
      while (repeating) {
        testExceptionWrapper();
      }
    } else {
      testExceptionWrapper();
    }
  }

  private void testExceptionWrapper()
  {
    try {
      test();
    } catch (TestException e) {
      System.err.println(
        "\n" + getExecName() + " failed:\n\n" + e.getMessage() + "\n"
      );
      e.printStackTrace(System.err);
      System.err.print('\n');
      System.exit(1);
    } catch (Exception e) {
      System.err.println(
        "\n" + getExecName() + " generated an Exception:\n\n" +
        e.getMessage() + "\n"
      );
      e.printStackTrace(System.err);
      System.err.print('\n');
      System.exit(3);
    } catch (Throwable t) {
      System.err.println(
        "\n" + getExecName() + " generated a Throwable:\n"
      );
      t.printStackTrace(System.err);
      System.err.print('\n');
      System.exit(4);
    }
  }

  protected void testMain
  (String[] argv)
  {
    getOpts(argv);
    if (nThreads > 1) {
      // Reentrancy test...
      Thread[] threads = new Thread[nThreads];

      for (int i = 0; i < nThreads - 1; i++) {
        threads[i] = new Thread(this);
        threads[i].start();
      }
      // The main thread becomes the nth thread:
      run();
    } else {
      run();
    }
    System.out.println("PASS: " + execName);
  }

  protected abstract void test
  ()
    throws Exception;

}
