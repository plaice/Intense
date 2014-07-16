// ****************************************************************************
//
// IntenseAEPTest0.cpp : A basic sanity test for the shared mode AEP 2.
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


#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>
#include <pthread.h>
#include "intense-aep.hpp"
#include "Test.hpp"
#include "IntenseAEPTest.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;
using namespace intense::aep;


class IntenseAEPTest0
  : public IntenseAEPTest {

public:

  IntenseAEPTest0
  ()
    : IntenseAEPTest("IntenseAEPTest0")
  {}

  void test
  ();

};


IntenseAEPTest* test = new IntenseAEPTest0();


//#define LOG_LEVEL Log::DEBUG2
#define LOG_LEVEL Log::ERROR
//#define LOG_LEVEL Log::CATASTROPHIC


void IntenseAEPTest0::test
()
{
  LogStream* logStream = new LogStream(cerr, LOG_LEVEL, true, true);
  Log* log = new Log(*logStream);
  AEPServer::AEther* aether = new AEPServer::AEther(10, log, LOG_LEVEL);

  setLog(*log);
  pairs.push_back(new SharedClientServerPair("s1", *aether, *log));
  pairs.push_back(new SharedClientServerPair("s2", *aether, *log));
  pairs.push_back(new SharedClientServerPair("s3", *aether, *log));
  pairs.push_back(new SharedClientServerPair("s4", *aether, *log));
  pairs.push_back(new SharedClientServerPair("s5", *aether, *log));
  pairs.push_back(new SharedClientServerPair("s6", *aether, *log));
  aepTest();
  deletePairs();
  delete aether;
  delete log;
  delete logStream;
}
