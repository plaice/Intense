// ****************************************************************************
//
// IntenseTest.hpp : Base for tests of libintense.
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


using namespace std;
using namespace intense;


class IntenseTestException
  : public runtime_error {

  int testId;

public:

  IntenseTestException
  (int newTestId, const string& what)
    : testId(newTestId), runtime_error(what)
  {
  }

  IntenseTestException
  (const string& what)
    : testId(-1), runtime_error(what)
  {}

  virtual const char* what
  ()
  {
    ostringstream oss;

    if (testId >= 0) {
      oss << "test " << testId << ": " << runtime_error::what();
    } else {
      oss << "test : " << runtime_error::what();
    }
    return oss.str().data();
  }

};


class IntenseTest {

private:

  pthread_t* threads;


protected:

  string execName;

  int nThreads;

  bool repeating;

  virtual void usage();

  void testParse
  (int i, Context& c, const char* canonical, const char* expectedCanonical);

  void testParse
  (int i, Context& c, const char* canonical)
  {
    testParse(i, c, canonical, canonical);
  }

  void testContext
  (int i, const char* input, const char* expectedCanonical,
   int expectedBasecount);

  void testContextOp
  (int i, const char* input, const char* expectedCanonical,
   const char* expectedShortCanonical, int expectedBlankcount,
   int expectedBasecount);

  void testOperation
  (int i, const char* c, const char* o, const char* expectedCanonical,
   int expectedBasecount);

  void testOperation
  (int i, const char* o1, const char* o2, const char* expectedCanonical,
   const char* expectedShortCanonical, int expectedBlankcount,
   int expectedBasecount);

  void testRefines
  (int i, Context& c1, Context& c2, bool expected);

  void testFit
  (int i, ContextDomain& domain, Context& requested,
   std::string expectedCanonical /* "" if no best fit expected */);

  void testNearFit
  (int i, ContextDomain& domain, Context& requested,
   std::string expectedCanonicals);

public:

  IntenseTest
  (const char* _execName)
    : threads(NULL), execName(_execName), nThreads(1), repeating(false)
  {
  }

  virtual ~IntenseTest
  ();

  void testMain
  ();

  string getExecName
  ()
  {
    return execName;
  }

  virtual void getOpts
  (int argc, char* argv[]);

  static void* testThread
  (void* arg);

  virtual void test
  () = 0;

};
