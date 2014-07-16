// ****************************************************************************
//
// Test.cpp : Generic test framework for threaded and memory-leak tests.
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
#include <pthread.h>
#include "Test.hpp"


void Test::usage
()
{
  cerr << "\nusage:\n\n" << execName << " [-t<n threads>] "
       << "[-m (inf. loop mem leak test)] [-h (help)]\n\n";
}


void Test::getOpts
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
        cerr << "\nUnknown option `-" << optopt << "'.\n";
      } else {
        cerr << "\nUnknown option character `\\x" << optopt << "'.\n";
      }
      cerr << "Could not parse command-line options.\n";
      usage();
      exit(1);
    default:
      abort();
    }
  }
}


void* Test::testThread
(void* arg)
{
  Test* test = (Test*)arg;

  if (test->repeating) {
    while (test->repeating) {
      test->test();
    }
  } else {
    test->test();
  }
  return NULL;
}


Test::~Test
()
{
  if (threads != NULL) {
    delete threads;
  }
}


void Test::testMain
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


extern Test* test;


int main
(int argc, char* argv[])
{
  try {
    test->getOpts(argc, argv);
    test->testMain();
  } catch (TestException& e) {
    cout << '\n' << test->getExecName() << " failed:\n\n" << e.what()
         << "\n\n";
    exit(1);
  } catch (runtime_error& e) {
    cout << '\n' << test->getExecName()
         << " generated a runtime error:\n\n" << e.what() << "\n\n";
    exit(2);
  } catch (exception& e) {
    cout << '\n' << test->getExecName()
         << " generated an unknown exception:\n\n" << e.what() << "\n\n";
    exit(3);
  } catch (...) {
    cout << '\n' << test->getExecName()
         << " generated an unknown exception.\n\n";
    exit(3);
  }
  return 0;
}
