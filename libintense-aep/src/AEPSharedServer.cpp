// ****************************************************************************
//
// AEPSharedServer.cpp : AEP 2.0 shared memory or threaded server.
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


#include <intense.hpp>
#include <queue>
#define yyFlexParser aetp_yyFlexParser
#include <FlexLexer.h>
#include "AETPLexer.hpp"
#include "AEPCommon.hpp"
#include "AEPClient.hpp"
#include "AEPServer.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;
using namespace intense::aep;


AEPSharedServer::AEPSharedServer
(AEther& aether, Log* log, Log::Priority logLevel, bool errorTolerant)
  : AEPServer(aether, log, logLevel, errorTolerant), client(NULL), incoming()
{
  pthread_cond_init(&receptionCondition, NULL);
}


AEPSharedServer::~AEPSharedServer
()
{
  lock();
  if (client != NULL) {
    client->server = NULL;
    client = NULL;
  }
  unlock();
  while (incoming.size() > 0) {
    delete incoming.front();
    incoming.pop();
  }
  pthread_cond_destroy(&receptionCondition);
}


void AEPSharedServer::send
(AEPCommon::Token& token)
{
  if (client == NULL) {
    throw AEPException(
      "INTERNAL ERROR: NULL client in AEPSharedServer::send", Log::FATAL
    );
  }
  client->append((AEPClient::Token&)token);
}


void AEPSharedServer::append
(Token& token)
{
  pthread_mutex_lock(&receptionMutex);
  incoming.push(&token);
  pthread_cond_signal(&receptionCondition);
  pthread_mutex_unlock(&receptionMutex);
}


AEPServer::Token* AEPSharedServer::receive
()
{
  Token* token;

  if (incoming.size() == 0) {
    throw AEPException(
      "INTERNAL ERROR: incoming queue empty in AEPSharedServer::receive",
      Log::FATAL
    );
  }
  token = incoming.front();
  incoming.pop();
  return token;
}


bool AEPSharedServer::blockForAvailableData
()
{
  pthread_mutex_lock(&receptionMutex);
  while (incoming.size() == 0) {
    pthread_cond_wait(&receptionCondition, &receptionMutex);
  }
  return true;
}


const char* AEPSharedServer::getName
()
{
  ostringstream oss;

  oss << "AEPSharedServer:" << this;
  return oss.str().data();
}
