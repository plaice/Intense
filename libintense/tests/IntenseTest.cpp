// ****************************************************************************
//
// IntenseTest.cpp : Base for tests of libintense.
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
#include <sstream>
#include <stdexcept>
#include <map>
#include <set>
#include <list>
#include <string>
#include <pthread.h>
#include <intense.hpp>
#include "IntenseTest.hpp"


using namespace intense;


void IntenseTest::usage
()
{
  cerr << "\nusage:\n\n" << execName << " [-t<n threads>] "
       << "[-m (infinite loop mem leak test)]\n\t"
       << "[-h (help - you're looking at it...)]\n\n";
}


void IntenseTest::getOpts
(int argc, char *argv[])
{
  int c;

  while ((c = getopt(argc, argv, "ht:m")) != -1) {
    switch (c) {
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
      }
      break;
    case 'm':
      repeating = true;
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


void IntenseTest::testParse
(int i, Context& c, const char* canonical, const char* expectedCanonical)
{
  try {
    c.parse(canonical);
  } catch (IntenseException& e) {
    ostringstream oss;

    oss << "ERROR: Context or ContextOp parse failed!\n"
        << "input: " << canonical << "\nerror: \"" << e.what() << "\"";
    throw IntenseTestException(i, oss.str());
  }
  if (c.canonical() != expectedCanonical) {
    ostringstream oss;

    oss << "ERROR: Canonical form was incorrect for parse!\n"
        << "input: " << canonical << "\n"
        << "c.canonical(): " << c.canonical() << "\n"
        << "expected:      " << expectedCanonical << "\n";
    throw IntenseTestException(i, oss.str());
  }
}


void IntenseTest::testContext
(int i, const char* input, const char* expectedCanonical, int expectedBasecount)
{
  Context c;

  testParse(i, c, input, expectedCanonical);
  if (c.baseCount() != expectedBasecount) {
    ostringstream oss;

    oss << "ERROR: Parsed Context basecount is wrong!\n"
        << "input: " << input << "\nc.canonical(): " << c.canonical()
        << "\nbasecount: " << c.baseCount() << "\nexpected: "
        << expectedBasecount << "\n";
    throw IntenseTestException(i, oss.str());
  }
}


void IntenseTest::testContextOp
(int i, const char* input, const char* expectedCanonical,
 const char* expectedShortCanonical, int expectedBlankcount,
 int expectedBasecount)
{
  ContextOp o, o2;

  try {
    o.parse(input);
  } catch (IntenseException& e) {
    ostringstream oss;

    oss << "ERROR: Context or ContextOp parse failed!\n"
        << "input: " << input << "\nerror: \"" << e.what() << "\"";
    throw IntenseTestException(i, oss.str());
  }
  o2.apply(o);
  if (o2.canonical() != expectedCanonical) {
    ostringstream oss;

    oss << "ERROR: Canonical form was incorrect for "
        << "parse after application to []!\ninput: " << input << "\n"
        << "o.canonical(): " << o.canonical()
        << "\no2.canonical(): " << o2.canonical() << "\nexpected:       "
        << expectedCanonical << "\n";
    throw IntenseTestException(i, oss.str());
  } else if (o2.shortCanonical() != expectedShortCanonical) {
    ostringstream oss;

    oss << "ERROR: Short canonical form was incorrect for "
        << "parse after application to []!\ninput: " << input
        << "\no.shortCanonical(): " << o.shortCanonical()
        << "\no2.shortCanonical(): " << o2.shortCanonical() << "\nexpected: "
        << expectedShortCanonical << "\n";
    throw IntenseTestException(i, oss.str());
  }
  if (o2.blankCount() != expectedBlankcount) {
    ostringstream oss;

    oss << "ERROR: Applied ContextOp blankcount is wrong!\n"
        << "input: " << input << "\no.canonical(): " << o.canonical()
        << "\no2.canonical(): " << o2.canonical() << "\nblankcount: "
        << o2.blankCount() << "\nexpected: " << expectedBlankcount << "\n";
    throw IntenseTestException(i, oss.str());
  }
  if (o2.baseCount() != expectedBasecount) {
    ostringstream oss;

    oss << "ERROR: Applied ContextOp basecount is wrong!\ninput: "
        << input << "\nbasecount: " << o2.baseCount() << "\no.canonical(): "
        << o.canonical() << "\no2.canonical(): " << o2.canonical()
        << "\nexpected: " << expectedBasecount << "\n";
    throw IntenseTestException(i, oss.str());
  }
}


void IntenseTest::testOperation
(int i, const char* c, const char* o, const char* expectedCanonical,
 int expectedBasecount)
{
  Context context, context2;
  ContextOp contextOp;

  try {
    context.parse(c);
  } catch (IntenseException& e) {
    ostringstream oss;

    oss << "ERROR: Context parse failed!\ninput: " << c
        << "\nerror: \"" << e.what() << "\"";
    throw IntenseTestException(i, oss.str());
  }
  context2.assign(context);
  if (!(context.equals(context2))) {
    ostringstream oss;

    oss << "ERROR: Context assignment was incorrect!\ncontext input: " << c 
        << "\ncontext: " << context << "\nresult: " << context2
        << "\nexpected: " << c;
    throw IntenseTestException(i, oss.str());
  }
  try {
    contextOp.parse(o);
  } catch (IntenseException& e) {
    ostringstream oss;

    oss << "ERROR: ContextOp parse failed!\ninput: " << o
        << "\nerror: \"" << e.what() << "\"";
    throw IntenseTestException(i, oss.str());
  }
  context2.apply(contextOp);
  if (context2.canonical() != expectedCanonical) {
    ostringstream oss;

    oss << "ERROR: Operation was incorrect!\ncontext: "
        << c << "\noperator: " << o << "\nresult:   " << context2.canonical()
        << "\nexpected: " << expectedCanonical << "\n";
    throw IntenseTestException(i, oss.str());
  }
  if (context2.baseCount() != expectedBasecount) {
    ostringstream oss;

    oss << "ERROR: Operation result basecount was ";
    oss << "incorrect!\ncontext: " << c << "\noperator: " << o << "\nresult: "
        << context2.canonical() << "\nresult basecount: "
        << context2.baseCount() << "\nexpected: " << expectedBasecount << "\n";
    throw IntenseTestException(i, oss.str());
  }
}


void IntenseTest::testOperation
(int i, const char* o1, const char* o2, const char* expectedCanonical,
 const char* expectedShortCanonical, int expectedBlankcount,
 int expectedBasecount)
{
  ContextOp op1, op2;

  try {
    op1.parse(o1);
  } catch (IntenseException& e) {
    ostringstream oss;

    oss << "ERROR: ContextOp parse failed!\ninput: " << o1
        << "\nerror: \"" << e.what() << "\"";
    throw IntenseTestException(i, oss.str());
  }
  try {
    op2.parse(o2);
  } catch (IntenseException& e) { 
    ostringstream oss;

    oss << "ERROR: ContextOp parse failed!\ninput: " << o2
        << "\nerror \"" << e.what() << "\"";
    throw IntenseTestException(i, oss.str());
  }
  op1.apply(op2);
  if (op1.canonical() != expectedCanonical) {
    ostringstream oss;

    oss << "ERROR: Operation canonical form was incorrect!\n"
        << "op1: " << o1 << "\nop2: "
        << o2 << "\nresult: " << op1.canonical() << "\nexpected:     "
        << expectedCanonical << "\n";
    throw IntenseTestException(i, oss.str());
  }
  if (op1.shortCanonical() != expectedShortCanonical) {
    ostringstream oss;

    oss << "ERROR: Operation short canonical form was "
        << "incorrect!\nop1: " << o1 << "\nop2: " << o2 << "\nresult: "
        << op1.shortCanonical() << "\nexpected: " << expectedShortCanonical
        << "\n";
    throw IntenseTestException(i, oss.str());
  }
  if (op1.blankCount() != expectedBlankcount) {
    ostringstream oss;

    oss << "ERROR: Operation result blankcount was "
        << "incorrect!\nop1: " << op1.canonical() << "\nop2: "
        << op2.canonical() << "\nop2 blankcount: " << op2.blankCount()
        << "\nresult: " << op1.canonical() << "\nresult blankcount: "
        << op1.blankCount() << "\nexpected: " << expectedBlankcount << "\n";
    throw IntenseTestException(i, oss.str());
  }
  if (op1.baseCount() != expectedBasecount) {
    ostringstream oss;

    oss << "ERROR: Operation result basecount was "
        << "incorrect!\nop1: " << o1 << "\nop2: " << o2 << "\n"
        << "result: " << op1.canonical() << "\nresult basecount: "
        << op1.baseCount() << "\nexpected: " << expectedBasecount << "\n";
    throw IntenseTestException(i, oss.str());
  }
}


void IntenseTest::testRefines
(int i, Context& c1, Context& c2, bool expected)
{
  if ((c1 <= c2) != expected) {
    ostringstream oss;

    oss << "ERROR: Refinement test was incorrect!\nc1.canonical(): "
        << c1.canonical() << "\nc2.canonical(): " << c2.canonical()
        << "\nexpected: " << expected << "\n";
    throw IntenseTestException(i, oss.str());
  }
}


void IntenseTest::testFit
(int i, ContextDomain& domain, Context& requested,
 std::string expectedCanonical)
{
  Context* fit = domain.best(requested);

  if (fit) {
    if (expectedCanonical == "") {
      ostringstream oss;

      cout << "Best fit for " << requested.canonical() << ":\n\t"
           << fit->canonical() << ".\n\tExected none!\n";
      throw IntenseTestException(i, oss.str());
    } else if (fit->canonical() != expectedCanonical) {
      ostringstream oss;

      cout << "Best fit for " << requested.canonical() << ":\n\t"
           << fit->canonical() << ".\nExected:\n\t "
           << expectedCanonical << "\n";
      throw IntenseTestException(i, oss.str());
    }
  } else if (expectedCanonical != "") {
    ostringstream oss;

    cout << "No best fit for " << requested.canonical() << ".\n"
         << "Expected:\n\t" << expectedCanonical << "\n";
    throw IntenseTestException(i, oss.str());
  }
}


void IntenseTest::testNearFit
(int i, ContextDomain& domain, Context& requested,
 std::string expectedCanonicals)
{
  string canonicals;
  list<Context*> nearFit;

  nearFit = domain.near(requested);
  if (nearFit.size() > 0) {
    ostringstream oss;
    list<Context*>::iterator itr = nearFit.begin();

    while (itr != nearFit.end()) {
      // Append canonical form of next expected near-fit context:
      oss << **itr++;
    }
    canonicals = oss.str();
  }
  if (expectedCanonicals != canonicals) {
    ostringstream oss;

    oss << "Near fit for " << requested.canonical() << ":\n\t"
        << canonicals << ".\nExected:\n\t" << expectedCanonicals << "\n";
    throw IntenseTestException(i, oss.str());
  }
}


void* IntenseTest::testThread
(void* arg)
{
  IntenseTest* test = (IntenseTest*)arg;

  if (test->repeating) {
    while (test->repeating) {
      test->test();
    }
  } else {
    test->test();
  }
  return NULL;
}


IntenseTest::~IntenseTest
()
{
  if (threads != NULL) {
    delete threads;
  }
}


void IntenseTest::testMain
()
{
  if (nThreads > 1) {
    // Reentrancy test...
    threads = new pthread_t[nThreads];

    for (int i = 0; i < nThreads - 1; i++) {
      if (pthread_create(&(threads[i]), NULL, testThread, this)) {
        cerr << "Couldn't create thread!\n";
        exit(1);
      }
    }
    // The main thread becomes the nth thread:
    testThread(this);
  } else {
    testThread(this);
  }
}


extern IntenseTest* test;


int main
(int argc, char* argv[])
{
  try {
    test->getOpts(argc, argv);
    test->testMain();
  } catch (IntenseTestException& e) {
    cout << '\n' << test->getExecName() << " failed:\n\n" << e.what()
         << "\n\n";
    exit(1);
  } catch (IntenseException& e) {
    cout << '\n' << test->getExecName()
         << " generated an IntenseException:\n\n" << e.what() << "\n\n";
    exit(2);
  } catch (runtime_error& e) {
    cout << '\n' << test->getExecName()
         << " generated a runtime error:\n\n" << e.what() << "\n\n";
    exit(3);
  } catch (exception& e) {
    cout << '\n' << test->getExecName()
         << " generated an exception:\n\n" << e.what() << "\n\n";
    exit(4);
  } catch (...) {
    cout << '\n' << test->getExecName()
         << " generated an unknown exception.\n\n";
    exit(5);
  }
  return 0;
}
