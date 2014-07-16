// ****************************************************************************
//
// AEPCommon.cpp : AEP 2.0 code common to AEPServer and AEPClient.
//
// Copyright 2001, 2002, 2004 Paul Swoboda.
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


#include <sstream>
#include <intense/container_stringtok.hpp>
#include <intense.hpp>
#include "AEPCommon.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;
using namespace intense::aep;


// This implementation follows a much looser appropach than that of AEP 1.x,
// which was non-multiplexed and stateful.  Here, we have only to keep track
// of a map of specialised participants.


const char* AEPCommon::modeTypeStrings[] = {
  "BINARY", "XDR", "TEXT"
};


void AEPCommon::Token::print
(Log& log, Log::Priority logLevel) const
{
  ostringstream oss;

  print(oss, logLevel);
  log << oss.str();
}


AEPCommon::AEPCommon
(Log* log_, Log::Priority logLevel_, bool errorTolerant_)
  : participants(), nextParticipantId(1), log(log_),
    logLevel(logLevel_), errorTolerant(errorTolerant_)
{
  pthread_mutex_init(&mutex, NULL);
  pthread_mutex_init(&sendMutex, NULL);
  pthread_cond_init(&condition, NULL);
  pthread_mutex_init(&receptionMutex, NULL);
}


AEPCommon::~AEPCommon
()
{
  pthread_mutex_destroy(&mutex);
  pthread_mutex_destroy(&sendMutex);
  pthread_cond_destroy(&condition);
  pthread_mutex_destroy(&receptionMutex);
}


long long AEPCommon::addParticipant
(Participant& participant)
{
  long long newId = nextParticipantId++;

  participants[newId] = &participant;
  return participant.id = newId;
}


void AEPCommon::addParticipantWithId
(Participant& participant, const char* location)
{
  pair<map<long long, Participant*>::iterator, bool> insertRC =
    participants.insert(map<long long, Participant*>::value_type(
      participant.id, map<long long, Participant*>::mapped_type()
    ));

  if (!insertRC.second) {
    ostringstream oss;

    oss << "Participant with id " << participant.id
        << " already exists in " << location;
    throw AEPException(oss.str(), Log::ERROR);
  }
  insertRC.first->second = &participant;
}


AEPCommon::Participant& AEPCommon::getParticipant
(long long participantId, const char* location)
{
  map<long long, Participant*>::iterator participantItr =
    participants.find(participantId);

  if (participantItr == participants.end()) {
    ostringstream oss;

    oss << "Could not find participant with id " << participantId
        << " in " << location;
    throw AEPException(oss.str(), Log::ERROR);
  }
  return *(participantItr->second);
}


void AEPCommon::removeParticipant
(Participant& participant, const char* location)
{
  map<long long, Participant*>::iterator participantItr =
    participants.find(participant.id);

  if (participantItr == participants.end()) {
    ostringstream oss;

    oss << "Could not find participant with id " << participant.id
        << " in " << location;
    throw AEPException(oss.str(), Log::ERROR);
  }
  participantItr->second->id = -1;
  participants.erase(participantItr);
}


void AEPCommon::removeAllParticipants
(bool deleteParticipants)
{
  map<long long, Participant*>::iterator participantItr;

  for (participantItr = participants.begin();
       participantItr != participants.end(); participantItr++) {
    if (deleteParticipants) {
      delete participantItr->second;
    } else {
      participantItr->second->id = -1;
    }
  }
  participants.clear();
}


void AEPCommon::lock
()
{
  pthread_mutex_lock(&mutex);
}


void AEPCommon::unlock
()
{
  pthread_mutex_unlock(&mutex);
}


void AEPCommon::sendLock
()
{
  pthread_mutex_lock(&sendMutex);
}


void AEPCommon::sendUnlock
()
{
  pthread_mutex_unlock(&sendMutex);
}


void AEPCommon::wait
(pthread_cond_t* condition_)
{
  pthread_cond_wait(condition_, &mutex);
}


void AEPCommon::wait
()
{
  pthread_cond_wait(&condition, &mutex);
}


void AEPCommon::signal
()
{
  pthread_cond_signal(&condition);
}


// This _really_ should use GNU regex and do a proper match with groups, but it
// was already implemented as such.
void AEPCommon::parseURL
(const char* url, Mode& mode, string& host, int& port,
 CompoundDimension& dimension)
{
  string product = url, portString, dimensionString;
  unsigned int n;
  char* endPtr;

  if (product.substr(0, 6) == "aep://") {
    mode = BINARY;
    product.erase(0, 6);
  } else if (product.substr(0, 7) == "aetp://") {
    mode = TEXT;
    product.erase(0, 7);
  } else if (product.substr(0, 7) == "aebp://") {
    mode = BINARY;
    product.erase(0, 7);
  } else if (product.substr(0, 7) == "aexp://") {
    mode = XDR;
    product.erase(0, 7);
  } else {
    throw AEPException(
      "AEP URLs must start with the prefix \"aep://\", \"aetp://\", "
      "\"aebp://\", or \"aexp://\"."
    );
  }
  if ((n = product.find('/')) != string::npos) {
    // The dimension is everything after the first /:
    if (product.substr(n + 1).length() > 0) {
      dimensionString = product.substr(n + 1);
    } else {
      dimensionString = "";
    }
    // Remove everything from the first '/':
    product.erase(n);
  } else {
    dimensionString = "";
  }
  // Now look for a ':' and if it's there, divide the host and port:
  if ((n = product.find(':')) != string::npos) {
    portString = product.substr(n + 1);
    if (portString.length() <= 0) {
      ostringstream oss;

      oss << "bad port string \"" << portString << "\" in AEP URL";
      throw AEPException(oss.str());
    }
    port = strtol(portString.data(), &endPtr, 10);
    if (*endPtr != 0) {
      throw AEPException("Non-int port string in AEP URL");
    }
    product.erase(n);
  } else {
    port = AEP_DEFAULT_PORT;
  }
  if (product.length() <= 0) {
    throw AEPException("bad host in AEP URL");
  }
  host = product;
  dimension.clear();
  dimension.parse(dimensionString.data());
}


// Construct an AEP URL, with a possibly-NULL dimension.
string AEPCommon::constructURL
(Mode& mode, std::string& host, int port, CompoundDimension* dimension)
{
  ostringstream oss;

  switch (mode) {
  case BINARY:
    oss << "aebp://";
    break;
  case XDR:
    oss << "aexp://";
    break;
  case TEXT:
    oss << "aetp://";
    break;
  }
  oss << host << ':' << port;
  if (dimension != NULL) {
    oss << '/' << dimension->canonical();
  }
  return oss.str();
}
