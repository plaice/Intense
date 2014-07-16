// ****************************************************************************
//
// AEPServer.cpp : AEP 2.0 server, for the abstract/core AEP implementation.
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


#include <intense/container_stringtok.hpp>
#include <intense.hpp>
#include <queue>
#include "AEPCommon.hpp"
#include "AEPClient.hpp"
#include "AEPServer.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;
using namespace intense::aep;


// This implementation follows a much looser appropach than that of AEP 1.x,
// which was non-multiplexed and stateful.  Here, we have only to keep track
// of a vector of specialised participants.


const char* AEPServer::Token::typeStrings[] = {
  "VOID", "SYNCH", "CLIENT_DISCONNECT", "JOIN", "LEAVE", "ASSIGN", "APPLY",
  "CLEAR", "KICK"
};


AEPServer::AEther::AEther
(int maxAccumulatedCount_, Log* log_, Log::Priority logLevel_)
  : intense::AEther(), serverSequence(0), running(false), incoming(),
    origin(*this), accumulated(NULL), accumulatedCount(0),
    maxAccumulatedCount(maxAccumulatedCount_), log(log_), logLevel(logLevel_)
{
  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&condition, NULL);
  // start thread:
  pthread_mutex_lock(&mutex);
  if (pthread_create(&thread, NULL, main, (void*)this) != 0) {
    throw runtime_error("Could not create AEther thread");
  }
  running = true;
  pthread_cond_wait(&condition, &mutex);
  // Thread is now waiting on condition.
  pthread_mutex_unlock(&mutex);
}


AEPServer::AEther::~AEther
()
{
  // stop thread:
  pthread_mutex_lock(&mutex);
  running = false;
  pthread_cond_signal(&condition);
  pthread_mutex_unlock(&mutex);
  pthread_join(thread, NULL);
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&condition);
}


void AEPServer::AEther::cleanupHandler
(void* aetherAsVoidPointer)
{
  AEther* aether = (AEther*)aetherAsVoidPointer;

  if ((aether->log != NULL)&&(aether->logLevel >= Log::ERROR)) {
    *(aether->log) << Log::NOTICE << "AEther thread " << pthread_self()
                   << ": cancelled (THIS SHOULD NOT HAPPEN!!)" << ENDLOG;
  }
}


void* AEPServer::AEther::main
(void* aetherAsVoidPointer)
{
  AEther* aether = (AEther*)aetherAsVoidPointer;

  pthread_cleanup_push(cleanupHandler, aetherAsVoidPointer);
  if ((aether->log != NULL)&&(aether->logLevel >= Log::NOTICE)) {
    *(aether->log) << Log::NOTICE << "AEther ear thread " << pthread_self()
                   << ": starting execution..." << ENDLOG;
  }
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  try {
    aether->lock();
    // Signal constructor:
    aether->signal();
    while (aether->running) {
      if (aether->incoming.size() > 0) {
        AEPServer::Token* token = aether->incoming.front();
        aether->incoming.pop();
        aether->unlock();
        aether->processToken(*token);
        aether->lock();
      } else {
        // Before waiting for more tokens, always apply any outstanding
        // accumulated asynchronous tokens:
        if ((aether->log != NULL)&&(aether->logLevel >= Log::DEBUG)) {
          *(aether->log)
            << Log::DEBUG << "AEther thread: applying outstanding "
            << "tokens and suspending..." << ENDLOG;
        }
        aether->applyAccumulated();
        pthread_cond_wait(&(aether->condition), &(aether->mutex));
      }
    }
    // Destructor is now waiting to join() this thread.
  } catch (exception& e) {
    if ((aether->log != NULL)&&(aether->logLevel >= Log::ERROR)) {
      *(aether->log)
        << Log::ERROR << "AEther thread caught exception in main: " << e.what()
        << ENDLOG;
    }
  } catch (...) {
    if ((aether->log != NULL)&&(aether->logLevel >= Log::ERROR)) {
      *(aether->log)
        << Log::ERROR << "AEther thread caught unknown exception in main"
        << ENDLOG;
    }
  }
  aether->unlock();
  if ((aether->log != NULL)&&(aether->logLevel >= Log::NOTICE)) {
    *(aether->log) << Log::NOTICE << "AEther thread " << pthread_self()
                   << ": END OF LINE." << ENDLOG;
  }
  pthread_cleanup_pop(0);
  return NULL;
}


void AEPServer::AEther::append
(AEPServer::Token& token)
{
  incoming.push(&token);
}


void AEPServer::AEther::wait
(pthread_cond_t& waitCondition)
{
  pthread_cond_wait(&waitCondition, &mutex);
}


void AEPServer::AEther::signal
()
{
  pthread_cond_signal(&condition);
}


void AEPServer::AEther::accumulate
(AEPServer::AsynchronousToken& token)
{
  int flags = token.flags;

  if (flags&AEPServer::preFenceBit) {
    if ((log != NULL)&&(logLevel >= Log::DEBUG)) {
      *log << Log::DEBUG << "AEther thread " << pthread_self()
           << ": preFenceBit set in token" << ENDLOG;
    }
    applyAccumulated();
    accumulated = &token;
  } else if (accumulated != NULL) {
    //    AsynchronousToken* accumulationResult = NULL;
    AsynchronousToken* accumulationResult = accumulated->accumulate(token);

    if (accumulationResult == accumulated) {
      AsynchronousToken::decrementRefCount(token);
    } else if (accumulationResult == &token) {
      AsynchronousToken::decrementRefCount(*accumulated);
      accumulated = accumulationResult;
    } else if (accumulationResult != NULL) {
      AsynchronousToken::decrementRefCount(*accumulated);
      AsynchronousToken::decrementRefCount(token);
      accumulated = accumulationResult;
    } else {
      applyAccumulated();
      accumulated = &token;
    }
  } else {
    accumulated = &token;
  }
  accumulatedCount++;
  if ((flags&AEPServer::postFenceBit)||
      (accumulatedCount > maxAccumulatedCount)) {
    if ((log != NULL)&&(logLevel >= Log::DEBUG)) {
      if (flags&AEPServer::postFenceBit) {
        *log << Log::DEBUG << "AEther thread " << pthread_self()
             << ": postFenceBit set in token" << ENDLOG;
      }
      if (accumulatedCount > maxAccumulatedCount) {
        *log << Log::DEBUG << "AEther thread " << pthread_self()
             << ": maximum accumulated count (" << maxAccumulatedCount
             << ") reached." << ENDLOG;
      }
    }
    applyAccumulated();
  }
}


void AEPServer::AEther::applyAccumulated
()
{
  if (accumulated != NULL) {
    if ((log != NULL)&&(logLevel >= Log::DEBUG)) {
      *log << Log::DEBUG << "AEther thread " << pthread_self()
           << ": applying accumulated asynchronous token: ";
      accumulated->print(*log, Log::DEBUG);
      *log << ENDLOG;
    }
    accumulated->aetherHandle(*this);
    AsynchronousToken::decrementRefCount(*accumulated);
    accumulated = NULL;
    accumulatedCount = 0;
  } else {
    if ((log != NULL)&&(logLevel >= Log::DEBUG)) {
      *log << Log::DEBUG << "AEther thread " << pthread_self()
           << ": no accumulated asynchronous operations to apply." << ENDLOG;
    }
  }
}


void AEPServer::AEther::lock
()
{
  pthread_mutex_lock(&mutex);
}


void AEPServer::AEther::unlock
()
{
  pthread_mutex_unlock(&mutex);
}


void AEPServer::AEther::processToken
(AEPServer::Token& token)
{
  long long serverSequence = token.getServerSequence();
  long long clientSequence = token.getClientSequence();

  if ((log != NULL)&&(logLevel >= Log::DEBUG)) {
    *log << Log::DEBUG << "AEPServer::AEther: Received token: ";
    token.print(*log, (logLevel > Log::DEBUG) ? logLevel : Log::DEBUG);
    *log << ENDLOG;
  }
  try {
    switch (token.getType()) {
    case AEPServer::Token::VOID:
      throw AEPException(
        "INTERNAL ERROR: Received VOID AEPServer::Token in "
        "AEPServer::AEther processing of AEPServer::Token", Log::FATAL
      );
    case AEPServer::Token::SYNCH:
    case AEPServer::Token::CLIENT_DISCONNECT:
    case AEPServer::Token::JOIN:
    case AEPServer::Token::LEAVE:
    case AEPServer::Token::KICK:
      // Synchronous tokens are implicitly associative pre-fences:
      applyAccumulated();
      token.aetherHandle(*this);
      ((SynchronousToken&)token).resumeServer();
      // AEPServer deletes synchronous tokens:
      break;
    case AEPServer::Token::ASSIGN:
    case AEPServer::Token::APPLY:
    case AEPServer::Token::CLEAR:
      // AEPServer::AEther must decrement the reference count of (and possibly
      // delete) asynchronous tokens, under accumulate somewhere:
      accumulate(((AsynchronousToken&)token));
      break;
    default:
      {
        ostringstream oss;

        oss << "INTERNAL ERROR: Got bad AEPServer::Token type int in "
            << "AEPServer::AEther processing of AEPServer::Token: "
            << token.getType();
        throw AEPException(oss.str(), Log::FATAL);
      }
    }
  } catch (AEPException& e) {
    if ((log != NULL)&&(logLevel >= e.getPriority())) {
      *log << e.getPriority() << "Caught AEPException in AEPServer::AEther "
           << "processing of AEPServer::Token: " << e.what() << ENDLOG;
    }
  } catch (IntenseException& e) {
    if ((log != NULL)&&(logLevel >= Log::CRITICAL)) {
      *log << Log::CRITICAL
           << "Caught IntenseException in AEPServer::AEther processing of "
           << "AEPServer::Token: " << e.what() << ENDLOG;
    }
  } catch (exception& e) {
    if ((log != NULL)&&(logLevel >= Log::CRITICAL)) {
      *log << Log::CRITICAL
           << "Caught std::exception in AEPServer::AEther processing of "
           << "AEPServer::Token: " << e.what() << ENDLOG;
    }
  } catch (...) {
    if ((log != NULL)&&(logLevel >= Log::CRITICAL)) {
      *log << Log::CRITICAL
           << "Caught unknown object in AEPServer::AEther processing of "
           << "AEPServer::Token" << ENDLOG;
    }
  }
}


void AEPServer::AEther::Origin::beginOperation
(AEPServer::AsynchronousToken& sourceToken_)
{
  serverMap.clear();
  sourceToken = &sourceToken_;
}


void AEPServer::AEther::Origin::clear
()
{
  serverMap.clear();
}


void AEPServer::AEther::Origin::execute
()
{
  map<AEPServer*, AEther*>::iterator itr;

  for (itr = serverMap.begin(); itr != serverMap.end(); itr++) {
    AEPClient::NotifyToken& token =
      (AEPClient::NotifyToken&)(itr->first->getCurrentNotifyToken());

    try {
      itr->first->lockedSend(token);
    } catch (...) {
      token.shallowClear();
      delete &token;
      throw;
    }
    if (itr->first->deletesSentTokens()) {
      token.shallowClear();
      delete &token;
    }
  }
}


void AEPServer::AEther::Origin::addServerMapEntry
(AEPServer* server, AEther* node)
{
  pair<map<AEPServer*, AEther*>::iterator, bool> serverMapInsertRC =
    serverMap.insert(map<AEPServer*, AEther*>::value_type(
      server, map<AEPServer*, AEther*>::mapped_type()
    ));

  if (serverMapInsertRC.second) {
    serverMapInsertRC.first->second = node;
    server->setCurrentNotifyToken(
      new AEPClient::NotifyToken(sourceToken->getServerSequence())
    );
  }
}


AEPServer::AsynchronousToken& AEPServer::AEther::Origin::getSourceToken
()
{
  if (sourceToken == NULL) {
    throw AEPException(
      "INTERNAL ERROR: NULL source token in "
      "AEPServer::AEther::Origin::getSourceToken", Log::FATAL
    );
  } else {
    return *sourceToken;
  }
}


AEPServer::Token* AEPServer::Token::factory
(Type type)
{
  switch (type) {
  case VOID:
    return NULL;
  case SYNCH:
    return new SynchToken();
  case CLIENT_DISCONNECT:
    return new ClientDisconnectToken();
  case JOIN:
    return new JoinToken();
  case LEAVE:
    return new LeaveToken();
  case ASSIGN:
    return new AssignToken(1);
  case APPLY:
    return new ApplyToken(1);
  case CLEAR:
    return new ClearToken();
  default:
    ostringstream oss;

    oss << "Bad AEPServer::Token type " << type;
    throw AEPException(oss.str());
  }
}


void AEPServer::Token::factorySerialise
(const Token* token, BaseSerialiser& baseSerialiser, ostream& os)
{
  int type;

  if (token != NULL) {
    type = ((AEPServer::Token*)token)->getType();
  } else {
    type = AEPServer::Token::VOID;
  }
  baseSerialiser.out(type, os);
  if (token != NULL) {
    ((AEPServer::Token*)token)->serialise(baseSerialiser, os);
  }
}


AEPServer::Token* AEPServer::Token::factoryDeserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  int type;
  AEPServer::Token* token;

  baseSerialiser.in(type, is);
  token = AEPServer::Token::factory((AEPServer::Token::Type)type);
  if (token != NULL) {
    token->deserialise(baseSerialiser, is);
  }
  return token;
}


void AEPServer::SynchToken::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  baseSerialiser.out(clientSequence, os);
}


void AEPServer::SynchToken::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  baseSerialiser.in(clientSequence, is);
}


bool AEPServer::SynchToken::handle
()
{
  static const char* location = "SynchToken::handle";

  aetherTransaction(location);
  return true;
}


void AEPServer::SynchToken::aetherHandle
(AEther& aether)
{
  reply = new AEPClient::AckToken(serverSequence, clientSequence);
}


void AEPServer::SynchToken::print
(ostream& os, Log::Priority logLevel) const
{
  os << "AEPServer::SynchToken (cs: " << clientSequence << ")";
}


void AEPServer::ClientDisconnectToken::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  baseSerialiser.out(clientSequence, os);
}


void AEPServer::ClientDisconnectToken::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  baseSerialiser.in(clientSequence, is);
}


bool AEPServer::ClientDisconnectToken::handle
()
{
  static const char* location = "ClientDisconnectToken::handle";

  // So the destructor doesn't try to cancel us, and just joins us:
  server->lock();
  server->running = false;
  server->unlock();
  aetherTransaction(location);
  return false;
}


void AEPServer::ClientDisconnectToken::aetherHandle
(AEther& aether)
{
  server->leaveAllParticipants();
  if (clientSequence >= 0) {
    reply = new AEPClient::AckToken(serverSequence, clientSequence);
  } else {
    reply = NULL;
  }
}


void AEPServer::ClientDisconnectToken::print
(ostream& os, Log::Priority logLevel) const
{
  os << "AEPServer::ClientDisconnectToken (cs: " << clientSequence << ")";
}


AEPServer::JoinToken::~JoinToken
()
{
  delete dimension;
}


void AEPServer::JoinToken::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  int notifyInt = (notify) ? 1 : 0;

  baseSerialiser.out(clientSequence, os);
  baseSerialiser.out(participantId, os);
  baseSerialiser.out(notifyInt, os);
  baseSerialiser.out(dimension, os);
}


void AEPServer::JoinToken::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  int notifyInt;

  baseSerialiser.in(clientSequence, is);
  baseSerialiser.in(participantId, is);
  baseSerialiser.in(notifyInt, is);
  notify = (notifyInt == 1);
  baseSerialiser.in(dimension, is);
}


bool AEPServer::JoinToken::handle
()
{
  static const char* location = "AEPServer::JoinToken::handle";

  participant = new Participant(*server, participantId);
  try {
    server->addParticipantWithId(*participant, location);
  } catch (AEPException& e) {
    AEPClient::ErrorToken* errorToken = new AEPClient::ErrorToken(
      -1, clientSequence, new string(e.what())
    );
    // Ignore send errors (they are logged):
    try {
      if (server->log != NULL) {
        *(server->log) << "SENDING AN ERROR TOKEN: ";
        errorToken->AEPCommon::Token::print(*(server->log), Log::DEBUG2);
        *(server->log) << ENDLOG;
      }
      server->lockedAndLoggedSend(*errorToken, location, -1, clientSequence);
    } catch (...) {
    }
    if (server->deletesSentTokens()) {
      delete errorToken;
    }
    delete participant;
    throw e;
  }
  try {
    if (!aetherTransaction(location)) {
      server->removeParticipant(*participant, location);
      delete participant;
      participant = NULL;
    }
  } catch (...) {
    server->removeParticipant(*participant, location);
    delete participant;
    participant = NULL;
  }
  return true;
}


void AEPServer::JoinToken::aetherHandle
(AEther& aether)
{
  intense::AEther* targetNode;

  if (dimension == NULL) {
    targetNode = &aether;
    participant->join(*targetNode);
  } else {
    targetNode = &((AEther&)aether[*dimension]);
    participant->join(*targetNode);
    delete participant->dimension;
    participant->dimension = dimension;
    dimension = NULL;
  }
  if (notify) {
    AEPClient::NotifyToken* notifyToken =
      new AEPClient::NotifyToken(serverSequence);
    Context* value = new Context;

    value->assign(*targetNode);
    notifyToken->addNode(*value);
    notifyToken->addTarget(*(new AEPClient::NotifyToken::AssignTarget(
      participantId, 0
    )));
    server->lockedAndLoggedSend(
      *notifyToken, "JoinToken::aetherHandle notification",
      serverSequence, clientSequence
    );
  }
  reply = new AEPClient::AckToken(serverSequence, clientSequence);
}


void AEPServer::JoinToken::print
(ostream& os, Log::Priority logLevel) const
{
  os << "AEPServer::JoinToken (cs: " << clientSequence << ", p:"
     << participantId;
  if (dimension != NULL) {
    os << ", d:" << dimension->canonical();
  }
  os << ')';
}


void AEPServer::LeaveToken::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  baseSerialiser.out(clientSequence, os);
  baseSerialiser.out(participantId, os);
}


void AEPServer::LeaveToken::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  baseSerialiser.in(clientSequence, is);
  baseSerialiser.in(participantId, is);
}


bool AEPServer::LeaveToken::handle
()
{
  AEPClient::AckToken* replyToken;
  static const char* location = "AEPServer::LeaveToken::handle";

  try {
    participant =
      &((Participant&)(server->getParticipant(participantId, location)));
  } catch (AEPException& e) {
    AEPClient::ErrorToken* errorToken = new AEPClient::ErrorToken(
      -1, clientSequence, new string(e.what())
    );

    // Ignore send errors (they are logged):
    try {
      server->lockedAndLoggedSend(*errorToken, location, -1, clientSequence);
    } catch (...) {
    }
    if (server->deletesSentTokens()) {
      delete errorToken;
    }
    throw e;
  }
  if (aetherTransaction(location)) {
    server->removeParticipant(*participant, location);
    delete participant;
  }
  return true;
}


void AEPServer::LeaveToken::aetherHandle
(AEther& aether)
{
  participant->leave();
  reply = new AEPClient::AckToken(serverSequence, clientSequence);
}


void AEPServer::LeaveToken::print
(ostream& os, Log::Priority logLevel) const
{
  os << "AEPServer::LeaveToken (cs: " << clientSequence << ", p:"
     << participantId << ')';
}


AEPServer::AsynchronousToken::AsynchronousToken
(int initialRefCount)
  : Token(-1), dimension(NULL), participant(NULL), targetNode(NULL),
    flags(notifySelfBit&notifyClientBit), refCount(initialRefCount)
{
  pthread_mutex_init(&mutex, NULL);
}


AEPServer::AsynchronousToken::AsynchronousToken
(CompoundDimension* dimension_, long long clientSequence, int flags_,
 int initialRefCount)
  : Token(clientSequence),
    dimension(
      (dimension_ != NULL) ? new CompoundDimension(*dimension_) : NULL
    ),
    participant(NULL), targetNode(NULL), flags(flags_),
    refCount(initialRefCount)
{
  pthread_mutex_init(&mutex, NULL);
}


void AEPServer::AsynchronousToken::lock
()
{
  pthread_mutex_lock(&mutex);
}


void AEPServer::AsynchronousToken::unlock
()
{
  pthread_mutex_unlock(&mutex);
}


AEPServer::AsynchronousToken::~AsynchronousToken
()
{
  pthread_mutex_destroy(&mutex);
  delete dimension;
}


void AEPServer::AsynchronousToken::decrementRefCount
(AsynchronousToken& token)
{
  int refCount;

  token.lock();
  token.refCount--;
  refCount = token.refCount;
  token.unlock();
  if (refCount == 0) {
    delete &token;
  } else if (refCount < 0) {
    ostringstream oss;

    oss << "INTERNAL ERROR: refCount (" << refCount
        << ") < 0 in AEPServer::AsynchronousToken::decrementRefCount";
    throw AEPException(oss.str(), Log::FATAL);
  }
}


void AEPServer::AsynchronousToken::incrementRefCount
(AsynchronousToken& token)
{
  token.lock();
  token.refCount++;
  token.unlock();
}


intense::AEther& AEPServer::AsynchronousToken::getTargetNode
()
{
  if (targetNode == NULL) {
    if (participant != NULL) {
      if (dimension != NULL) {
        targetNode = &(participant->getNode().value(*dimension));
      } else {
        targetNode = &(participant->getNode());
      }
    } else if (server != NULL) {
      if (dimension != NULL) {
        targetNode = &(server->aether->value(*dimension));
      } else {
        targetNode = server->aether;
      }
    } else {
      ostringstream oss;

      oss << "Got an asynch token with a null participant and server: ";
      print(oss, Log::DEBUG2);
      throw AEPException(oss.str());
    }
  }
  return *targetNode;
}


AEPServer::AssignToken::~AssignToken
()
{
  delete context;
}


void AEPServer::AssignToken::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  baseSerialiser.out(clientSequence, os);
  baseSerialiser.out(participantId, os);
  baseSerialiser.out(context, os);
  baseSerialiser.out(dimension, os);
  baseSerialiser.out(flags, os);
}


void AEPServer::AssignToken::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  baseSerialiser.in(clientSequence, is);
  baseSerialiser.in(participantId, is);
  baseSerialiser.in(context, is);
  baseSerialiser.in(dimension, is);
  baseSerialiser.in(flags, is);
}


bool AEPServer::AssignToken::handle
()
{
  static const char* location = "AssignToken::handle";

  if (participantId > 0) {
    try {
      participant =
        &((Participant&)(server->getParticipant(participantId, location)));
    } catch (AEPException& e) {
      AEPClient::ErrorToken* errorToken = new AEPClient::ErrorToken(
        -1, clientSequence, new string(e.what())
      );

      // Ignore send errors (they are logged):
      try {
        server->lockedAndLoggedSend(*errorToken, location, -1, clientSequence);
      } catch (...) {
      }
      if (server->deletesSentTokens()) {
        delete errorToken;
      }
      throw e;
    }
  }
  aetherTransaction(location);
  return true;
}


void AEPServer::AssignToken::aetherHandle
(AEther& aether)
{
  aether.origin.beginOperation(*this);
  getTargetNode().assign(*context, &(aether.origin));
  aether.origin.execute();
}


AEPServer::AsynchronousToken* AEPServer::AssignToken::accumulate
(AsynchronousToken& token)
{
  CompoundDimension path;
  int ancestry = getTargetNode().getAncestry(token.getTargetNode(), &path);

  switch (token.getType()) {
  case ASSIGN:
    {
      AssignToken& assignToken = (AssignToken&)token;

      if (ancestry <= 0) {
        // [path:<c1>]<c2>:
        // <c1><c2>:
        return &token;
      } else {
        // <c1>[path:<c2>]:
        context->value(path).assign(*(assignToken.context));
        serverSequence = assignToken.serverSequence;
        return this;
      }
    }
    break;
  case APPLY:
    {
      ApplyToken& applyToken = (ApplyToken&)token;

      if (ancestry == 0) {
        // <c1>[o2]:
        context->apply(*(applyToken.op));
        serverSequence = applyToken.serverSequence;
        return this;
      } else if (ancestry < 0) {
        // [path:<c1>][o2]:
        ApplyToken* returnValue = new ApplyToken();

        returnValue->op = new ContextOp();
        returnValue->op->value(path).apply(*context);
        returnValue->op->apply(*(applyToken.op));
        returnValue->targetNode = applyToken.targetNode;
        returnValue->serverSequence = applyToken.serverSequence;
        return returnValue;
      } else {
        // <c1>[path:[o2]]:
        context->value(path).apply(*(applyToken.op));
        serverSequence = applyToken.serverSequence;
        return this;
      }
    }
    break;
  case CLEAR:
    {
      ClearToken& clearToken = (ClearToken&)token;

      if (ancestry <= 0) {
        // <c1><>:
        // [path:<c1>]<>:
        return &token;
      } else {
        // <c1>[path:<>]:
        context->value(path).clear();
        return this;
      }
    }
    break;
  default:
    {
      ostringstream oss;

      oss << "Attempt to accumulate token of type " << token.getTypeString()
          << " into AssignToken: ";
      token.print(oss, Log::DEBUG2);
      throw AEPException(oss.str(), Log::FATAL);
    }
  }
}


void AEPServer::AssignToken::print
(ostream& os, Log::Priority logLevel) const
{
  os << "AEPServer::AssignToken (cs: " << clientSequence << ", p:"
     << participantId;
  if (dimension != NULL) {
    os << ", d:" << dimension->canonical();
  }
  if (preFenceBit&flags) {
    os << ", preFence";
  }
  if (postFenceBit&flags) {
    os << ", postFence";
  }
  if (notifySelfBit&flags) {
    os << ", self-notify";
  }
  if (notifyClientBit&flags) {
    os << ", client-notify";
  }
  if (logLevel >= Log::DEBUG1) {
    os << ", context:" << ((AssignToken*)this)->context->canonical();
  }
  os << ')';
}


AEPServer::ApplyToken::~ApplyToken
()
{
  delete op;
}


void AEPServer::ApplyToken::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  baseSerialiser.out(clientSequence, os);
  baseSerialiser.out(participantId, os);
  baseSerialiser.out(op, os);
  baseSerialiser.out(dimension, os);
  baseSerialiser.out(flags, os);
}


void AEPServer::ApplyToken::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  baseSerialiser.in(clientSequence, is);
  baseSerialiser.in(participantId, is);
  baseSerialiser.in(op, is);
  baseSerialiser.in(dimension, is);
  baseSerialiser.in(flags, is);
}


bool AEPServer::ApplyToken::handle
()
{
  static const char* location = "ApplyToken::handle";

  if (participantId > 0) {
    try {
      participant =
        &((Participant&)(server->getParticipant(participantId, location)));
    } catch (AEPException& e) {
      AEPClient::ErrorToken* errorToken = new AEPClient::ErrorToken(
        -1, clientSequence, new string(e.what())
      );

      // Ignore send errors (they are logged):
      try {
        server->lockedAndLoggedSend(*errorToken, location, -1, clientSequence);
      } catch (...) {
      }
      if (server->deletesSentTokens()) {
        delete errorToken;
      }
      throw e;
    }
  }
  aetherTransaction(location);
  return true;
}


void AEPServer::ApplyToken::aetherHandle
(AEther& aether)
{
  aether.origin.beginOperation(*this);
  getTargetNode().apply(*op, &(aether.origin));
  aether.origin.execute();
}


AEPServer::AsynchronousToken* AEPServer::ApplyToken::accumulate
(AsynchronousToken& token)
{
  CompoundDimension path;
  int ancestry = getTargetNode().getAncestry(token.getTargetNode(), &path);

  switch (token.getType()) {
  case ASSIGN:
    {
      AssignToken& assignToken = (AssignToken&)token;

      if (ancestry <= 0) {
        // [path:[o1]]<c2>:
        // [o1]<c2>:
        return &token;
      } else {
        // [o1][path:<c2>]:
        op->value(path).apply(*(assignToken.context));
        serverSequence = assignToken.serverSequence;
        return this;
      }
    }
    break;
  case APPLY:
    {
      ApplyToken& applyToken = (ApplyToken&)token;

      if (ancestry == 0) {
        // [o1][o2]:
        op->apply(*(applyToken.op));
        serverSequence = applyToken.serverSequence;
        return this;
      } else if (ancestry < 0) {
        // [path:[o1]][o2]:
        ApplyToken* returnValue = new ApplyToken();

        returnValue->op = new ContextOp();
        returnValue->op->value(path).apply(*op);
        returnValue->op->apply(*(applyToken.op));
        returnValue->targetNode = applyToken.targetNode;
        returnValue->serverSequence = applyToken.serverSequence;
        return returnValue;
      } else {
        // [o1][path:[o2]]:
        op->value(path).apply(*(applyToken.op));
        serverSequence = applyToken.serverSequence;
        return this;
      }
    }
    break;
  case CLEAR:
    {
      ClearToken& clearToken = (ClearToken&)token;

      if (ancestry <= 0) {
        // [o1]<>:
        // [path:[o1]]<>:
        return &token;
      } else {
        // [o1][path:<>]:
        op->value(path).clear();
        op->value(path).setClearBaseFlag();
        op->value(path).setClearDimsFlag();
        return this;
      }
    }
    break;
  default:
    {
      ostringstream oss;

      oss << "Attempt to accumulate token of type " << token.getTypeString()
          << " into ApplyToken: ";
      token.print(oss, Log::DEBUG2);
      throw AEPException(oss.str(), Log::FATAL);
    }
  }
}


void AEPServer::ApplyToken::print
(ostream& os, Log::Priority logLevel) const
{
  os << "AEPServer::ApplyToken (cs: " << clientSequence << ", p:"
     << participantId;
  if (dimension != NULL) {
    os << ", d:" << dimension->canonical();
  }
  if (preFenceBit&flags) {
    os << ", prefence";
  }
  if (postFenceBit&flags) {
    os << ", postfence";
  }
  if (notifySelfBit&flags) {
    os << ", self-notify";
  }
  if (notifyClientBit&flags) {
    os << ", client-notify";
  }
  if (logLevel >= Log::DEBUG1) {
    os << ", op:" << ((ApplyToken*)this)->op->canonical();
  }
  os << ')';
}


void AEPServer::ClearToken::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  baseSerialiser.out(clientSequence, os);
  baseSerialiser.out(participantId, os);
  baseSerialiser.out(dimension, os);
  baseSerialiser.out(flags, os);
}


void AEPServer::ClearToken::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  baseSerialiser.in(clientSequence, is);
  baseSerialiser.in(participantId, is);
  baseSerialiser.in(dimension, is);
  baseSerialiser.in(flags, is);
}


bool AEPServer::ClearToken::handle
()
{
  static const char* location = "ClearToken::handle";

  if (participantId > 0) {
    try {
      participant =
        &((Participant&)(server->getParticipant(participantId, location)));
    } catch (AEPException& e) {
      AEPClient::ErrorToken* errorToken = new AEPClient::ErrorToken(
        -1, clientSequence, new string(e.what())
      );

      // Ignore send errors (they are logged):
      try {
        server->lockedAndLoggedSend(*errorToken, location, -1, clientSequence);
      } catch (...) {
      }
      if (server->deletesSentTokens()) {
        delete errorToken;
      }
      throw e;
    }
  }
  aetherTransaction(location);
  return true;
}


void AEPServer::ClearToken::aetherHandle
(AEther& aether)
{
  aether.origin.beginOperation(*this);
  getTargetNode().clear(&(aether.origin));
  aether.origin.execute();
}


AEPServer::AsynchronousToken* AEPServer::ClearToken::accumulate
(AsynchronousToken& token)
{
  CompoundDimension path;
  int ancestry = getTargetNode().getAncestry(token.getTargetNode(), &path);

  switch (token.getType()) {
  case ASSIGN:
    {
      AssignToken& assignToken = (AssignToken&)token;

      if (ancestry <= 0) {
        // [path:<>]<c2>:
        // <><c2>:
        return &token;
      } else {
        // <>[path:<c2>]:
        AssignToken* returnValue = new AssignToken();

        returnValue->context = new Context();
        returnValue->context->value(path).assign(*(assignToken.context));
        returnValue->targetNode = assignToken.targetNode;
        returnValue->serverSequence = assignToken.serverSequence;
        return returnValue;
      }
    }
    break;
  case APPLY:
    {
      ApplyToken& applyToken = (ApplyToken&)token;

      if (ancestry == 0) {
        // <>[o2]:
        ApplyToken* returnValue = new ApplyToken();

        returnValue->op = new ContextOp();
        returnValue->op->setClearBaseFlag();
        returnValue->op->setClearDimsFlag();
        returnValue->op->apply(*(applyToken.op));
        returnValue->targetNode = applyToken.targetNode;
        returnValue->serverSequence = applyToken.serverSequence;
        return returnValue;
      } else if (ancestry < 0) {
        // [path:<>][o2]:
        ApplyToken* returnValue = new ApplyToken();

        returnValue->op = new ContextOp();
        returnValue->op->value(path).setClearBaseFlag();
        returnValue->op->value(path).setClearDimsFlag();
        returnValue->op->apply(*(applyToken.op));
        returnValue->targetNode = applyToken.targetNode;
        returnValue->serverSequence = applyToken.serverSequence;
        return returnValue;
      } else {
        // <>[path:[o2]]:
        ApplyToken* returnValue = new ApplyToken();

        returnValue->op = new ContextOp();
        returnValue->op->setClearBaseFlag();
        returnValue->op->setClearDimsFlag();
        returnValue->op->apply(*(applyToken.op));
        returnValue->targetNode = applyToken.targetNode;
        returnValue->serverSequence = applyToken.serverSequence;
        return returnValue;
      }
    }
    break;
  case CLEAR:
    {
      ClearToken& clearToken = (ClearToken&)token;

      if (ancestry <= 0) {
        // <><>:
        // [path:<>]<>:
        return &token;
      } else {
        // <>[path:<>]:
        return this;
      }
    }
    break;
  default:
    {
      ostringstream oss;

      oss << "Attempt to accumulate token of type " << token.getTypeString()
          << " into ClearToken: ";
      token.print(oss, Log::DEBUG2);
      throw AEPException(oss.str(), Log::FATAL);
    }
  }
}


void AEPServer::ClearToken::print
(ostream& os, Log::Priority logLevel) const
{
  os << "AEPServer::ClearToken (cs: " << clientSequence << ", p:"
     << participantId;
  if (dimension != NULL) {
    os << ", d:" << dimension->canonical();
  }
  if (preFenceBit&flags) {
    os << ", preFence";
  }
  if (postFenceBit&flags) {
    os << ", postFence";
  }
  if (notifySelfBit&flags) {
    os << ", self-notify";
  }
  if (notifyClientBit&flags) {
    os << ", client-notify";
  }
  os << ')';
}


AEPServer::Participant::~Participant
()
{
  delete dimension;
}


void AEPServer::Participant::leave
()
{
  AEtherParticipant::leave();
}


void AEPServer::Participant::join
(intense::AEther& a)
{
  AEtherParticipant::join(a);
}



AEPServer::AEther::Origin& AEPServer::Participant::getNotificationOrigin
(Origin* origin_, const char* location)
{
  AEther::Origin* origin;

  if (origin_ == NULL) {
    ostringstream oss;

    oss << "INTERNAL ERROR: NULL origin passed as origin in " << location;
    throw AEPException(oss.str(), Log::FATAL);
  } else if ((origin = dynamic_cast<AEther::Origin*>(origin_)) == NULL) {
    ostringstream oss;

    oss << "INTERNAL ERROR: Non-AEPServer::AEther::Origin passed as origin in "
        << location;
    throw AEPException(oss.str(), Log::FATAL);
  }
  return *origin;
}


long long AEPServer::Participant::assign
(Context const&, CompoundDimension const*)
{
  throw AEPException(
    "INTERNAL ERROR: AEPServer::Participant::assign called",
    Log::FATAL
  );
}


long long AEPServer::Participant::apply
(intense::ContextOp const&, intense::CompoundDimension const*)
{
  throw AEPException(
    "INTERNAL ERROR: AEPServer::Participant::apply called",
    Log::FATAL
  );
}


long long AEPServer::Participant::clear
(intense::CompoundDimension const*)
{
  throw AEPException(
    "INTERNAL ERROR: AEPServer::Participant::clear called",
    Log::FATAL
  );
}


void AEPServer::Participant::assignNotify
(const Context& newValue, const CompoundDimension* dim, Origin* origin_)
{
  int targetNodeIndex;
  CompoundDimension* targetInternalDimensionPtr;
  AEther::Origin& origin =
    getNotificationOrigin(origin_, "AEPServer::Participant::assignNotify");
  AEPClient::NotifyToken::AssignTarget* assignTarget;
  AEPServer::AsynchronousToken& sourceToken = origin.getSourceToken();
  AEther& aether = (AEther&)(*(this->aether));
  AEPClient::NotifyToken* notifyToken;

  if ((sourceToken.flags&AEPServer::preFenceBit)&&
      (sourceToken.flags&AEPServer::postFenceBit)) {
    if ((!(sourceToken.flags&AEPServer::notifySelfBit))&&
        (sourceToken.participant == this)) {
      if ((aether.log != NULL)&&(aether.logLevel >= Log::DEBUG)) {
        *(aether.log)
          << Log::DEBUG << "AEPServer::Participant: Skipping assignNotify of "
          << "self (server:" << sourceToken.serverSequence << ')' << ENDLOG;
      }
      return;
    }
    if ((!(sourceToken.flags&AEPServer::notifyClientBit))&&
        (sourceToken.server == server)) {
      if ((aether.log != NULL)&&(aether.logLevel >= Log::DEBUG)) {
        *(aether.log)
          << Log::DEBUG << "AEPServer::Participant: Skipping assignNotify of "
          << "client (server:" << sourceToken.serverSequence << ')' << ENDLOG;
      }
      return;
    }
  }
  origin.addServerMapEntry(server, &aether);
  notifyToken = (AEPClient::NotifyToken*)(server->currentNotifyToken);
  notifyToken->addNode(
    (Context&)newValue, targetNodeIndex, targetInternalDimensionPtr
  );
  if (dim != NULL) {
    if (targetInternalDimensionPtr != NULL) {
      throw AEPException(
        "INTERNAL ERROR: Both internal and external dimensions are non-NULL "
        "in AEPServer::Participant::assignNotify", Log::FATAL
      );
    }
    assignTarget = new AEPClient::NotifyToken::AssignTarget(
      getId(), targetNodeIndex,
      *(new CompoundDimension(*(CompoundDimension*)dim)), true
    );
  } else if (targetInternalDimensionPtr != NULL) {
    assignTarget = new AEPClient::NotifyToken::AssignTarget(
      getId(), targetNodeIndex, *targetInternalDimensionPtr, false
    );
  } else {
    assignTarget = new AEPClient::NotifyToken::AssignTarget(
      getId(), targetNodeIndex
    );
  }
  if (!(server->deletesSentTokens())) {
    assignTarget->setAssignToken(sourceToken);
  }
  notifyToken->addTarget(*assignTarget);
}


void AEPServer::Participant::applyNotify
(const ContextOp& op, const CompoundDimension* dim, Origin* origin_)
{
  int targetNodeIndex;
  CompoundDimension* targetInternalDimensionPtr;
  AEther::Origin& origin =
    getNotificationOrigin(origin_, "AEPServer::Participant::applyNotify");
  AEPClient::NotifyToken::ApplyTarget* applyTarget;
  AEPServer::AsynchronousToken& sourceToken = origin.getSourceToken();
  AEther& aether = (AEther&)(*(this->aether));
  AEPClient::NotifyToken* notifyToken;

  if ((sourceToken.flags&AEPServer::preFenceBit)&&
      (sourceToken.flags&AEPServer::postFenceBit)) {
    if ((!(sourceToken.flags&AEPServer::notifySelfBit))&&
        (sourceToken.participant == this)) {
      if ((aether.log != NULL)&&(aether.logLevel >= Log::DEBUG)) {
        *(aether.log)
          << Log::DEBUG << "AEPServer::Participant: Skipping applyNotify of "
          << "self (server:" << sourceToken.serverSequence << ')' << ENDLOG;
      }
      return;
    }
    if ((!(sourceToken.flags&AEPServer::notifyClientBit))&&
        (sourceToken.server == server)) {
      if ((aether.log != NULL)&&(aether.logLevel >= Log::DEBUG)) {
        *(aether.log)
          << Log::DEBUG << "AEPServer::Participant: Skipping applyNotify of "
          << "client (server:" << sourceToken.serverSequence << ')' << ENDLOG;
      }
      return;
    }
  }
  origin.addServerMapEntry(server, &aether);
  notifyToken = (AEPClient::NotifyToken*)(server->currentNotifyToken);
  notifyToken->addNode(
    (Context&)op, targetNodeIndex, targetInternalDimensionPtr
  );
  if (dim != NULL) {
    if (targetInternalDimensionPtr != NULL) {
      throw AEPException(
        "INTERNAL ERROR: Both internal and external dimensions are non-NULL "
        "in AEPServer::Participant::applyNotify", Log::FATAL
      );
    }
    applyTarget = new AEPClient::NotifyToken::ApplyTarget(
      getId(), targetNodeIndex,
      *(new CompoundDimension(*(CompoundDimension*)dim)), true
    );
  } else if (targetInternalDimensionPtr != NULL) {
    applyTarget = new AEPClient::NotifyToken::ApplyTarget(
      getId(), targetNodeIndex, *targetInternalDimensionPtr, false
    );
  } else {
    applyTarget = new AEPClient::NotifyToken::ApplyTarget(
      getId(), targetNodeIndex
    );
  }
  if (!(server->deletesSentTokens())) {
    applyTarget->setApplyToken(sourceToken);
  }
  notifyToken->addTarget(*applyTarget);
}


void AEPServer::Participant::clearNotify
(const CompoundDimension* dim, Origin* origin_)
{
  AEther::Origin& origin =
    getNotificationOrigin(origin_, "AEPServer::Participant::clearNotify");
  AEPClient::NotifyToken::ClearTarget* clearTarget;
  AEPServer::AsynchronousToken& sourceToken = origin.getSourceToken();
  AEther& aether = (AEther&)(*(this->aether));
  AEPClient::NotifyToken* notifyToken;

  if ((sourceToken.flags&AEPServer::preFenceBit)&&
      (sourceToken.flags&AEPServer::postFenceBit)) {
    if ((!(sourceToken.flags&AEPServer::notifySelfBit))&&
        (sourceToken.participant == this)) {
      if ((aether.log != NULL)&&(aether.logLevel >= Log::DEBUG)) {
        *(aether.log)
          << Log::DEBUG << "AEPServer::Participant: Skipping clearNotify of "
          << "self (server:" << sourceToken.serverSequence << ')' << ENDLOG;
      }
      return;
    }
    if ((!(sourceToken.flags&AEPServer::notifyClientBit))&&
        (sourceToken.server == server)) {
      if ((aether.log != NULL)&&(aether.logLevel >= Log::DEBUG)) {
        *(aether.log)
          << Log::DEBUG << "AEPServer::Participant: Skipping clearNotify of "
          << "client (server:" << sourceToken.serverSequence << ')' << ENDLOG;
      }
      return;
    }
  }
  origin.addServerMapEntry(server, &aether);
  notifyToken = (AEPClient::NotifyToken*)(server->currentNotifyToken);
  if (dim != NULL) {
    clearTarget = new AEPClient::NotifyToken::ClearTarget(
      getId(), *((CompoundDimension*)dim)
    );
  } else  {
    clearTarget = new AEPClient::NotifyToken::ClearTarget(getId());
  }
  notifyToken->addTarget(*clearTarget);
}


void AEPServer::Participant::kickNotify
(Origin* origin)
{
}


AEPServer::AEPServer
(AEther& aether_, Log* log, Log::Priority logLevel, bool errorTolerant)
  : AEPCommon(log, logLevel, errorTolerant), aether(&aether_), running(false),
    currentNotifyToken(NULL)
{}


AEPServer::~AEPServer
()
{
  ClientDisconnectToken disconnectToken;

  stop();
  disconnectToken.setServer(*this);
  disconnectToken.handle();
}


AEPCommon::Token& AEPServer::getCurrentNotifyToken
()
{
  if (currentNotifyToken == NULL) {
    throw AEPException(
      "INTERNAL ERROR: NULL token in AEPServer::getCurrentNotificationToken",
      Log::FATAL
    );
  }
  return *currentNotifyToken;
}


void AEPServer::setCurrentNotifyToken
(AEPCommon::Token* newToken)
{
  currentNotifyToken = newToken;
}


void AEPServer::earCleanupHandler
(void* serverAsVoidPointer)
{
  AEPServer* server = (AEPServer*)serverAsVoidPointer;

  if ((server->log != NULL)&&(server->logLevel >= Log::NOTICE)) {
    *(server->log) << Log::NOTICE << "AEPServer ear thread " << pthread_self()
                   << ": cancelled." << ENDLOG;
  }
}


void* AEPServer::earMain
(void* serverAsVoidPointer)
{
  AEPServer* server = (AEPServer*)serverAsVoidPointer;
  bool runningFlag;

  pthread_cleanup_push(earCleanupHandler, serverAsVoidPointer);
  if ((server->log != NULL)&&(server->logLevel >= Log::NOTICE)) {
    *(server->log) << Log::NOTICE << "AEPServer ear thread " << pthread_self()
                   << ": starting execution..." << ENDLOG;
  }
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  try {
    server->aep();
  } catch (exception& e) {
    if ((server->log != NULL)&&(server->logLevel >= Log::ERROR)) {
      *(server->log)
        << Log::ERROR
        << "AEPServer ear thread caught exception in earMain: "
        << e.what() << ENDLOG;
    }
  } catch (...) {
    if ((server->log != NULL)&&(server->logLevel >= Log::ERROR)) {
      *(server->log)
        << Log::ERROR
        << "AEPServer ear thread caught unknown exception in earMain"
        << ENDLOG;
    }
  }
  if ((server->log != NULL)&&(server->logLevel >= Log::NOTICE)) {
    *(server->log) << Log::NOTICE << "AEPServer ear thread " << pthread_self()
                   << ": END OF LINE." << ENDLOG;
  }
  pthread_cleanup_pop(0);
  server->lock();
  runningFlag = server->running;
  if (runningFlag) {
    server->running = false;
    // stop() will not cancel, just join:
    server->unlock();
  } else {
    // someone has cancelled us (via stop()):
    server->unlock();
    pthread_testcancel();
  }
  return NULL;
}


void AEPServer::aep
()
{
  bool running = true;
  Token* token;

  while (running) {
    if ((log != NULL)&&(logLevel >= Log::DEBUG2)) {
      *log << Log::DEBUG2
           << "AEPServer::aep: blocking for available data..." << ENDLOG;
    }
    if (!blockForAvailableData()) {
      pthread_mutex_lock(&receptionMutex);
    }
    if ((log != NULL)&&(logLevel >= Log::DEBUG2)) {
      *log << Log::DEBUG2
           << "AEPServer::aep: data available; reading token..." << ENDLOG;
    }
    try {
      token = receive();
      if (token == NULL) {
        throw AEPException("INTERNAL ERROR: Received NULL AEPServer::Token");
      }
      pthread_mutex_unlock(&receptionMutex);
    } catch (exception& e) {
      if ((log != NULL)&&(logLevel >= Log::ERROR)) {
        *log << Log::ERROR << "AEPServer ear thread caught exception: "
             << e.what() << ENDLOG;
      }
      running = false;
      pthread_mutex_unlock(&receptionMutex);
    } catch (...) {
      if ((log != NULL)&&(logLevel >= Log::ERROR)) {
        *log
          << Log::ERROR
          << "AEPServer ear thread caught unknown exception" << ENDLOG;
      }
      running = false;
      pthread_mutex_unlock(&receptionMutex);
    }
    if (running) {
      running = processToken(*token);
    }
  }
}


void AEPServer::start
()
{
  lock();
  if (running) {
    unlock();
    return;
  }
  if (pthread_create(&earThread, NULL, earMain, (void*)this) != 0) {
    throw runtime_error("Could not create AEPServer ear thread");
  }
  if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
    *log << Log::NOTICE << "AEPServer::start created ear thread "
         << earThread << "." << ENDLOG;
  }
  running = true;
  unlock();
}


void AEPServer::stop
()
{
  bool runningFlag;

  lock();
  runningFlag = running;
  if (running) {
    running = false;
  }
  // We can't lock later on in stop(), since the ear is using deferred
  // cancellation and we won't be able to cancel it if it has called
  // server->lock():
  if (runningFlag) {
    if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
      *log << Log::NOTICE << "AEPServer::stop: cancelling ear thread "
           << earThread << "..." << ENDLOG;
    }
    pthread_cancel(earThread);
  }
  unlock();
  if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
    *log << Log::NOTICE << "AEPServer::stop: joining "
         << "ear thread " << earThread << "..." << ENDLOG;
  }
  pthread_join(earThread, NULL);
  if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
    *log << Log::NOTICE << "AEPServer::stop: joined ear thread "
         << earThread << "." << ENDLOG;
  }
}


void AEPServer::stop
(pthread_t externalEarThread)
{
  lock();
  if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
    *log << Log::DEBUG << "AEPServer::stop: cancelling ear thread "
         << externalEarThread << "..." << ENDLOG;
  }
  pthread_cancel(externalEarThread);
  unlock();
  if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
    *log << Log::DEBUG << "AEPServer::stop: joining "
         << "ear thread " << externalEarThread << "..." << ENDLOG;
  }
  pthread_join(earThread, NULL);
  if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
    *log << Log::NOTICE << "AEPServer::stop: joined ear thread "
         << externalEarThread << "." << ENDLOG;
  }
}


bool AEPServer::processToken
(Token& token)
{
  bool keepRunning = false;

  // AEPExceptions thrown during reception may not terminate the session, if
  // the server is error tolerant:
  token.setServer(*this);
  if ((log != NULL)&&(logLevel >= Log::DEBUG)) {
    *log << Log::DEBUG << "AEPServer: Received token: ";
    token.print(*log, (logLevel > Log::DEBUG) ? logLevel : Log::DEBUG);
    *log << ENDLOG;
  }
  try {
    switch (token.getType()) {
    case AEPServer::Token::VOID:
      throw AEPException(
        "INTERNAL ERROR: Received VOID AEPServer::Token in "
        "AEPServer processing of AEPServer::Token", Log::FATAL
      );
    case AEPServer::Token::SYNCH:
    case AEPServer::Token::CLIENT_DISCONNECT:
    case AEPServer::Token::JOIN:
    case AEPServer::Token::LEAVE:
      keepRunning = token.handle();
      delete &token;
      break;
    case AEPServer::Token::ASSIGN:
    case AEPServer::Token::APPLY:
    case AEPServer::Token::CLEAR:
      // For asynchronous tokens, we delete by reference count (the token might
      // get deleted by the aether, or, in the case of assign/apply
      // tokens, where one or more shared clients were notified by the aether
      // and incremented the reference count of the token, by the last shared
      // client to decrement the token's reference:
      // Reference count is 1 here (for the aether thread to decrement); we
      // increment it to 2, and decrement it after it is handled:
      AsynchronousToken::incrementRefCount((AsynchronousToken&)token);
      keepRunning = token.handle();
      AsynchronousToken::decrementRefCount((AsynchronousToken&)token);
      break;
    default:
      {
        ostringstream oss;

        oss << "INTERNAL ERROR: Got bad AEPServer::Token type int in "
            << "AEPServer processing of AEPServer::Token: "
            << token.getType();
        throw AEPException(oss.str(), Log::FATAL);
      }
    }
  } catch (AEPException& e) {
    if ((log != NULL)&&(logLevel >= e.getPriority())) {
      *log << e.getPriority() << "Caught AEPException in AEPServer "
           << "processing of AEPServer::Token: " << e.what() << ENDLOG;
    }
    if ((errorTolerant)&&(e.getPriority() > Log::FATAL)) {
      keepRunning = true;
    }
  } catch (IntenseException& e) {
    if ((log != NULL)&&(logLevel >= Log::CRITICAL)) {
      *log << Log::CRITICAL
           << "Caught IntenseException in AEPServer processing of "
           << "AEPServer::Token: " << e.what() << ENDLOG;
    }
  } catch (exception& e) {
    if ((log != NULL)&&(logLevel >= Log::CRITICAL)) {
      *log << Log::CRITICAL
           << "Caught std::exception in AEPServer processing of "
           << "AEPServer::Token: " << e.what() << ENDLOG;
    }
  } catch (...) {
    if ((log != NULL)&&(logLevel >= Log::CRITICAL)) {
      *log << Log::CRITICAL
           << "Caught unknown object in AEPServer processing of "
           << "AEPServer::Token" << ENDLOG;
    }
  }
  return keepRunning;
}


void AEPServer::AsynchronousToken::aetherTransaction
(const char* location)
{
  server->aether->lock();
  serverSequence = server->aether->nextServerSequence();
  server->sendLock();
  server->aether->append(*this);
  server->aether->signal();
  server->aether->unlock();
  // server sendLock still held
  try {
    AEPClient::AckToken* ackToken = new AEPClient::AckToken(
      serverSequence, clientSequence
    );

    server->send(*ackToken);
    server->sendUnlock();
    if (server->deletesSentTokens()) {
      delete ackToken;
    }
  } catch (exception& e) {
    if ((server->log != NULL)&&(server->logLevel >= Log::ERROR)) {
      *(server->log)
        << Log::ERROR
        << "AEPServer ear thread caught exception during sending of ack "
        << "(ss: " << serverSequence << ", cs: " << clientSequence
        << "): " << e.what() << ENDLOG;
    }
    server->sendUnlock();
    throw e;
  } catch (...) {
    if ((server->log != NULL)&&(server->logLevel >= Log::ERROR)) {
      *(server->log)
        << Log::ERROR
        << "AEPServer ear thread caught unknown exception during sending "
        << "of ack (ss: " << serverSequence << ", cs: " << clientSequence
        << ")" << ENDLOG;
    }
    server->sendUnlock();
    throw;
  }
}


bool AEPServer::SynchronousToken::aetherTransaction
(const char* location)
{
  string* errorMessage = NULL;
  bool successful = false;

  server->aether->lock();
  serverSequence = server->aether->nextServerSequence();
  server->aether->append(*this);
  server->aether->signal();
  server->aether->wait(server->condition);
  server->aether->unlock();
  if (reply == NULL) {
    return true;
  } else if (dynamic_cast<AEPClient::AckToken*>(reply) != NULL) {
    successful = true;
  } else if (dynamic_cast<AEPClient::ReplyToken*>(reply) == NULL) {
    ostringstream oss;

    delete reply;
    reply = NULL;
    oss << "INTERNAL ERROR: Non-AEPClient::ReplyToken reply in "
        << location;
    errorMessage = new string(oss.str());
  }
  if (errorMessage == NULL) {
    server->lockedAndLoggedSend(
      *reply, location, serverSequence, clientSequence
    );
    if (server->deletesSentTokens()) {
      delete reply;
    }
    reply = NULL;
  } else {
    AEPClient::ErrorToken* errorToken = new AEPClient::ErrorToken(
      serverSequence, clientSequence, errorMessage
    );

    if ((server->log != NULL)&&(server->logLevel >= Log::ERROR)) {
      *(server->log)
        << Log::ERROR << *errorMessage << " (ss: " << serverSequence
        << ", cs: " << clientSequence << ")" << ENDLOG;
    }
    try {
      server->lockedAndLoggedSend(
        *errorToken, location, serverSequence, clientSequence
      );
    } catch (...) {
    }
    if (server->deletesSentTokens()) {
      delete errorToken;
    }
    throw AEPException(*errorMessage, Log::FATAL);
  }
  return successful;
}


void AEPServer::SynchronousToken::resumeServer
()
{
  server->signal();
}


void AEPServer::lockedSend
(AEPClient::Token& token)
{
  pthread_mutex_lock(&sendMutex);
  try {
    send(token);
    pthread_mutex_unlock(&sendMutex);
  } catch (...) {
    pthread_mutex_unlock(&sendMutex);
    throw;
  }
}


void AEPServer::lockedAndLoggedSend
(AEPClient::Token& replyToken, const char* location,
 long long serverSequence, long long clientSequence)
{
  try {
    if ((log != NULL)&&(logLevel >= Log::DEBUG1)) {
      *log << Log::DEBUG1 << location << ": sending: ";
      replyToken.print(*log, logLevel);
      *log << ENDLOG;
    }
    lockedSend(replyToken);
  } catch (exception& e) {
    if ((log != NULL)&&(logLevel >= Log::ERROR)) {
      *log << Log::ERROR
           << "AEPServer ear thread caught exception during sending of reply "
           << "(ss: " << serverSequence << ", cs: " << clientSequence
           << ") in " << location << ": " << e.what() << ENDLOG;
    }
    throw e;
  } catch (...) {
    if ((log != NULL)&&(logLevel >= Log::ERROR)) {
      *log << Log::ERROR
           << "AEPServer ear thread caught unknown exception during sending "
           << "of ack (ss: " << serverSequence << ", cs: " << clientSequence
           << ") in " << location << ENDLOG;
    }
    throw;
  }
}


void AEPServer::leaveAllParticipants
()
{
  map<long long, AEPCommon::Participant*>::iterator participantItr;

  for (participantItr = participants.begin();
       participantItr != participants.end(); participantItr++) {
    Participant* participant = (Participant*)participantItr->second;

    participant->leave();
    delete participant;
  }
  participants.clear();
}


AEPStreamServer::AEPStreamServer
(AEther& aether, istream& in_, ostream& out_, Log* log,
 Log::Priority logLevel, bool errorTolerant)
  : AEPServer(aether, log, logLevel, errorTolerant), in(&in_),
    out(&out_)
{}


bool AEPStreamServer::blockForAvailableData
()
{
  // Linuxthreads non-POSIX read block cancellation point workaround:
  pthread_testcancel();
  in->peek();
  pthread_testcancel();
  return false;
}


AEPStreamServer* AEPStreamServer::accept
(AEther& aether, istream& is, ostream& os, Log* log, Log::Priority logLevel,
 bool errorTolerant)
{
  AEPStreamServer* server = NULL;
  char buffer[128];
  vector<string> tokens;
  const char* serverBinaryArch =
    BinaryBaseSerialiser::compatibleArchitectureString();

  is.getline(buffer, 128, '\n');
  stringtok(tokens, buffer);
  // We are looking for an initial line of the form:
  // AEP x.y CONNECT BINARY <compatible arch>
  // AEP x.y CONNECT XDR
  // AEP x.y CONNECT TEXT

  // ...followed by an empty line, or by an auth line, followed by an empty
  // line:
  is.getline(buffer, 128, '\n');
  if ((tokens.size() < 4)||(tokens.size() > 5)||(tokens[0] != "AEP")||
      (tokens[2] != "CONNECT")) {
    throwBadInput(os, buffer);
  } else if (tokens[1] != "2.0") {
    // This should really be a version number check:
    throwBadInput(os, buffer);
  } else if (tokens[3] == "BINARY") {
    // Check for a compatible architecture:
    if (tokens.size() < 5) {
      throwBadInput(os, buffer);
    } else if (tokens[4] == serverBinaryArch) {
      // Binary match:
      server = new AEPBinaryServer(
        aether, is, os, log, logLevel, errorTolerant
      );
      os << "AEP 2.0 CONNECT BINARY\n\n";
    } else {
      // Binary match failure - revert to XDR:
      server = new AEPXDRServer(
        aether, is, os, log, logLevel, errorTolerant
      );
      os << "AEP 2.0 CONNECT XDR\n\n";
    }
  } else if (tokens[3] == "XDR") {
    // XDR accept:
    server = new AEPXDRServer(
      aether, is, os, log, logLevel, errorTolerant
    );
    os << "AEP 2.0 CONNECT XDR\n\n";
  } else if (tokens[3] == "TEXT") {
    // AETP accept:
    server = new AETPServer(
      aether, is, os, log, logLevel, errorTolerant
    );
    os << "AEP 2.0 CONNECT TEXT\n\n";
  } else {
    throwBadInput(os, buffer);
  }
  os.flush();
  return server;
}


void AEPStreamServer::throwBadInput
(ostream& os, const char* badInput)
{
  ostringstream oss;

  os << "AEP 2.0 ERROR\n\n"
     << "YOUR CLIENT SENT A REQUEST THIS SERVER DID NOT UNDERSTAND\n\n";
  os.flush();
  oss << "Invalid AEP connect string: " << badInput;
  throw AEPException(oss.str());
}


AEPSerialServer::AEPSerialServer
(AEther& aether, istream& in, ostream& out, BaseSerialiser& serialiser_,
 Log* log, Log::Priority logLevel, bool errorTolerant)
  : AEPStreamServer(aether, in, out, log, logLevel, errorTolerant),
    serialiser(&serialiser_)
{}


AEPSerialServer::~AEPSerialServer
()
{
  delete serialiser;
}


void AEPSerialServer::send
(AEPCommon::Token& token)
{
  Token::factorySerialise((Token*)&token, *serialiser, *out);
}


AEPServer::Token* AEPSerialServer::receive
()
{
  return Token::factoryDeserialise(*serialiser, *in);
}


AEPBinaryServer::AEPBinaryServer
(AEther& aether, istream& in, ostream& out, Log* log,
 Log::Priority logLevel, bool errorTolerant)
  : AEPSerialServer(
      aether, in, out, *(new BinaryBaseSerialiser), log, logLevel,
      errorTolerant
    )
{}


AEPBinaryServer::~AEPBinaryServer
()
{}


const char* AEPBinaryServer::getName
()
{
  ostringstream oss;

  oss << "AEPBinaryServer:" << this;
  return oss.str().data();
}


AEPXDRServer::AEPXDRServer
(AEther& aether, istream& in, ostream& out, Log* log, Log::Priority logLevel,
 bool errorTolerant)
  : AEPSerialServer(
      aether, in, out, *(new XDRBaseSerialiser), log, logLevel, errorTolerant
    )
{}


AEPXDRServer::~AEPXDRServer
()
{}


const char* AEPXDRServer::getName
()
{
  ostringstream oss;

  oss << "AEPBinaryServer:" << this;
  return oss.str().data();
}
