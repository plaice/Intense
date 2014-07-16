// ****************************************************************************
//
// IntenseAEPTest.cpp : Base for tests of libintense-aep.
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
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <errno.h>
#include <intense.hpp>
#define yyFlexParser aetp_yyFlexParser
#include <FlexLexer.h>
#include "AETPLexer.hpp"
#include <intense-aep.hpp>
#include "Test.hpp"
#include "IntenseAEPTest.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;
using namespace intense::aep;


void IntenseAEPTest::TestParticipant::assignNotify
(const Context& newValue, const CompoundDimension* dim, Origin* origin)
{
  if (log != NULL) {
    *log << logLevel;
  }
  if (dim != NULL) {
    if (log != NULL) {
      *log << name << ": " << dim->canonical() << ":" << newValue;
    }
    localContext[*dim].assign(newValue);
  } else { 
    if (log != NULL) {
      *log << name << ": " << newValue;
    }
    localContext.assign(newValue);
  }
  if (origin == NULL) {
    if (log != NULL) {
      *log << " origin: void";
    }
  } else {
    if (log != NULL) {
      *log << " origin: server sequence: "
           << ((AEPClient::NotifyToken*)origin)->getServerSequence();
    }
  }
  if (log != NULL) {
    *log << ENDLOG;
  }
}


void IntenseAEPTest::TestParticipant::applyNotify
(const ContextOp& o, const CompoundDimension* dim, Origin* origin)
{
  if (log != NULL) {
    *log << logLevel;
  }
  if (dim != NULL) {
    if (log != NULL) {
      *log << name << ": " << dim->canonical() << ":" << o;
    }
    localContext[*dim].apply(o);
  } else { 
    if (log != NULL) {
      *log << name << ": " << o;
    }
    localContext.apply(o);
  }
  if (origin == NULL) {
    if (log != NULL) {
      *log << " origin: void";
    }
  } else {
    if (log != NULL) {
      *log << " origin: server sequence: "
           << ((AEPClient::NotifyToken*)origin)->getServerSequence();
    }
  }
  if (log != NULL) {
    *log << ENDLOG;
  }
}


void IntenseAEPTest::TestParticipant::clearNotify
(const CompoundDimension* dim, Origin* origin)
{
  if (log != NULL) {
    *log << logLevel;
  }
  if (dim != NULL) {
    if (log != NULL) {
      *log << name << ": clear(" << dim->canonical() << ").";
    }
    localContext[*dim].clear();
  } else {
    if (log != NULL) {
      *log << name << ": clear.";
    }
    localContext.clear();
  }
  if (origin == NULL) {
    if (log != NULL) {
      *log << " origin: void";
    }
  } else {
    if (log != NULL) {
      *log << " origin: server sequence: "
           << ((AEPClient::NotifyToken*)origin)->getServerSequence();
    }
  }
  if (log != NULL) {
    *log << ENDLOG;
  }
}


void IntenseAEPTest::TestParticipant::kickNotify
(Origin* origin)
{
  if (log != NULL) {
    *log << logLevel;
    *log << name << ": kicked.";
    if (origin == NULL) {
      *log << " origin: void";
    } else {
      *log << " origin: server sequence: "
           << ((AEPClient::NotifyToken*)origin)->getServerSequence();
    }
    *log << ENDLOG;
  }
}


void IntenseAEPTest::TestParticipant::checkContext
(int testId, const char* expectedCanonical)
{
  if (localContext.canonical() != expectedCanonical) {
    ostringstream oss;

    oss << "IntenseAEPTest::TestParticipant::checkContext: "
        << "unexpected context:\n" << localContext.canonical()
        << "\nexpected: " << expectedCanonical;
    oss << "\nbasecount: " << localContext.baseCount();
    throw TestException(testId, oss.str());
  }
}


IntenseAEPTest::ClientServerPair::~ClientServerPair
()
{
  map<long, TestParticipant*>::iterator itr;

  delete client;
  delete server;
  for (itr = participantMap.begin(); itr != participantMap.end(); itr++) {
    delete itr->second;
  }
}


void IntenseAEPTest::ClientServerPair::disconnect
()
{
  client->disconnect();
}


IntenseAEPTest::TestParticipant&
IntenseAEPTest::ClientServerPair::newParticipant
(long id)
{
  TestParticipant* participant;
  ostringstream oss;

  oss << "Participant " << id;
  participant = new TestParticipant(*client, oss.str().data(), log);
  participantMap[id] = participant;
  return *participant;
}


void
IntenseAEPTest::ClientServerPair::removeParticipant
(long id)
{
  map<long, TestParticipant*>::iterator itr = participantMap.find(id);

  if (itr == participantMap.end()) {
    ostringstream oss;

    oss << "Attempt to remove non-existent participant with "
        << "id \"" << id << "\"";
    throw TestException(oss.str());
  }
  delete itr->second;
  participantMap.erase(itr);
}


IntenseAEPTest::TestParticipant&
IntenseAEPTest::ClientServerPair::participantRef
(long id)
{
  map<long, TestParticipant*>::iterator itr = participantMap.find(id);

  if (itr == participantMap.end()) {
    ostringstream oss;

    oss << "Attempt to acquire reference to non-existent participant with "
        << "id \"" << id << "\"";
    throw TestException(oss.str());
  }
  return *(itr->second);
}


IntenseAEPTest::SharedClientServerPair::SharedClientServerPair
(const char* name, AEPServer::AEther& aether, Log& log, Log::Priority logLevel)
  : ClientServerPair(name, log)
{
  AEPSharedServer* sharedServer =
    new AEPSharedServer(aether, &log, logLevel, true);
  AEPSharedClient* sharedClient =
    new AEPSharedClient(*sharedServer, &log, logLevel);

  setServer(*sharedServer);
  setClient(*sharedClient);
  sharedServer->start();
  sharedClient->start();
}


IntenseAEPTest::StreamClientServerPair::StreamClientServerPair
(const char* name,  Log& log)
  : ClientServerPair(name, log)
{
  int rc;

  if ((rc = pipe(serverInPipes)) != 0) {
    ostringstream oss;

    oss << "Couldn't create server pipes!  error=" << strerror(errno);
    throw TestException(oss.str());
  }
  if ((rc = pipe(clientInPipes)) != 0) {
    ostringstream oss;

    oss << "Couldn't create client pipes!  error=" << strerror(errno);
    throw TestException(oss.str());
  }
  serverIn = new FileInStream(serverInPipes[0]);
  serverIn->exceptions(ios::failbit|ios::badbit);
  serverOut = new FileOutStream(clientInPipes[1]);
  serverOut->exceptions(ios::failbit|ios::badbit);
  clientIn = new FileInStream(clientInPipes[0]);
  clientIn->exceptions(ios::failbit|ios::badbit);
  clientOut = new FileOutStream(serverInPipes[1]);
  clientOut->exceptions(ios::failbit|ios::badbit);
}


IntenseAEPTest::StreamClientServerPair::~StreamClientServerPair
()
{
  disconnect();
  // So ~ClientServerPair doesn't cause use of the streams after
  // ~StreamClientServerPair destroys them:
  client->stop();
  delete client;
  client = NULL;
  server->stop();
  delete server;
  server = NULL;
  serverIn->close();
  delete serverIn;
  serverOut->close();
  delete serverOut;
  clientIn->close();
  delete clientIn;
  clientOut->close();
  delete clientOut;
}


IntenseAEPTest::BinaryClientServerPair::BinaryClientServerPair
(const char* name, AEPServer::AEther& aether, Log& log, Log::Priority logLevel)
  : StreamClientServerPair(name, log)
{
  AEPBinaryServer* binaryServer =
    new AEPBinaryServer(aether, *serverIn, *serverOut, &log, logLevel, true);
  AEPBinaryClient* binaryClient =
    new AEPBinaryClient(*clientIn, *clientOut, &log, logLevel, true);

  setServer(*binaryServer);
  setClient(*binaryClient);
  binaryServer->start();
  binaryClient->start();
}


IntenseAEPTest::XDRClientServerPair::XDRClientServerPair
(const char* name, AEPServer::AEther& aether, Log& log, Log::Priority logLevel)
  : StreamClientServerPair(name, log)
{
  AEPXDRServer* xdrServer =
    new AEPXDRServer(aether, *serverIn, *serverOut, &log, logLevel, true);
  AEPXDRClient* xdrClient =
    new AEPXDRClient(*clientIn, *clientOut, &log, logLevel, true);

  setServer(*xdrServer);
  setClient(*xdrClient);
  xdrServer->start();
  xdrClient->start();
}


IntenseAEPTest::AETPClientServerPair::AETPClientServerPair
(const char* name, AEPServer::AEther& aether, Log& log, Log::Priority logLevel)
  : StreamClientServerPair(name, log)
{
  AETPServer* aetpServer =
    new AETPServer(aether, *serverIn, *serverOut, &log, logLevel, true);
  AETPClient* aetpClient =
    new AETPClient(*clientIn, *clientOut, &log, logLevel, true);

  setServer(*aetpServer);
  setClient(*aetpClient);
  aetpServer->start();
  aetpClient->start();
}


void IntenseAEPTest::deletePairs
()
{
  for (int i = 0; i < pairs.size(); i++) {
    delete pairs[i];
  }
  pairs.clear();
}


IntenseAEPTest::~IntenseAEPTest
()
{
  deletePairs();
}


void IntenseAEPTest::aepTest
()
{
  int flags =
    //    AEPServer::preFenceBit|
    //    AEPServer::postFenceBit|
    AEPServer::notifySelfBit|
    AEPServer::notifyClientBit;
  CompoundDimension dimension;
  long long serverSequence;

  if (log != NULL) {
    *log << "Starting aepTest..." << ENDLOG;
  }
  for (int i = 0; i < pairs.size(); i++) {
    for (int j = 1; j <= 10; j++) {
      TestParticipant& participant = pairs[i]->newParticipant(j);

      if (log != NULL) {
        *log << "Created new participant " << participant.getName() << ENDLOG;
      }
    }
    (*pairs[i])[1].join(true);
    (*pairs[i])[1].leave();
    // participant 1 joined at root:
    (*pairs[i])[1].join(true);
    (*pairs[i])[2].join(true);
    // participant 2 joined at root:
    (*pairs[i])[2].join(true);
    (*pairs[i])[3].join(true, "bogus");
    // participant 3 joined at reactor:
    (*pairs[i])[3].join(true, "reactor");
    // participant 4 joined at reactor:core:
    (*pairs[i])[4].join(true, "reactor:core");
    // participant 5 joined at reactor:core:temp:
    (*pairs[i])[5].join(true, "reactor:core:temp");
    // participant 6 joined at reactor:core:temp:
    (*pairs[i])[6].join(true, "reactor:core:temp");
    // participant 7 joined at reactor:core:pressure:
    (*pairs[i])[7].join(true, "reactor:core:pressure");
    // participant 8 joined at reactor:output:power:
    (*pairs[i])[8].join(true, "reactor:output:power");
    // participant 9 joined at some:bogus:dimension:
    (*pairs[i])[9].join(true, "some:bogus:dimension");
    // participant 10 joined at someother:bogus:dimension:
    (*pairs[i])[10].join(true, "someother:bogus:dimension");
  }
  for (int i = 0; i < pairs.size(); i++) {
    serverSequence = (*pairs[i])[1].apply(
      "[reactor:core:[--+temp:[10+--]]]",
      (const CompoundDimension*)NULL, flags
    );
    (*pairs[i])[1].synch();
    (*pairs[i])[1].checkContext(100 + i, "<reactor:<core:<temp:<10>>>>");
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(200 + i, "<reactor:<core:<temp:<10>>>>");
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(300 + i, "<core:<temp:<10>>>");
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(400 + i, "<temp:<10>>");
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(500 + i, "<10>");
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(600 + i, "<10>");
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(700 + i, "<>");
    (*pairs[i])[8].checkContext(800 + i, "<>");
    (*pairs[i])[9].checkContext(900 + i, "<>");
    (*pairs[i])[10].checkContext(1000 + i, "<>");
    // ************************************************************************
    serverSequence = (*pairs[i])[1].apply(
      "[core:temp:20]", "reactor", flags
    );
    (*pairs[i])[1].synch();
    (*pairs[i])[1].checkContext(1100 + i, "<reactor:<core:<temp:<20>>>>");
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(1200 + i, "<reactor:<core:<temp:<20>>>>");
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(1300 + i, "<core:<temp:<20>>>");
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(1400 + i, "<temp:<20>>");
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(1500 + i, "<20>");
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(1600 + i, "<20>");
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(1700 + i, "<>");
    (*pairs[i])[8].checkContext(1800 + i, "<>");
    (*pairs[i])[9].checkContext(1900 + i, "<>");
    (*pairs[i])[10].checkContext(2000 + i, "<>");
    // ************************************************************************
    serverSequence = (*pairs[i])[1].apply(
      "[30]", "reactor:core:temp", flags
    );
    (*pairs[i])[1].synch();
    (*pairs[i])[1].checkContext(2100 + i, "<reactor:<core:<temp:<30>>>>");
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(2200 + i, "<reactor:<core:<temp:<30>>>>");
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(2300 + i, "<core:<temp:<30>>>");
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(2400 + i, "<temp:<30>>");
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(2500 + i, "<30>");
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(2600 + i, "<30>");
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(2700 + i, "<>");
    (*pairs[i])[8].checkContext(2800 + i, "<>");
    (*pairs[i])[9].checkContext(2900 + i, "<>");
    (*pairs[i])[10].checkContext(3000 + i, "<>");
    // ************************************************************************
    serverSequence = (*pairs[i])[5].apply(
      "[40]", (const CompoundDimension*)NULL, flags
    );
    (*pairs[i])[5].synch();
    (*pairs[i])[1].synch(serverSequence);
    (*pairs[i])[1].checkContext(3100 + i, "<reactor:<core:<temp:<40>>>>");
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(3200 + i, "<reactor:<core:<temp:<40>>>>");
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(3300 + i, "<core:<temp:<40>>>");
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(3400 + i, "<temp:<40>>");
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(3500 + i, "<40>");
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(3600 + i, "<40>");
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(3700 + i, "<>");
    (*pairs[i])[8].checkContext(3800 + i, "<>");
    (*pairs[i])[9].checkContext(3900 + i, "<>");
    (*pairs[i])[10].checkContext(4000 + i, "<>");
    // ************************************************************************
    serverSequence = (*pairs[i])[7].apply(
      "[50]", (const CompoundDimension*)NULL, flags
    );
    (*pairs[i])[7].synch();
    (*pairs[i])[1].synch(serverSequence);
    (*pairs[i])[1].checkContext(
      4100 + i, "<reactor:<core:<pressure:<50>+temp:<40>>>>"
    );
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(
      4200 + i, "<reactor:<core:<pressure:<50>+temp:<40>>>>"
    );
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(
      4300 + i, "<core:<pressure:<50>+temp:<40>>>"
    );
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(
      4400 + i, "<pressure:<50>+temp:<40>>"
    );
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(
      4500 + i, "<40>"
    );
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(
      4600 + i, "<40>"
    );
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(
      4700 + i, "<50>"
    );
    (*pairs[i])[8].checkContext(
      4800 + i, "<>"
    );
    (*pairs[i])[9].checkContext(
      4900 + i, "<>"
    );
    (*pairs[i])[10].checkContext(
      5000 + i, "<>"
    );
    // ************************************************************************
    serverSequence = (*pairs[i])[1].apply(
      "[reactor:core:temp:[60+level:\"CRITICAL\"]+reactor:core:pressure:70]",
      (const CompoundDimension*)NULL, flags
    );
    (*pairs[i])[1].synch();
    (*pairs[i])[1].checkContext(
      5100 + i,
      "<reactor:<core:<pressure:<70>+temp:<60+level:<\"CRITICAL\">>>>>"
    );
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(
      5200 + i,
      "<reactor:<core:<pressure:<70>+temp:<60+level:<\"CRITICAL\">>>>>"
    );
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(
      5300 + i, "<core:<pressure:<70>+temp:<60+level:<\"CRITICAL\">>>>"
    );
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(
      5400 + i, "<pressure:<70>+temp:<60+level:<\"CRITICAL\">>>"
    );
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(
      5500 + i, "<60+level:<\"CRITICAL\">>"
    );
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(
      5600 + i, "<60+level:<\"CRITICAL\">>"
    );
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(
      5700 + i, "<70>"
    );
    (*pairs[i])[8].checkContext(
      5800 + i, "<>"
    );
    (*pairs[i])[9].checkContext(
      5900 + i, "<>"
    );
    (*pairs[i])[10].checkContext(
      6000 + i, "<>"
    );
  }
  for (int i = 0; i < pairs.size(); i++) {
    serverSequence = (*pairs[i])[1].assign(
      "<reactor:core:temp:10>", (const CompoundDimension*)NULL, flags
    );
    (*pairs[i])[1].synch();
    (*pairs[i])[1].checkContext(7100 + i, "<reactor:<core:<temp:<10>>>>");
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(7200 + i, "<reactor:<core:<temp:<10>>>>");
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(7300 + i, "<core:<temp:<10>>>");
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(7400 + i, "<temp:<10>>");
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(7500 + i, "<10>");
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(7600 + i, "<10>");
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(7700 + i, "<>");
    (*pairs[i])[8].checkContext(7800 + i, "<>");
    (*pairs[i])[9].checkContext(7900 + i, "<>");
    (*pairs[i])[10].checkContext(8000 + i, "<>");
    // ************************************************************************
    serverSequence = (*pairs[i])[1].assign(
      "<core:temp:20>", "reactor", flags
    );
    (*pairs[i])[1].synch();
    (*pairs[i])[1].checkContext(8100 + i, "<reactor:<core:<temp:<20>>>>");
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(8200 + i, "<reactor:<core:<temp:<20>>>>");
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(8300 + i, "<core:<temp:<20>>>");
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(8400 + i, "<temp:<20>>");
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(8500 + i, "<20>");
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(8600 + i, "<20>");
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(8700 + i, "<>");
    (*pairs[i])[8].checkContext(8800 + i, "<>");
    (*pairs[i])[9].checkContext(8900 + i, "<>");
    (*pairs[i])[10].checkContext(9000 + i, "<>");
    // ************************************************************************
    serverSequence = (*pairs[i])[1].assign(
      "<30>", "reactor:core:temp", flags
    );
    (*pairs[i])[1].synch();
    (*pairs[i])[1].checkContext(10100 + i, "<reactor:<core:<temp:<30>>>>");
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(10200 + i, "<reactor:<core:<temp:<30>>>>");
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(10300 + i, "<core:<temp:<30>>>");
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(10400 + i, "<temp:<30>>");
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(10500 + i, "<30>");
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(10600 + i, "<30>");
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(10700 + i, "<>");
    (*pairs[i])[8].checkContext(10800 + i, "<>");
    (*pairs[i])[9].checkContext(10900 + i, "<>");
    (*pairs[i])[10].checkContext(11000 + i, "<>");
    // ************************************************************************
    serverSequence = (*pairs[i])[5].assign(
      "<40>", (const CompoundDimension*)NULL, flags
    );
    (*pairs[i])[5].synch();
    (*pairs[i])[1].synch(serverSequence);
    (*pairs[i])[1].checkContext(11100 + i, "<reactor:<core:<temp:<40>>>>");
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(11200 + i, "<reactor:<core:<temp:<40>>>>");
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(11300 + i, "<core:<temp:<40>>>");
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(11400 + i, "<temp:<40>>");
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(11500 + i, "<40>");
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(11600 + i, "<40>");
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(11700 + i, "<>");
    (*pairs[i])[8].checkContext(11800 + i, "<>");
    (*pairs[i])[9].checkContext(11900 + i, "<>");
    (*pairs[i])[10].checkContext(12000 + i, "<>");
    // ************************************************************************
    serverSequence = (*pairs[i])[7].assign(
      "<50>", (const CompoundDimension*)NULL, flags
    );
    (*pairs[i])[7].synch();
    (*pairs[i])[1].synch(serverSequence);
    (*pairs[i])[1].checkContext(
      12100 + i, "<reactor:<core:<pressure:<50>+temp:<40>>>>"
    );
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(
      12200 + i, "<reactor:<core:<pressure:<50>+temp:<40>>>>"
    );
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(
      12300 + i, "<core:<pressure:<50>+temp:<40>>>"
    );
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(
      12400 + i, "<pressure:<50>+temp:<40>>"
    );
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(
      12500 + i, "<40>"
    );
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(
      12600 + i, "<40>"
    );
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(
      12700 + i, "<50>"
    );
    (*pairs[i])[8].checkContext(
      12800 + i, "<>"
    );
    (*pairs[i])[9].checkContext(
      12900 + i, "<>"
    );
    (*pairs[i])[10].checkContext(
      13000 + i, "<>"
    );
    // ************************************************************************
    serverSequence = (*pairs[i])[1].assign(
      "<reactor:core:temp:<60+level:\"CRITICAL\">+reactor:core:pressure:70>",
      (const CompoundDimension*)NULL, flags
    );
    (*pairs[i])[1].synch();
    (*pairs[i])[1].checkContext(
      13100 + i,
      "<reactor:<core:<pressure:<70>+temp:<60+level:<\"CRITICAL\">>>>>"
    );
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(
      13200 + i,
      "<reactor:<core:<pressure:<70>+temp:<60+level:<\"CRITICAL\">>>>>"
    );
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(
      13300 + i, "<core:<pressure:<70>+temp:<60+level:<\"CRITICAL\">>>>"
    );
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(
      13400 + i, "<pressure:<70>+temp:<60+level:<\"CRITICAL\">>>"
    );
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(
      13500 + i, "<60+level:<\"CRITICAL\">>"
    );
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(
      13600 + i, "<60+level:<\"CRITICAL\">>"
    );
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(
      13700 + i, "<70>"
    );
    (*pairs[i])[8].checkContext(
      13800 + i, "<>"
    );
    (*pairs[i])[9].checkContext(
      13900 + i, "<>"
    );
    (*pairs[i])[10].checkContext(
      14000 + i, "<>"
    );
  }
  for (int i = 0; i < pairs.size(); i++) {
    (*pairs[i])[1].clear("reactor:core", flags);
    (*pairs[i])[1].synch();
    (*pairs[i])[1].checkContext(14100 + i, "<>");
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(14200 + i, "<>");
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(14300 + i, "<>");
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(14400 + i, "<>");
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(14500 + i, "<>");
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(14600 + i, "<>");
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(14700 + i, "<>");
    (*pairs[i])[8].checkContext(14800 + i, "<>");
    (*pairs[i])[9].checkContext(14900 + i, "<>");
    (*pairs[i])[10].checkContext(15000 + i, "<>");
    serverSequence = (*pairs[i])[1].assign(
      "<reactor:core:temp:<80>+reactor:core:pressure:90>",
      (const CompoundDimension*)NULL, flags
    );
    (*pairs[i])[1].synch();
    (*pairs[i])[1].checkContext(
      15100 + i,
      "<reactor:<core:<pressure:<90>+temp:<80>>>>"
    );
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(
      15200 + i,
      "<reactor:<core:<pressure:<90>+temp:<80>>>>"
    );
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(
      15300 + i, "<core:<pressure:<90>+temp:<80>>>"
    );
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(
      15400 + i, "<pressure:<90>+temp:<80>>"
    );
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(
      15500 + i, "<80>"
    );
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(
      15600 + i, "<80>"
    );
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(
      15700 + i, "<90>"
    );
    (*pairs[i])[8].checkContext(
      15800 + i, "<>"
    );
    (*pairs[i])[9].checkContext(
      15900 + i, "<>"
    );
    (*pairs[i])[10].checkContext(
      16000 + i, "<>"
    );
    // ************************************************************************
    serverSequence = (*pairs[i])[5].clear(
      (const CompoundDimension*)NULL, flags
    );
    (*pairs[i])[5].synch();
    (*pairs[i])[1].synch(serverSequence);
    (*pairs[i])[1].checkContext(
      16100 + i,
      "<reactor:<core:<pressure:<90>>>>"
    );
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(
      16200 + i,
      "<reactor:<core:<pressure:<90>>>>"
    );
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(
      16300 + i, "<core:<pressure:<90>>>"
    );
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(
      16400 + i, "<pressure:<90>>"
    );
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(
      16500 + i, "<>"
    );
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(
      16600 + i, "<>"
    );
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(
      16700 + i, "<90>"
    );
    (*pairs[i])[8].checkContext(
      16800 + i, "<>"
    );
    (*pairs[i])[9].checkContext(
      16900 + i, "<>"
    );
    (*pairs[i])[10].checkContext(
      17000 + i, "<>"
    );
    // ************************************************************************
    // Now a (non-deterministic / race-dependent) test of accumulation:
    // ************************************************************************
    serverSequence = (*pairs[i])[1].assign(
      "<reactor:core:temp:<60>+reactor:core:pressure:70>",
      (const CompoundDimension*)NULL, flags
    );
    dimension.parse("reactor:core");
    serverSequence = (*pairs[i])[1].apply(
      "[temp:1+pressure:11]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:2+pressure:12]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:3+pressure:13]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:4+pressure:14]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:5+pressure:15]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:6+pressure:16]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:7+pressure:17]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:8+pressure:18]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:9+pressure:19]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:10+pressure:20]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:11+pressure:21]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:12+pressure:22]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:13+pressure:23]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:14+pressure:24]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:15+pressure:25]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:16+pressure:26]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:17+pressure:27]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:18+pressure:28]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:19+pressure:29]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:20+pressure:30]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:21+pressure:31]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:22+pressure:32]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:23+pressure:33]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:24+pressure:34]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:25+pressure:35]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:26+pressure:36]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:27+pressure:37]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:28+pressure:38]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:29+pressure:39]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:30+pressure:40]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:31+pressure:41]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:32+pressure:42]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:33+pressure:43]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:34+pressure:44]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:35+pressure:45]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:36+pressure:46]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:37+pressure:47]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:38+pressure:48]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:39+pressure:49]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:40+pressure:50]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:41+pressure:51]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:42+pressure:52]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:43+pressure:53]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:44+pressure:54]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:45+pressure:55]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:46+pressure:56]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:47+pressure:57]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:48+pressure:58]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:49+pressure:59]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:50+pressure:60]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:51+pressure:61]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:52+pressure:62]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:53+pressure:63]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:54+pressure:64]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:55+pressure:65]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:56+pressure:66]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:57+pressure:67]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:58+pressure:68]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:59+pressure:69]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:60+pressure:70]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:61+pressure:71]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:62+pressure:72]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:63+pressure:73]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:64+pressure:74]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:65+pressure:75]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:66+pressure:76]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:67+pressure:77]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:68+pressure:78]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:69+pressure:79]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:70+pressure:80]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:71+pressure:81]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:72+pressure:82]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:73+pressure:83]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:74+pressure:84]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:75+pressure:85]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:76+pressure:86]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:77+pressure:87]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:78+pressure:88]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:79+pressure:89]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:80+pressure:90]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:81+pressure:91]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:82+pressure:92]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:83+pressure:93]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:84+pressure:94]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:85+pressure:95]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:86+pressure:96]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:87+pressure:97]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:88+pressure:98]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:89+pressure:99]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:90+pressure:100]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:91+pressure:101]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:92+pressure:102]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:93+pressure:103]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:94+pressure:104]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:95+pressure:105]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].clear(&dimension, flags);
    serverSequence = (*pairs[i])[1].apply(
      "[temp:96+pressure:106]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:97+pressure:107]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:98+pressure:108]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:99+pressure:109]", &dimension, flags
    );
    serverSequence = (*pairs[i])[1].apply(
      "[temp:100+pressure:110]", &dimension, flags
    );

    (*pairs[i])[1].synch();
    (*pairs[i])[1].checkContext(
      18100 + i,
      "<reactor:<core:<pressure:<110>+temp:<100>>>>"
    );
    (*pairs[i])[2].synch(serverSequence);
    (*pairs[i])[2].checkContext(
      18200 + i,
      "<reactor:<core:<pressure:<110>+temp:<100>>>>"
    );
    (*pairs[i])[3].synch(serverSequence);
    (*pairs[i])[3].checkContext(
      18300 + i, "<core:<pressure:<110>+temp:<100>>>"
    );
    (*pairs[i])[4].synch(serverSequence);
    (*pairs[i])[4].checkContext(
      18400 + i, "<pressure:<110>+temp:<100>>"
    );
    (*pairs[i])[5].synch(serverSequence);
    (*pairs[i])[5].checkContext(
      18500 + i, "<100>"
    );
    (*pairs[i])[6].synch(serverSequence);
    (*pairs[i])[6].checkContext(
      18600 + i, "<100>"
    );
    (*pairs[i])[7].synch(serverSequence);
    (*pairs[i])[7].checkContext(
      181100 + i, "<110>"
    );
    (*pairs[i])[8].checkContext(
      18800 + i, "<>"
    );
    (*pairs[i])[9].checkContext(
      18900 + i, "<>"
    );
    (*pairs[i])[10].checkContext(
      19000 + i, "<>"
    );

  }
  for (int i = 0; i < pairs.size(); i++) {
    // Leave() the even-numbered participants only, before the client (and all
    // participants with it) gets deleted in ~ClientServerPair:
    for (int j = 1; j <= 5; j++) {
      (*pairs[i])[j*2].leave();
    }
  }
  if (log != NULL) {
    *log << "Finished aepTest." << ENDLOG;
  }
}


void IntenseAEPTest::usage
()
{
  cerr << "\nusage:\n\n" << execName
       << " [-m (inf. loop)] [-n<n iterations>] [-h (help)]\n\n";
}
