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


class TestException
  : public std::runtime_error {

  int testId;

public:

  TestException
  (int newTestId, const std::string& what)
    : testId(newTestId), std::runtime_error(what)
  {
  }

  TestException
  (const std::string& what)
    : testId(-1), std::runtime_error(what)
  {}

  virtual const char* what
  ()
  {
    std::ostringstream oss;

    if (testId >= 0) {
      oss << "test " << testId << ": " << std::runtime_error::what();
    } else {
      oss << "test : " << std::runtime_error::what();
    }
    return oss.str().data();
  }

};


class Test {

private:

  pthread_t* threads;


protected:

  std::string execName;

  int nThreads;

  int nTimes;

  bool repeating;

  virtual void usage();

  virtual bool permitMultiThreadedTest
  ()
  {
    return true;
  }

public:

  Test
  (const char* _execName)
    : threads(NULL), execName(_execName), nThreads(1), nTimes(0),
      repeating(false)
  {
  }

  virtual ~Test
  ();

  void testMain
  ();

  std::string getExecName
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
