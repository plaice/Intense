// ****************************************************************************
//
// AEPSharedClient.cpp : AEP 2.0 shared memory or threaded client.
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


// Bogus for now - no way to attach, post-construction:
AEPSharedClient::AEPSharedClient
(Log* log, Log::Priority logLevel, bool errorTolerant, bool useReceiverThread,
 int maxReceiveSize)
  : AEPClient(
      log, logLevel, errorTolerant, useReceiverThread, maxReceiveSize
    ),
    server(NULL), incoming()
{
  pthread_cond_init(&receptionCondition, NULL);
}


AEPSharedClient::AEPSharedClient
(AEPSharedServer& server_, Log* log, Log::Priority logLevel,
 bool errorTolerant, bool useReceiverThread, int maxReceiveSize)
  : AEPClient(
      log, logLevel, errorTolerant, useReceiverThread, maxReceiveSize
    ),
    server(&server_), incoming()
{
  pthread_cond_init(&receptionCondition, NULL);
  attach(*server);
}


void AEPSharedClient::attach
(AEPSharedServer& server)
{
  server.lock();
  try {
    if (server.client != NULL) {

      throw AEPException(
        "Server already attached to client in AEPSharedClient::attach"
      );
    }
    server.client = this;
    server.unlock();
  } catch (...) {
    server.unlock();
  }
}


AEPSharedClient::~AEPSharedClient
()
{
  lock();
  if (server != NULL) {
    server->client = NULL;
    server = NULL;
  }
  unlock();
  pthread_cond_destroy(&receptionCondition);
}


AEPSharedClient* AEPSharedClient::connect
(AEPSharedServer& server, Log* log, Log::Priority logLevel,
 bool errorTolerant, bool useReceiverThread, int maxReceiveSize)
{
  AEPSharedClient* client = new AEPSharedClient(
    server, log, logLevel, errorTolerant, useReceiverThread, maxReceiveSize
  );

  client->start();
  return client;
}


void AEPSharedClient::send
(AEPCommon::Token& token)
{
  if (server == NULL) {
    throw AEPException(
      "INTERNAL ERROR: NULL server in AEPSharedClient::send", Log::FATAL
    );
  }
  server->append((AEPServer::Token&)token);
}


void AEPSharedClient::append
(Token& token)
{
  pthread_mutex_lock(&receptionMutex);
  incoming.push(&token);
  pthread_cond_signal(&receptionCondition);
  pthread_mutex_unlock(&receptionMutex);
}


AEPClient::Token* AEPSharedClient::receive
()
{
  Token* token;

  if (incoming.size() == 0) {
    throw AEPException(
      "INTERNAL ERROR: incoming queue empty in AEPSharedClient::receive",
      Log::FATAL
    );
  }
  token = incoming.front();
  incoming.pop();
  return token;
}


bool AEPSharedClient::blockForAvailableData
()
{
  pthread_mutex_lock(&receptionMutex);
  while (incoming.size() == 0) {
    pthread_cond_wait(&receptionCondition, &receptionMutex);
    if (stopped) {
      return true;
    }
  }
  return true;
}
