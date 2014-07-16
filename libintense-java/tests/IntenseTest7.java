// ****************************************************************************
//
// IntenseTest7.java : A test of Context and ContextOp serialisation.
//
// The following combinations are useful for memory leak testing, between C++
// and C++ or between C++ and Java:
//
// IntenseTest7 -m -o -b | IntenseTest7 -i -m -v -b
// IntenseTest7 -m -o -x | IntenseTest7 -i -m -v -x
// java IntenseTest7 -m -o | IntenseTest7 -i -m -v -x
// IntenseTest7 -m -o -x | java IntenseTest7 -i -m -v
//
// (Omit the -v to get rid of the deserialised context / context op output.)
//
// Copyright 2003 Paul Swoboda.
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
import java.io.*;
import java.util.*;
import java.util.regex.*;


public class IntenseTest7
  extends IntenseTest {

  // contextStrings and contextOpStrings, below, are pretty much just a random
  // cut-and-paste job with replacements, from parsed canonical forms from
  // previous unit tests.  They are the same in IntenseTest7.java and
  // IntenseTest7.cpp.

  private static String[] contextStrings = {
    "<>",
    "<\"base\">",
    "<\"newbase\">",
    "<\"newbase\">",
    "<\"base\"+dim1:<\"base2\">>",
    "<\"base\"+dim1:<\"base2\">>",
    "<\"base\"+dim1:<\"newbase2\">>",
    "<\"base\"+dim1:<\"newbase2\">>",
    "<d1:<\"d1val\">+d2:<\"d2val\"+d21:<~>+d22:<^+d221:<12345.6>>>>",
    "<d1:<\"d2val\"+d21:<~>+d22:<^+d221:<12345.6>>>>",
    "<d1:<\"d1val\"+d21:<~>+d22:<^+d221:<12345.6>>>>",
    "<d1:<\"d1val\"+d21:<~>+d22:<#494e54454e53494f4e414c2052554c45532100+d221:<12345.6>>>>",
    "<d1:<~>+d2:<^+d21:<d211:<~>+d212:<^>>>>",
    "<d1:<1.2345E8>+d2:<1.2345E12+d21:<d211:<-1.2345E-6>+d212:<\"123\">>>>",
    "<1:<\"1val\"+12:<\"12val\"+121:<\"121val\">>>>",
    "<2:<\"2val\">+\u0091\u0092\u0091\u0092\u0091\u0092\u0091\u0092:<\"1val\">>",
    "<2:<\"2val\">+abc123_\u0091\u0092\u0091\u0092\u0091\u0092\u0091\u0092_def456:<\"1val\">>",
    "<2:<\"2val\">+\"abc 123 \u0091\u0092\u0091\u0092\u0091\u0092\u0091\u0092 def 456\":<\"1val\">>",
    "<dim1:<\"\u0091\u0092\u0091\u0092\u0091\u0092\u0091\u0092\">>",
    "<dim1:<\"abc123_\u0091\u0092\u0091\u0092\u0091\u0092\u0091\u0092_def456\">>",
    "<dim1:<\"abc 123 \u0091\u0092\u0091\u0092\u0091\u0092\u0091\u0092 def 456\">>",
    "<d1:<\"_\\a\">+d2:<\"_\\b\">+d3:<\"_\\t\">+d4:<\"_\\n\">+d5:<\"_\\v\">+d6:<\"_\\f\">+d7:<\"_\\r\">>",
    "<da:<\"_\\\\\">+db:<\"_\\\"\">+dc:<\"_#\">+dd:<\"_ \">+de:<\"_~\">+df:<\"_^\">+dg:<\"_-\">+dh:<\"_<\">+di:<\"_>\">+dj:<\"_[\">+dk:<\"_]\">+dl:<\"_+\">+dm:<\"_:\">>",
    "<\"d1\\a\":<1.0>+\"d2\\b\":<2.0>+\"d3\\t\":<3.0>+\"d4\\n\":<4.0>+\"d5\\v\":<5.0>+\"d6\\f\":<6.0>+\"d7\\r\":<7.0>>",
    "<\"da\\\\\":<1.0>+\"db\\\"\":<2.0>+\"dc#\":<3.0>+\"dd \":<4.0>+\"de~\":<5.0>+\"df^\":<6.0>+\"dg-\":<7.0>+\"dh<\":<8.0>+\"di>\":<9.0>+\"dj[\":<10.0>+\"dk]\":<11.0>+\"dl+\":<12.0>+\"dm:\":<13.0>>"
  };

  private static String[] contextOpStrings = {
    "[]",
    "[-]",
    "[---]",
    "[--+dim:[]]",
    "[---+dim:[]]",
    "[--+dim:[-]]",
    "[---+dim:[-]]",
    "[--+dim:[\"base\"]]",
    "[---+dim:[\"base\"]]",
    "[--+dim:[#494e54454e53494f4e414c2052554c45532100]]",
    "[---+dim:[#494e54454e53494f4e414c2052554c45532100]]",
    "[--+dim:[~]]",
    "[---+dim:[~]]",
    "[--+dim:[^]]",
    "[---+dim:[^]]",
    "[--+\"base\"]",
    "[--+\"base value\\t\\t\\n\\r\\n\\t\\tthis is a test\"]",
    "[--+#494e54454e53494f4e414c2052554c45532100]",
    "[--+\"base\"+dim1:[--+\"base2\"]]",
    "[--+\"base\"+dim1:[--+\"newbase2\"]]",
    "[---+d1:[--+\"d1val\"]+d2:[--+\"d2val\"+d21:[--+~]+d22:[--+^+d221:[--+12345.6]]]]",
    "[---+d1:[--+\"d2val\"+d21:[--+~]+d22:[--+^+d221:[--+12345.6]]]]",
    "[---+d1:[--+\"d1val\"+d21:[--+~]+d22:[--+^+d221:[--+12345.6]]]]",
    "[---+d1:[--+\"d1val\"+d21:[--+~]+d22:[--+#494e54454e53494f4e414c2052554c45532100+d221:[--+12345.6]]]]",
    "[---+d1:[\"d1val\"]]",
    "[---+d1:[--+\"d1newval\"+d21:[--+~]+d22:[#494e54454e53494f4e414c2052554c45532100+d221:[--+12345.6]]]]",
    "[--+\"base\"+dim1:[--+\"base2\"]]",
    "[---+d1:[--+\"d1val\"]+d2:[--+\"d2val\"+d21:[--+~]+d22:[--+^+d221:[--+12345.6]]]]",
    "[---+d1:[--+\"d2val\"+d21:[--+~]+d22:[--+^+d221:[--+12345.6]]]]",
    "[---+d1:[--+\"d1val\"]]",
    "[---+d1:[--+\"d1val\"+d22:[--+#494e54454e53494f4e414c2052554c45532100]]]",
    "[---+d1:[---+d12:[--+\"d12newval\"]]]",
    "[--+d1:[--+\"d1val\"+d12:[\"d12val\"+d121:[--+\"d121val\"]]]]",
    "[--+1:[--+\"1val\"+12:[\"12val\"+121:[--+\"121val\"]]]]"
  };

  private static String[] compoundDimensionStrings = {
    "a",
    "a:b",
    "a:b:c",
    "abc:def",
    "abc:def:ghi",
    "123",
    "123:456",
    "123:456:789",
    "abc:123",
    "123:abc"
  };

  private boolean unitTestMode = true;

  private boolean inputMode = false;

  DataInputStream inputStream = null;

  private boolean outputMode = false;

  DataOutputStream outputStream = null;

  private boolean verboseFlag = false;

  private Context[] contexts;

  private ContextOp[] contextOps;

  private CompoundDimension[] compoundDimensions;

  public IntenseTest7
  ()
  {
    super("IntenseTest7");
    int i;
    try {
      contexts = new Context[contextStrings.length];
      for (i = 0; i < contextStrings.length; i++) {
        contexts[i] = new Context();
        contexts[i].parse(contextStrings[i]);
      }
      contextOps = new ContextOp[contextOpStrings.length];
      for (i = 0; i < contextOpStrings.length; i++) {
        contextOps[i] = new ContextOp();
        contextOps[i].parse(contextOpStrings[i]);
      }
      compoundDimensions =
        new CompoundDimension[compoundDimensionStrings.length];
      for (i = 0; i < compoundDimensionStrings.length; i++) {
        compoundDimensions[i] = new CompoundDimension();
        compoundDimensions[i].parse(compoundDimensionStrings[i]);
      }
    } catch (IntenseException e) {
      System.err.println("Error during intialisation: " + e.getMessage());
      System.exit(1);
    }
  }

  protected void test
  ()
    throws Exception
  {
    if (unitTestMode) {
      ByteArrayOutputStream outArray = new ByteArrayOutputStream();
      DataOutputStream out = new DataOutputStream(outArray);
      writeAll(out);
      ByteArrayInputStream inArray =
        new ByteArrayInputStream(outArray.toByteArray());
      DataInputStream in = new DataInputStream(inArray);
      readAll(in);
    } else if (inputMode) {
      readAll(inputStream);
    } else if (outputMode) {
      writeAll(outputStream);
    }
  }

  protected void writeAll
  (DataOutputStream out)
    throws IntenseException, IOException
  {
    int i;

    for (i = 0; i < contextStrings.length; i++) {
      contexts[i].serialise(out, null);
      if (verboseFlag) {
        System.err.println("Serialised Context: " + contexts[i].canonical());
      }
    }
    for (i = 0; i < contextOpStrings.length; i++) {
      contextOps[i].serialise(out, null);
      if (verboseFlag) {
        System.err.println("Serialised ContextOp: " + contextOps[i].canonical());
      }
    }
    for (i = 0; i < compoundDimensionStrings.length; i++) {
      compoundDimensions[i].serialise(out);
      if (verboseFlag) {
        System.err.println(
          "Serialised CompoundDimension: " + compoundDimensions[i].canonical()
        );
      }
    }
  }

  protected void readAll
  (DataInputStream in)
    throws IntenseException, IOException
  {
    Context context = new Context();
    ContextOp contextOp = new ContextOp();
    CompoundDimension compoundDimension = new CompoundDimension();
    int i;

    for (i = 0; i < contextStrings.length; i++) {
      context.deserialise(in, null);
      if (context.canonical().compareTo(contextStrings[i]) != 0) {
        throw new IntenseTestException(
          "Deserialised Context " + i + " yielded canonical:\n" +
          context.canonical() + "expected:\n" + contextStrings[i]
        );
      }
      if (verboseFlag) {
        System.err.println("Deserialised Context: " + context.canonical());
      }
    }
    for (i = 0; i < contextOpStrings.length; i++) {
      contextOp.deserialise(in, null);
      if (contextOp.canonical().compareTo(contextOpStrings[i]) != 0) {
        throw new IntenseTestException(
          "Deserialised ContextOp " + i + " yielded canonical:\n" +
          contextOp.canonical() + "expected:\n" + contextOpStrings[i]
        );
      }
      if (verboseFlag) {
        System.err.println("Deserialised ContextOp: " + contextOp.canonical());
      }
    }
    for (i = 0; i < compoundDimensionStrings.length; i++) {
      compoundDimension.deserialise(in);
      if (compoundDimension.canonical().
          compareTo(compoundDimensionStrings[i]) != 0) {
        throw new IntenseTestException(
          "Deserialised CompoundDimension " + i + " yielded canonical:\n" +
          compoundDimension.canonical() + "expected:\n" +
          compoundDimensionStrings[i]
        );
      }
      if (verboseFlag) {
        System.err.println(
          "Deserialised CompoundDimension: " + compoundDimension.canonical()
        );
      }
    }
  }

  protected void usage
  ()
  {
    System.err.println(
      "\nusage:\n\n" + execName + " [-t<n threads>] " +
      "[-m (infinite loop mem leak test)]\n" +
      "\t[-i (input mode, use with -o or C++ IntenseTest7 XDR output mode)]\n" +
      "\t[-o (output mode, use with -i or C++ IntenseTest7 XDR input mode)]\n" +
      "\t[-v (verbose - see contexts / context ops)]\n" +
      "\t[-h (help - you're looking at it...)]\n"
    );
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
      } else if (argv[i].equals("-i")) {
        if (nThreads > 1) {
          System.err.println("\n-t(n > 1) not compatable with -i or -o:");
          usage();
        }
        unitTestMode = false;
        inputMode = true;
        outputMode = false;
        inputStream = new DataInputStream(System.in);
      } else if (argv[i].equals("-o")) {
        if (nThreads > 1) {
          System.err.println("\n-t(n > 1) not compatable with -i or -o:");
          usage();
        }
        unitTestMode = false;
        inputMode = false;
        outputMode = true;
        outputStream = new DataOutputStream(System.out);
      } else if (argv[i].equals("-v")) {
        verboseFlag = true;
      } else if ((matcher = tPattern.matcher(argv[i])).find()) {
        String nThreadsString = matcher.group(1);

        try {
          nThreads = Integer.parseInt(nThreadsString);
        } catch (NumberFormatException e) {
          System.err.println(
            "Invalid threads-number expression \"" + nThreadsString + "\""
          );
          System.exit(1);
        }
        if ((nThreads > 1)&&(inputMode||outputMode)) {
          System.err.println("\n-t(n > 1) not compatable with -i or -o:");
          usage();
        }
      } else {
        usage();
      }
    }
  }

  public static void main(String[] argv)
  {
    IntenseTest7 test = new IntenseTest7();
    test.testMain(argv);
  }

}
