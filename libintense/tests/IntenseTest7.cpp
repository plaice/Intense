// ****************************************************************************
//
// IntenseTest7.cpp : A test of Context and ContextOp serialisation.
//
// IntenseTest7 -m -o -b | IntenseTest7 -i -m -v -b
// IntenseTest7 -m -o -x | IntenseTest7 -i -m -v -x
// java IntenseTest7 -m -o | IntenseTest7 -i -m -v -x
// IntenseTest7 -m -o -x | java IntenseTest7 -i -m -v
//
// The following combinations are useful for memory leak testing, between C++
// and C++ or between C++ and Java:
//
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


#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <map>
#include <set>
#include <list>
#include <string>
#include <pthread.h>
#include <intense.hpp>
#include "IntenseTest.hpp"


using namespace intense;
using namespace intense::io;


class IntenseTest7
  : public IntenseTest {

  // contextStrings and contextOpStrings, below, are pretty much just a random
  // cut-and-paste job with replacements, from parsed canonical forms from
  // previous unit tests.  They are the same in IntenseTest7.java and
  // IntenseTest7.cpp.

private:

  const static char* contextStrings[];

  const static char* contextOpStrings[];

  const static char* compoundDimensionStrings[];

  bool unitTestMode;

  bool inputMode;

  bool outputMode;

  bool verboseFlag;

  Context* contexts;

  int contextsCount;

  ContextOp* contextOps;

  CompoundDimension* compoundDimensions;

  int contextOpsCount;

  int compoundDimensionsCount;

  bool xdrMode;

public:

  IntenseTest7
  ();

  void test
  ();

  void writeAll
  (BaseSerialiser* serialiser, ostream& out);

  void readAll
  (BaseSerialiser* serialiser, istream& in);

  void IntenseTest7::usage
  ();

  void getOpts
  (int argc, char *argv[]);

};


const char* IntenseTest7::contextStrings[] = {
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
  "<d1:<1.2345e+08>+d2:<1.2345e+12+d21:<d211:<-1.2345e-06>+d212:<\"123\">>>>",
  "<1:<\"1val\"+12:<\"12val\"+121:<\"121val\">>>>",
  "<2:<\"2val\">+\302\221\302\222\302\221\302\222\302\221\302\222\302\221\302\222:<\"1val\">>",
  "<2:<\"2val\">+abc123_\302\221\302\222\302\221\302\222\302\221\302\222\302\221\302\222_def456:<\"1val\">>",
  "<2:<\"2val\">+\"abc 123 \302\221\302\222\302\221\302\222\302\221\302\222\302\221\302\222 def 456\":<\"1val\">>",
  "<dim1:<\"\302\221\302\222\302\221\302\222\302\221\302\222\302\221\302\222\">>",
  "<dim1:<\"abc123_\302\221\302\222\302\221\302\222\302\221\302\222\302\221\302\222_def456\">>",
  "<dim1:<\"abc 123 \302\221\302\222\302\221\302\222\302\221\302\222\302\221\302\222 def 456\">>",
  "<d1:<\"_\\a\">+d2:<\"_\\b\">+d3:<\"_\\t\">+d4:<\"_\\n\">+d5:<\"_\\v\">+d6:<\"_\\f\">+d7:<\"_\\r\">>",
  "<da:<\"_\\\\\">+db:<\"_\\\"\">+dc:<\"_#\">+dd:<\"_ \">+de:<\"_~\">+df:<\"_^\">+dg:<\"_-\">+dh:<\"_<\">+di:<\"_>\">+dj:<\"_[\">+dk:<\"_]\">+dl:<\"_+\">+dm:<\"_:\">>",
  "<\"d1\\a\":<1>+\"d2\\b\":<2>+\"d3\\t\":<3>+\"d4\\n\":<4>+\"d5\\v\":<5>+\"d6\\f\":<6>+\"d7\\r\":<7>>",
  "<\"da\\\\\":<1>+\"db\\\"\":<2>+\"dc#\":<3>+\"dd \":<4>+\"de~\":<5>+\"df^\":<6>+\"dg-\":<7>+\"dh<\":<8>+\"di>\":<9>+\"dj[\":<10>+\"dk]\":<11>+\"dl+\":<12>+\"dm:\":<13>>",
  NULL
};


const char* IntenseTest7::contextOpStrings[] = {
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
  "[--+1:[--+\"1val\"+12:[\"12val\"+121:[--+\"121val\"]]]]",
  NULL
};


const char* IntenseTest7::compoundDimensionStrings[] = {
  "a",
  "a:b",
  "a:b:c",
  "abc:def",
  "abc:def:ghi",
  "123",
  "123:456",
  "123:456:789",
  "abc:123",
  "123:abc",
  NULL
};


IntenseTest* test = new IntenseTest7();


IntenseTest7::IntenseTest7
()
  : IntenseTest("IntenseTest7"), unitTestMode(true), inputMode(false),
    outputMode(false), verboseFlag(false), contexts(NULL), contextsCount(0),
    contextOps(NULL), contextOpsCount(0), xdrMode(true)
{
  int i;
  char** marker;

  marker = (char**)contextStrings;
  while (*marker != NULL) {
    contextsCount++;
    marker++;
  }
  marker = (char**)contextOpStrings;
  while (*marker != NULL) {
    contextOpsCount++;
    marker++;
  }
  marker = (char**)compoundDimensionStrings;
  while (*marker != NULL) {
    compoundDimensionsCount++;
    marker++;
  }
  try {
    contexts = new Context[contextsCount];
    for (i = 0; i < contextsCount; i++) {
      contexts[i].parse(contextStrings[i]);
    }
    contextOps = new ContextOp[contextOpsCount];
    for (i = 0; i < contextOpsCount; i++) {
      contextOps[i].parse(contextOpStrings[i]);
    }
    compoundDimensions = new CompoundDimension[compoundDimensionsCount];
    for (i = 0; i < compoundDimensionsCount; i++) {
      compoundDimensions[i].parse(compoundDimensionStrings[i]);
    }
  } catch (IntenseException& e) {
    cerr << "Error during intialisation: " << e.what() << "\n";
    exit(1);
  }
}


void IntenseTest7::test
()
{
  BinaryBaseSerialiser binaryBaseSerialiser;
  XDRBaseSerialiser xdrBaseSerialiser;
  BaseSerialiser* serialiser;

  if (xdrMode) {
    serialiser = &xdrBaseSerialiser;
  } else {
    serialiser = &binaryBaseSerialiser;
  }
  if (unitTestMode) {
    stringstream outAndIn;

    outAndIn.exceptions(ios::failbit|ios::badbit);
    writeAll(&xdrBaseSerialiser, outAndIn);
    writeAll(&binaryBaseSerialiser, outAndIn);
    readAll(&xdrBaseSerialiser, outAndIn);
    readAll(&binaryBaseSerialiser, outAndIn);
  } else if (inputMode) {
    cin.exceptions(ios::failbit|ios::badbit);
    readAll(serialiser, cin);
  } else if (outputMode) {
    cout.exceptions(ios::failbit|ios::badbit);
    writeAll(serialiser, cout);
  }
}


void IntenseTest7::writeAll
(BaseSerialiser* serialiser, ostream& out)
{
  int i;

  for (i = 0; i < contextsCount; i++) {
    contexts[i].serialise(*serialiser, out, NULL);
    if (verboseFlag) {
      cerr << "Serialised Context: " << contexts[i] << "\n";
    }
  }
  for (i = 0; i < contextOpsCount; i++) {
    contextOps[i].serialise(*serialiser, out, NULL);
    if (verboseFlag) {
      cerr << "Serialised ContextOp: " << contextOps[i] << "\n";
    }
  }
  for (i = 0; i < compoundDimensionsCount; i++) {
    compoundDimensions[i].serialise(*serialiser, out);
    if (verboseFlag) {
      cerr << "Serialised CompoundDimension: "
           << compoundDimensions[i].canonical() << "\n";
    }
  }
}


void IntenseTest7::readAll
(BaseSerialiser* serialiser, istream& in)
{
  Context context;
  ContextOp contextOp;
  CompoundDimension compoundDimension;
  int i;

  for (i = 0; i < contextsCount; i++) {
    context.deserialise(*serialiser, in, NULL);
    if (context.canonical() != contextStrings[i]) {
      ostringstream oss;

      oss << "Deserialised Context " << i << " yielded canonical:\n"
          << context << "\nexpected:\n" << contextStrings[i];
      throw IntenseTestException(oss.str());
    }
    if (verboseFlag) {
      cerr << "Deserialised Context: " << context.canonical() << "\n";
    }
  }
  for (i = 0; i < contextOpsCount; i++) {
    contextOp.deserialise(*serialiser, in, NULL);
    if (contextOp.canonical() != contextOpStrings[i]) {
      ostringstream oss;

      oss << "Deserialised ContextOp " << i << " yielded canonical:\n"
          << contextOp << "\nexpected:\n" << contextOpStrings[i];
      throw IntenseTestException(oss.str());
    }
    if (verboseFlag) {
      cerr << "Deserialised ContextOp: " << contextOp << "\n";
    }
  }
  for (i = 0; i < compoundDimensionsCount; i++) {
    compoundDimension.deserialise(*serialiser, in);
    if (compoundDimension.canonical() != compoundDimensionStrings[i]) {
      ostringstream oss;

      oss << "Deserialised CompoundDimension " << i << " yielded canonical:\n"
          << compoundDimension.canonical() << "\nexpected:\n"
          << compoundDimensionStrings[i];
      throw IntenseTestException(oss.str());
    }
    if (verboseFlag) {
      cerr << "Deserialised CompoundDimension: "
           << compoundDimension.canonical() << "\n";
    }
  }
}


void IntenseTest7::usage
()
{
  cerr << "\nusage:\n\n" << execName << " [-t<n threads>] "
       <<"[-m (infinite loop mem leak test)]\n"
       << "\t[-i (input mode, use with -o or C++ IntenseTest7 XDR output mode)]\n"
       << "\t[-o (output mode, use with -i or C++ IntenseTest7 XDR input mode)]\n"
       << "\t[-b (Binary serialisation mode)]\n"
       << "\t[-x (XDR serialisation mode (default))]\n"
       << "\t[-v (verbose - see contexts / context ops)]\n"
       << "\t[-h (help - you're looking at it...)]\n\n";
}


void IntenseTest7::getOpts
(int argc, char *argv[])
{
  int c;

  while ((c = getopt(argc, argv, "ht:mvioabx")) != -1) {
    switch (c) {
    case 'i':
      if (nThreads > 1) {
        cerr << "\n-t(n > 1) not compatable with -i or -o:\n";
        usage();
      }
      unitTestMode = false;
      inputMode = true;
      outputMode = false;
      break;
    case 'o':
      if (nThreads > 1) {
        cerr << "\n-t(n > 1) not compatable with -i or -o:\n";
        usage();
      }
      unitTestMode = false;
      inputMode = false;
      outputMode = true;
      break;
    case 'v':
      verboseFlag = true;
      break;
    case 'h':
      // Help:
      usage();
      exit(0);
      break;
    case 't':
      nThreads = atoi(optarg);
      if (nThreads < 1) {
        cerr << "\nError: negative number of threads specified!\n";
        usage();
        exit(1);
      } else if ((nThreads > 1)&&(inputMode||outputMode)) {
        cerr << "\n-t(n > 1) not compatable with -i or -o:\n";
        usage();
      }
      break;
    case 'm':
      repeating = true;
      break;
    case 'b':
      xdrMode = false;
      break;
    case 'x':
      xdrMode = true;
      break;
    case '?':
      if (isprint(optopt)) {
        cerr << "\nUnknown option `-" << (char)optopt << "'.\n";
      } else {
        cerr << "\nUnknown option character `\\x" << (char)optopt << "'.\n";
      }
      cerr << "Could not parse command-line options.\n";
      usage();
      exit(1);
    default:
      abort();
    }
  }
}
