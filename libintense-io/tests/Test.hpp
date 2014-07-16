// ****************************************************************************
//
// Test.hpp : Generic test framework for threaded and memory-leak tests.
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


class TestException
  : public runtime_error {

  int testId;

public:

  TestException
  (int newTestId, const string& what)
    : testId(newTestId), runtime_error(what)
  {
  }

  TestException
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


class Test {

private:

  pthread_t* threads;


protected:

  string execName;

  int nThreads;

  bool repeating;

  void usage();

public:

  Test
  (const char* _execName)
    : threads(NULL), execName(_execName), nThreads(1), repeating(false)
  {
  }

  virtual ~Test
  ();

  void testMain
  ();

  string getExecName
  ()
  {
    return execName;
  }

  void getOpts
  (int argc, char* argv[]);

  static void* testThread
  (void* arg);

  virtual void test
  () = 0;

};
