// ****************************************************************************
//
// IntenseAEPTest1.cpp : A basic sanity test for the binary and XDR modes of
// AEP 2.
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
#include <intense.hpp>
#define yyFlexParser aetp_yyFlexParser
#include <FlexLexer.h>
#include "AETPLexer.hpp"
#include "intense-aep.hpp"
#include "Test.hpp"
#include "IntenseAEPTest.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;
using namespace intense::aep;


class IntenseAEPTest1
  : public IntenseAEPTest {

public:

  IntenseAEPTest1
  ()
    : IntenseAEPTest("IntenseAEPTest1")
  {}

  void test
  ();

};


IntenseAEPTest* test = new IntenseAEPTest1();


//#define LOG_LEVEL Log::DEBUG2
#define LOG_LEVEL Log::ERROR
//#define LOG_LEVEL Log::CATASTROPHIC


#define TEST_BINARY_AEP 1
#define TEST_XDR_AEP 1


void IntenseAEPTest1::test
()
{
  LogStream* logStream = new LogStream(cerr, LOG_LEVEL, true, true);
  Log* log = new Log(*logStream);
  AEPServer::AEther* aether = new AEPServer::AEther(10, log, LOG_LEVEL);

  setLog(*log);
#if TEST_BINARY_AEP
  pairs.push_back(new BinaryClientServerPair("s1", *aether, *log));
  pairs.push_back(new BinaryClientServerPair("s2", *aether, *log));
  pairs.push_back(new BinaryClientServerPair("s3", *aether, *log));
#endif // TEST_BINARY_AEP
#if TEST_XDR_AEP
  pairs.push_back(new XDRClientServerPair("x1", *aether, *log));
  pairs.push_back(new XDRClientServerPair("x2", *aether, *log));
  pairs.push_back(new XDRClientServerPair("x3", *aether, *log));
#endif // TEST_XDR_AEP
  aepTest();
  deletePairs();
  delete aether;
  delete log;
  delete logStream;
}
