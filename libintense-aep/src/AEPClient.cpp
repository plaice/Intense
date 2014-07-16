// ****************************************************************************
//
// AEPClient.cpp : AEP 2.0 client, for the abstract/core AEP implementation.
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
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <queue>
#include <iostream>
#include <errno.h>
#include <intense.hpp>
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


// This implementation follows a much looser appropach than that of AEP 1.x,
// which was non-multiplexed and stateful.  Here, we have only to keep track
// of a vector of specialised participants.


const char* AEPClient::Token::typeStrings[] = {
  "VOID", "CLIENT_TERMINATE", "SERVER_DISCONNECT", "NOTIFY",
  "ACK", "DENY", "ERROR"
};


AEPClient::Token* AEPClient::Token::factory
(Type type)
{
  switch (type) {
  case VOID:
    return NULL;
  case SERVER_DISCONNECT:
    return new ServerDisconnectToken();
  case NOTIFY:
    return new NotifyToken();
  case ACK:
    return new AckToken();
  case DENY:
    return new DenyToken();
  default:
    ostringstream oss;

    oss << "Bad AEPClient::Token type " << type;
    throw AEPException(oss.str(), Log::FATAL);
  }
}


void AEPClient::Token::factorySerialise
(const Token* token, BaseSerialiser& baseSerialiser, ostream& os)
{
  int type;

  if (token != NULL) {
    type = ((AEPClient::Token*)token)->getType();
  } else {
    type = AEPClient::Token::VOID;
  }
  baseSerialiser.out(type, os);
  if (token != NULL) {
    ((AEPClient::Token*)token)->serialise(baseSerialiser, os);
  }
}


AEPClient::Token* AEPClient::Token::factoryDeserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  int type;
  AEPClient::Token* token;

  baseSerialiser.in(type, is);
  token = AEPClient::Token::factory((AEPClient::Token::Type)type);
  if (token != NULL) {
    token->deserialise(baseSerialiser, is);
  }
  return token;
}


void AEPClient::ClientTerminateToken::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  throw AEPException(
    "INTERNAL ERROR: ClientTerminateToken::serialise() called", Log::FATAL
  );
}


void AEPClient::ClientTerminateToken::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  throw AEPException(
    "INTERNAL ERROR: ClientTerminateToken::deserialise() called", Log::FATAL
  );
}


bool AEPClient::ClientTerminateToken::handle
()
{
  if (!client->usingReceiverThread) {
    throw AEPException(
      "INTERNAL ERROR: usingReceiverThread false in "
      "ClientTerminateToken::handle()", Log::FATAL
    );
  }
  client->stopped = true;
  client->clientSequenceBinderMap.resumeAll();
  return false; 
}


void AEPClient::ClientTerminateToken::print
(ostream& os, Log::Priority logLevel) const
{
  os << "AEPClient::ClientTerminateToken";
  if (message != NULL) {
    os << ": " << *message;
  }
}


void AEPClient::ServerDisconnectToken::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  baseSerialiser.out(serverSequence, os);
}


void AEPClient::ServerDisconnectToken::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  baseSerialiser.in(serverSequence, is);
}


bool AEPClient::ServerDisconnectToken::handle
()
{
  client->stopped = true;
  client->clientSequenceBinderMap.resumeAll();
  return false;
}


void AEPClient::ServerDisconnectToken::print
(ostream& os, Log::Priority logLevel) const
{
  os << "AEPClient::ServerDisconnectToken(server:" << serverSequence << ")";
}


const char* AEPClient::NotifyToken::Target::typeStrings[] = {
  "ASSIGN", "APPLY", "CLEAR", "KICK"
};


AEPClient::NotifyToken::Target::~Target
()
{
}


AEPClient::NotifyToken::Target* AEPClient::NotifyToken::Target::factory
(Type type)
{
  switch (type) {
  case ASSIGN:
    return new AssignTarget();
  case APPLY:
    return new ApplyTarget();
  case CLEAR:
    return new ClearTarget();
  case KICK:
    return new KickTarget();
  default:
    ostringstream oss;

    oss << "Bad AEPClient::NotifyToken::Target type " << type;
    throw AEPException(oss.str());
  }
}


void AEPClient::NotifyToken::Target::factorySerialise
(const Target* target, BaseSerialiser& baseSerialiser, ostream& os)
{
  int type;

  if (target != NULL) {
    type = ((Target*)target)->getType();
    baseSerialiser.out(type, os);
    ((Target*)target)->serialise(baseSerialiser, os);
  } else {
    throw AEPException(
      "NULL target in AEPClient::NotifyToken::Target::factorySerialise"
    );
  }
}


AEPClient::NotifyToken::Target*
AEPClient::NotifyToken::Target::factoryDeserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  int type;
  AEPClient::NotifyToken::Target* target;

  baseSerialiser.in(type, is);
  target = AEPClient::NotifyToken::Target::factory((Type)type);
  target->deserialise(baseSerialiser, is);
  return target;
}


AEPClient::NotifyToken::AssignTarget::~AssignTarget
()
{
  delete dimension;
  if (assignToken != NULL) {
    AEPServer::AsynchronousToken::decrementRefCount(
      *((AEPServer::AsynchronousToken*)assignToken)
    );
  }
}


void AEPClient::NotifyToken::AssignTarget::setAssignToken
(AEPCommon::Token& assignToken_)
{
  assignToken = &assignToken_;
  AEPServer::AsynchronousToken::incrementRefCount(
    *((AEPServer::AsynchronousToken*)assignToken)
  );
}


void AEPClient::NotifyToken::AssignTarget::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  int haveDimension = (dimension != NULL) ? 1 : 0;

  baseSerialiser.out(participantId, os);
  baseSerialiser.out(contextNodeIndex, os);
  baseSerialiser.out(haveDimension, os);
  if (dimension != NULL) {
    int dimensionIsExternalInt = (dimensionIsExternal) ? 1 : 0;

    dimension->serialise(baseSerialiser, os);
    baseSerialiser.out(dimensionIsExternalInt, os);
  }
}


void AEPClient::NotifyToken::AssignTarget::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  int haveDimension;

  baseSerialiser.in(participantId, is);
  baseSerialiser.in(contextNodeIndex, is);
  baseSerialiser.in(haveDimension, is);
  if (haveDimension != 0) {
    int dimensionIsExternalInt;

    dimension = new CompoundDimension();
    dimension->deserialise(baseSerialiser, is);
    baseSerialiser.in(dimensionIsExternalInt, is);
    dimensionIsExternal = (dimensionIsExternalInt != 0) ? true : false;
  } else {
    dimension = NULL;
    dimensionIsExternal = false;
  }
}


void AEPClient::NotifyToken::AssignTarget::print
(ostream& os, Log::Priority logLevel, NotifyToken& token) const
{
  os << "AssignTarget(participant:" << participantId << ", dimension:";
  if (dimension != NULL) {
    os << dimension->canonical() << ", external:";
    if (dimensionIsExternal) {
      os << 't';
    } else {
      os << 'f';
    }
  } else {
    os << '-';
  }
  if (logLevel >= Log::DEBUG2) {
    os << ", node:" << contextNodeIndex << ", value: "
       << token.nodes[contextNodeIndex]->canonical();
  }
  os << ')';
}


void AEPClient::NotifyToken::AssignTarget::handle
(NotifyToken& token)
{
  const char* location = "AEPClient::NotifyToken::AssignTarget::handle";

  Participant& participant = (Participant&)token.client->getParticipant(
    participantId, location
  );

  if ((contextNodeIndex < 0)||(contextNodeIndex >= token.nodes.size())) {
    ostringstream oss;

    oss << "Bad contextNodeIndex " << contextNodeIndex << " in " << location;
    throw AEPException(oss.str());
  }
  if (dimension == NULL) { 
    participant.assignNotify(
      token.getContext(contextNodeIndex, NULL), NULL, &token
    );
  } else if (dimensionIsExternal) {
    participant.assignNotify(
      token.getContext(contextNodeIndex, NULL), dimension, &token
    );
  } else {
    participant.assignNotify(
      token.getContext(contextNodeIndex, dimension), NULL, &token
    );
  }
}


AEPClient::NotifyToken::ApplyTarget::~ApplyTarget
()
{
  delete dimension;
  if (applyToken != NULL) {
    AEPServer::AsynchronousToken::decrementRefCount(
      *((AEPServer::AsynchronousToken*)applyToken)
    );
  }
}


void AEPClient::NotifyToken::ApplyTarget::setApplyToken
(AEPCommon::Token& applyToken_)
{
  applyToken = &applyToken_;
  AEPServer::AsynchronousToken::incrementRefCount(
    *((AEPServer::AsynchronousToken*)applyToken)
  );
}


void AEPClient::NotifyToken::ApplyTarget::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  int haveDimension = (dimension != NULL) ? 1 : 0;

  baseSerialiser.out(participantId, os);
  baseSerialiser.out(opNodeIndex, os);
  baseSerialiser.out(haveDimension, os);
  if (dimension != NULL) {
    int dimensionIsExternalInt = (dimensionIsExternal) ? 1 : 0;

    dimension->serialise(baseSerialiser, os);
    baseSerialiser.out(dimensionIsExternalInt, os);
  }
}


void AEPClient::NotifyToken::ApplyTarget::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  int haveDimension;

  baseSerialiser.in(participantId, is);
  baseSerialiser.in(opNodeIndex, is);
  baseSerialiser.in(haveDimension, is);
  if (haveDimension != 0) {
    int dimensionIsExternalInt;

    dimension = new CompoundDimension();
    dimension->deserialise(baseSerialiser, is);
    baseSerialiser.in(dimensionIsExternalInt, is);
    dimensionIsExternal = (dimensionIsExternalInt != 0) ? true : false;
  } else {
    dimension = NULL;
    dimensionIsExternal = false;
  }
}


void AEPClient::NotifyToken::ApplyTarget::print
(ostream& os, Log::Priority logLevel, NotifyToken& token) const
{
  os << "ApplyTarget(participant:" << participantId << ", dimension:";
  if (dimension != NULL) {
    os << dimension->canonical() << ", external:";
    if (dimensionIsExternal) {
      os << 't';
    } else {
      os << 'f';
    }
  } else {
    os << '-';
  }
  os << ", node:" << opNodeIndex << ')';
}


void AEPClient::NotifyToken::ApplyTarget::handle
(NotifyToken& token)
{
  const char* location = "AEPClient::NotifyToken::ApplyTarget::handle";
  Participant& participant = (Participant&)token.client->getParticipant(
    participantId, location
  );

  if ((opNodeIndex < 0)||(opNodeIndex >= token.nodes.size())) {
    ostringstream oss;

    oss << "Bad contextNodeIndex " << opNodeIndex << " in " << location;
    throw AEPException(oss.str());
  }
  if (dimension == NULL) { 
    participant.applyNotify(
      token.getContextOp(opNodeIndex, NULL), NULL, &token
    );
  } else if (dimensionIsExternal) {
    participant.applyNotify(
      token.getContextOp(opNodeIndex, NULL), dimension, &token
    );
  } else {
    participant.applyNotify(
      token.getContextOp(opNodeIndex, dimension), NULL, &token
    );
  }
}


AEPClient::NotifyToken::ClearTarget::~ClearTarget
()
{
  delete dimension;
}


void AEPClient::NotifyToken::ClearTarget::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  int haveDimension = (dimension != NULL) ? 1 : 0;

  baseSerialiser.out(participantId, os);
  baseSerialiser.out(haveDimension, os);
  if (dimension != NULL) {
    dimension->serialise(baseSerialiser, os);
  }
}


void AEPClient::NotifyToken::ClearTarget::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  int haveDimension;

  baseSerialiser.in(participantId, is);
  baseSerialiser.in(haveDimension, is);
  if (haveDimension != 0) {
    dimension = new CompoundDimension();
    dimension->deserialise(baseSerialiser, is);
  } else {
    dimension = NULL;
  }
}


void AEPClient::NotifyToken::ClearTarget::print
(ostream& os, Log::Priority logLevel, NotifyToken& token) const
{
  os << "ClearTarget(participant:" << participantId << ", dimension:";
  if (dimension != NULL) {
    os << dimension->canonical();
  } else {
    os << '-';
  }
  os << ')';
}


void AEPClient::NotifyToken::ClearTarget::handle
(NotifyToken& token)
{
  const char* location = "AEPClient::NotifyToken::ClearTarget::handle";

  token.client->getParticipant(
    participantId, location
  ).clearNotify(dimension, &token);
}


void AEPClient::NotifyToken::KickTarget::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  baseSerialiser.out(participantId, os);
}


void AEPClient::NotifyToken::KickTarget::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  baseSerialiser.in(participantId, is);
}


void AEPClient::NotifyToken::KickTarget::print
(ostream& os, Log::Priority logLevel, NotifyToken& token) const
{
  os << "KickTarget(participant:" << participantId << ')';
}


void AEPClient::NotifyToken::KickTarget::handle
(NotifyToken& token)
{
  const char* location = "AEPClient::NotifyToken::KickTarget::handle";

  token.client->getParticipant(participantId, location).kickNotify(&token);
}


Context& AEPClient::NotifyToken::getContext
(int nodeIndex, CompoundDimension* dimension)
{
  Context* node;

  if (nodeIndex >= nodes.size()) {
    ostringstream oss;

    oss << "PROTOCOL ERROR: Attempt to reference notification node with index "
        << nodeIndex << " >= " << nodes.size();
    throw AEPException(oss.str(), Log::ERROR);
  } else if ((node = nodes[nodeIndex])->getType() != Context::CONTEXT) {
    ostringstream oss;

    oss << "INTERNAL ERROR: Attempt to reference notification non-Context "
        << "node with index " << nodeIndex << " as Context";
    throw AEPException(oss.str(), Log::ERROR);
  }
  if (dimension != NULL) {
    return (*node)[*dimension];
  } else {
    return (*node);
  }
}


ContextOp& AEPClient::NotifyToken::getContextOp
(int nodeIndex, CompoundDimension* dimension)
{
  Context* node;

  if (nodeIndex >= nodes.size()) {
    ostringstream oss;

    oss << "PROTOCOL ERROR: Attempt to reference notification node with index "
        << nodeIndex << " >= " << nodes.size();
    throw AEPException(oss.str(), Log::ERROR);
  } else if ((node = nodes[nodeIndex])->getType() != Context::CONTEXTOP) {
    ostringstream oss;

    oss << "INTERNAL ERROR: Attempt to reference notification non-ContextOp "
        << "node with index " << nodeIndex << " as ContextOp";
  }
  if (dimension != NULL) {
    return (ContextOp&)(*node)[*dimension];
  } else {
    return (ContextOp&)(*node);
  }
}


void AEPClient::NotifyToken::addTarget
(Target& target)
{
  targets.push_back(&target);
}


void AEPClient::NotifyToken::addNode
(Context& node, int& nodeIndex, CompoundDimension*& internalDimension)
{
  // libintense has been changed to greatly facilitate things here - first
  // propagation, then preorder assignment or application (i.e., the minimal
  // context/op for an operation is always seen first, by a server with
  // participants in multiple places):
  if ((nodes.size() == 0)||
      ((&node != nodes.back())&&(!(nodes.back()->isAncestor(node))))) {
    nodes.push_back(&node);
    internalDimension = NULL;
  } else if (&node == nodes.back()) {
    internalDimension = NULL;
  } else {
    // We find the internal dimension path from node to nodes.back():
    internalDimension = new CompoundDimension;
    nodes.back()->findPathTo(node, *internalDimension);
  }
  nodeIndex = nodes.size() - 1;
}


void AEPClient::NotifyToken::addNode
(Context& node)
{
  nodes.push_back(&node);
}


void AEPClient::NotifyToken::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  int type = ((NotifyToken*)this)->getType();
  int targetsSize = targets.size();
  int nodesSize = nodes.size();

  baseSerialiser.out(serverSequence, os);
  baseSerialiser.out(targetsSize, os);
  for (int i = 0; i < targetsSize; i++) {
    Target::factorySerialise(targets[i], baseSerialiser, os);
  }
  baseSerialiser.out(nodesSize, os);
  for (int i = 0; i < nodesSize; i++) {
    Context::factorySerialise(nodes[i], baseSerialiser, os);
  }
}


void AEPClient::NotifyToken::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  int type;
  int targetsSize = targets.size();
  int nodesSize = nodes.size();

  baseSerialiser.in(serverSequence, is);
  baseSerialiser.in(targetsSize, is);
  for (int i = 0; i < targetsSize; i++) {
    targets.push_back(Target::factoryDeserialise(baseSerialiser, is));
  }
  baseSerialiser.in(nodesSize, is);
  for (int i = 0; i < nodesSize; i++) {
    nodes.push_back(Context::factoryDeserialise(baseSerialiser, is));
  }
}


bool AEPClient::NotifyToken::handle
()
{
  for (vector<Target*>::iterator itr = targets.begin(); itr != targets.end();
       itr++) {
    (*itr)->handle(*this);
  }
  return true;
}


AEPClient::NotifyToken::~NotifyToken
()
{
  for (vector<Target*>::iterator itr = targets.begin(); itr != targets.end();
       itr++) {
    delete *itr;
  }
  // If this token is from a shared server/client, then the
  // AsynchronousToken::decrementRefCount() calls, from the deletion of the
  // targets, will have deleted the nodes along with the source token:
  if ((client != NULL)&&(client->deletesSentTokens())) {
    for (vector<Context*>::iterator itr = nodes.begin(); itr != nodes.end();
         itr++) {
      delete *itr;
    }
  } else {
    nodes.clear();
  }
}


void AEPClient::NotifyToken::print
(ostream& os, Log::Priority logLevel) const
{
  os << "AEPClient::NotifyToken(server:" << serverSequence << ", targets(";
  for (int i = 0; i < targets.size(); i++) {
    if (i > 0) {
      os << ", ";
    }
    targets[i]->print(os, logLevel, *((NotifyToken*)this));
  }
  os << ')';
  if (logLevel >= Log::DEBUG1) {
    os << ", nodes(";
    for (int i = 0; i < nodes.size(); i++) {
      if (i > 0) {
        os << ", ";
      }
      os << nodes[i]->canonical();
    }
    os << ')';
  }
  os << ')';
}


void AEPClient::NotifyToken::shallowClear
()
{
}


bool AEPClient::ReplyToken:: handle
()
{
  ostringstream oss;

  oss << "INTERNAL ERROR: " << getTypeString() << " token handle() called.";
  throw AEPException(oss.str(), Log::FATAL);
  // Unreachable:
  return false; 
}


void AEPClient::ReplyToken::print
(ostream& os, Log::Priority logLevel) const
{
  os << "AEPClient::ReplyToken(type:" << ((ReplyToken*)this)->getTypeString()
     << ", server:" << ((ReplyToken*)this)->getServerSequence() << ", client:"
     << ((ReplyToken*)this)->getClientSequence() << ")";
  if (((ReplyToken*)this)->getMessage() != NULL) {
    os << ", message: \"" << *(((ReplyToken*)this)->getMessage()) << '"';
  }
  os << ')';
}


void AEPClient::AckToken::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  baseSerialiser.out(serverSequence, os);
  baseSerialiser.out(clientSequence, os);
}


void AEPClient::AckToken::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  baseSerialiser.in(serverSequence, is);
  baseSerialiser.in(clientSequence, is);
}


void AEPClient::AckToken::print
(ostream& os, Log::Priority logLevel) const
{
  os << "AEPClient::AckToken: (s:"
     << ((AckToken*)this)->getServerSequence() << ", c:"
     << ((AckToken*)this)->getClientSequence() << ")";
  if (((AckToken*)this)->getMessage() != NULL) {
    os << ": " << *(((AckToken*)this)->getMessage());
  }
}


void AEPClient::DenyToken::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  baseSerialiser.out(serverSequence, os);
  baseSerialiser.out(clientSequence, os);
}


void AEPClient::DenyToken::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  baseSerialiser.in(serverSequence, is);
  baseSerialiser.in(clientSequence, is);
}


void AEPClient::DenyToken::print
(ostream& os, Log::Priority logLevel) const
{
  os << "AEPClient::DenyToken: (s:"
     << ((DenyToken*)this)->getServerSequence() << ", c:"
     << ((DenyToken*)this)->getClientSequence() << ")";
  if (((DenyToken*)this)->getMessage() != NULL) {
    os << ": " << *(((DenyToken*)this)->getMessage());
  }
}


void AEPClient::ErrorToken::print
(ostream& os, Log::Priority logLevel) const
{
  os << "AEPClient::ErrorToken: (s:"
     << ((ErrorToken*)this)->getServerSequence() << ", c:"
     << ((ErrorToken*)this)->getClientSequence() << ")";
  if (((ErrorToken*)this)->getMessage() != NULL) {
    os << ": " << *(((ErrorToken*)this)->getMessage());
  }
}


long AEPClient::Participant::join
(AEPClient& newClient, bool notify, const CompoundDimension* dimension,
 pthread_cond_t* optionalBlockCondition)
{
  return join(&newClient, notify, dimension, optionalBlockCondition);
}


long AEPClient::Participant::join
(AEPClient& newClient, bool notify, const char* dim,
 pthread_cond_t* optionalBlockCondition)
{
  CompoundDimension dimension(dim);

  return join(&newClient, notify, &dimension, optionalBlockCondition);
}


long AEPClient::Participant::join
(bool notify, const CompoundDimension* dimension,
 pthread_cond_t* optionalBlockCondition)
{
  return join((AEPClient*)NULL, notify, dimension, optionalBlockCondition);
}


long AEPClient::Participant::join
(bool notify, const char* dim, pthread_cond_t* optionalBlockCondition)
{
  CompoundDimension dimension(dim);

  return join((AEPClient*)NULL, notify, &dimension, optionalBlockCondition);
}


long AEPClient::Participant::join
(AEPClient* newClient, bool notify, const CompoundDimension* dimension,
 pthread_cond_t* blockCondition)
{
  if ((client != NULL)&&(getId() >= 0)) {
    leave();
  }
  if (newClient != NULL) {
    client = newClient;
  } else {
    if (client == NULL) {
      throw AEPException(
        "NULL AEPClient in AEPClient::Participant::join()", Log::ERROR
      );
    }
  }
  client->lock();
  client->addParticipant(*this);
  return client->transaction(
    new AEPServer::JoinToken(
      client->nextSequence(), getId(), notify, (CompoundDimension*)dimension
    ),
    true, false, blockCondition
  );
}


long AEPClient::Participant::leave
(pthread_cond_t* blockCondition)
{
  const char* location = "AEPClient::Participant::leave";
  long returnValue;

  checkValidClientAndJoined(location);
  client->lock();
  returnValue = client->transaction(
    new AEPServer::LeaveToken(client->nextSequence(), getId()),
    true, false, blockCondition
  );
  client->removeParticipant(*this, location);
  return returnValue;
}


long AEPClient::Participant::assign
(const Context& context, const char* dim,
 int flags, pthread_cond_t* blockCondition)
{
  CompoundDimension dimension(dim);

  return assign(context, &dimension, flags, blockCondition);
}


long AEPClient::Participant::assign
(const Context& context, const CompoundDimension* dim,
 int flags, pthread_cond_t* blockCondition)
{
  checkValidClientAndJoined("AEPClient::Participant::assign");
  client->lock();
  return client->transaction(
    new AEPServer::AssignToken(
      client->nextSequence(), getId(), (Context&)context,
      (CompoundDimension*)dim, flags
    ),
    true, false, blockCondition
  );
}


long AEPClient::Participant::apply
(const ContextOp& op, const char* dim,
 int flags, pthread_cond_t* blockCondition)
{
  CompoundDimension dimension(dim);

  return apply(op, &dimension, flags, blockCondition);
}


long AEPClient::Participant::apply
(const ContextOp& op, const CompoundDimension* dim,
 int flags, pthread_cond_t* blockCondition)
{
  checkValidClientAndJoined("AEPClient::Participant::apply");
  client->lock();
  return client->transaction(
    new AEPServer::ApplyToken(
      client->nextSequence(), getId(), (ContextOp&)op,
      (CompoundDimension*)dim, flags
    ),
    true, false, blockCondition
  );
}


long AEPClient::Participant::clear
(const char* dim, int flags, pthread_cond_t* blockCondition)
{
  CompoundDimension dimension(dim);

  return clear(&dimension, flags, blockCondition);
}


long AEPClient::Participant::clear
(const CompoundDimension* dim, int flags, pthread_cond_t* blockCondition)
{
  checkValidClientAndJoined("AEPClient::Participant::clear");
  client->lock();
  return client->transaction(
    new AEPServer::ClearToken(
      client->nextSequence(), getId(), (CompoundDimension*)dim, flags
    ),
    true, false, blockCondition
  );
}


long AEPClient::Participant::synch
(pthread_cond_t* blockCondition)
{
  client->lock();
  return client->transaction(
    new AEPServer::SynchToken(client->nextSequence()), true, false,
    blockCondition
  );
}


void AEPClient::Participant::synch
(long serverSequence, pthread_cond_t* blockCondition)
{
  client->lock();
  if (client->getServerSequence() >= serverSequence) {
    client->unlock();
    return;
  }
  client->serverSequenceBinderMap.wait(serverSequence, false, blockCondition);
  client->unlock();
}


void AEPClient::Participant::checkValidClientAndJoined
(const char* location)
{
  if (client == NULL) {
    ostringstream oss;

    oss << "NULL AEPClient in " << location;
    throw AEPException(oss.str(), Log::ERROR);
  }
  if (getId() < 0) {
    ostringstream oss;

    oss << "Not joined in " << location;
    throw AEPException(oss.str(), Log::ERROR);
  }
}


AEPClient::SequenceBinder::SequenceBinder
(long sequence_, bool terminate_, pthread_cond_t* condition_)
  : sequence(sequence_), terminate(terminate_), token(NULL),
    allocatedCondition(false), condition(condition_)
{
  if (condition == NULL) {
    condition = new pthread_cond_t;
    allocatedCondition = true;
    pthread_cond_init(condition, NULL);
  }
}


AEPClient::SequenceBinder::~SequenceBinder
()
{
  if (allocatedCondition) {
    pthread_cond_destroy(condition);
    delete condition;
  }
}


AEPClient::ReplyToken* AEPClient::SequenceBinderMap::wait
(long sequence, bool terminate, pthread_cond_t* condition)
{
  ReplyToken* returnValue;
  SequenceBinder* binder = new SequenceBinder(sequence, terminate, condition);

  (*this)[sequence] = binder;
  pthread_cond_wait(binder->condition, &(client->mutex));
  returnValue = binder->token;
  delete binder;
  return returnValue;
}


bool AEPClient::SequenceBinderMap::resumeAll
()
{
  map<long, SequenceBinder*>::iterator itr;
  bool keepRunning = true;

  for (itr = begin(); itr != end(); itr++) {
    pthread_cond_signal(itr->second->condition);
    if (itr->second->terminate) {
      keepRunning = false;
    }
  }
  clear();
  return keepRunning;
}


bool AEPClient::SequenceBinderMap::resume
(ReplyToken& token)
{
  map<long, SequenceBinder*>::iterator itr;
  bool keepRunning = true;

  if ((itr = find(token.getClientSequence())) == end()) {
    ostringstream oss;

    oss << "PROTOCOL ERROR: SequenceBinder with client sequence "
        << token.getClientSequence() << " not found in client map";
    // Really only critical if we care about the server:
    throw AEPException(oss.str(), Log::ERROR);
  }
  itr->second->token = &token;
  pthread_cond_signal(itr->second->condition);
  if (itr->second->terminate) {
    keepRunning = false;
  }
  erase(itr);
  return keepRunning;
}


bool AEPClient::SequenceBinderMap::resumeUntil
(long sequence)
{
  map<long, SequenceBinder*>::iterator itr = begin();
  bool keepRunning = true;

  while ((itr != end())&&(itr->second->sequence <= sequence)) {
    pthread_cond_signal(itr->second->condition);
    if (itr->second->terminate) {
      keepRunning = false;
    }
    erase(itr);
  }
  return keepRunning;
}


AEPClient::AEPClient
(Log* log, Log::Priority logLevel, bool errorTolerant, bool useReceiverThread,
 int maxReceiveSize_)
  : AEPCommon(log, logLevel, errorTolerant), sequence(0), serverSequence(0),
    clientSequenceBinderMap(*this), serverSequenceBinderMap(*this),
    stopped(true), usingReceiverThread(useReceiverThread),
    maxReceiveSize(maxReceiveSize_), receiverQueue(NULL)
{
  if (usingReceiverThread) {
    if (maxReceiveSize <= 0) {
      ostringstream oss;

      oss << "ERROR: Invalid maxReceiveSize " << maxReceiveSize
          << " supplied to AEPClient";
      throw AEPException(oss.str(), Log::CRITICAL);
    }
    receiverQueue = new std::queue<Token*>;
    pthread_mutex_init(&receiverQueueMutex, NULL);
    pthread_cond_init(&receiverQueueCondition, NULL);
  }
}


// Assumes client has been locked and unlocks it before returning:
long AEPClient::transaction
(AEPCommon::Token* outgoing_, bool allowDeny, bool terminate,
 pthread_cond_t* blockCondition)
{
  long serverSequence = -1;
  AEPServer::Token& outgoing = *((AEPServer::Token*)outgoing_);
  Token* reply = NULL;
  const char* typeString = outgoing.getTypeString();
  long clientSequence = outgoing.getClientSequence();

  try {
    if ((log != NULL)&&(logLevel >= Log::DEBUG1)) {
      *log << Log::DEBUG1 << "AEPClient::transaction: sending: ";
      outgoing.print(*log, logLevel);
      *log << ENDLOG;
    }
    send(outgoing);
    if (deletesSentTokens()) {
      delete outgoing_;
      outgoing_ = NULL;
    }
    if ((log != NULL)&&(logLevel >= Log::DEBUG1)) {
      *log << Log::DEBUG1 << "AEPClient::transaction: "
           << "token sent; waiting for reply..." << ENDLOG;
    }
    reply =
      waitForReply(clientSequence, typeString, terminate, blockCondition);
    if ((log != NULL)&&(logLevel >= Log::DEBUG1)) {
      *log << Log::DEBUG1
           << "AEPClient::transaction: received reply: ";
      reply->print(*log, logLevel);
      *log << ENDLOG;
    }
    switch (reply->getType()) {
    case Token::ACK:
      // Success:
      serverSequence = ((AckToken*)reply)->getServerSequence();
      break;
    case Token::DENY:
      if (allowDeny) {
        string* message = ((DenyToken*)reply)->getMessage();
        ostringstream oss;

        oss << typeString << " DENIED";
        if (message != NULL) {
          oss << ": " << *message;
        }
        throw AEPException(oss.str(), Log::WARN);
      }
      // else fall through:
    case Token::ERROR:
      {
        string* message = ((ErrorToken*)reply)->getMessage();
        ostringstream oss;

        oss << typeString << " ERROR";
        if (message != NULL) {
          oss << ": " << *message;
        }
        throw AEPException(oss.str(), Log::ERROR);
      }
      break;
    default:
      {
        ostringstream oss;

        oss << "INTERNAL ERROR: Bad Token type " << reply->getTypeString()
            << " in " << typeString;
        throw AEPException(oss.str(), Log::FATAL);
      }
    }
    delete reply;
    reply = NULL;
  } catch (...) {
    unlock();
    if (deletesSentTokens()) {
      delete outgoing_;
    }
    delete reply;
    throw;
  }
  unlock();
  return serverSequence;
}


void AEPClient::start
()
{
  lock();
  if (!stopped) {
    unlock();
    return;
  }
  if (usingReceiverThread) {
    pthread_mutex_lock(&receiverQueueMutex);
    if (pthread_create(&earThread, NULL, earMain, (void*)this) != 0) {
      throw runtime_error("Could not create AEPClient ear thread");
    }
    if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
      *log << Log::NOTICE << "AEPClient::start created ear thread "
           << earThread << "." << ENDLOG;
    }
    pthread_cond_wait(&receiverQueueCondition, &receiverQueueMutex);
    // Ear is now waiting on receiverCondition in aep()...
    if (pthread_create(&receiverThread, NULL, receiverMain, (void*)this)
        != 0) {
      throw runtime_error("Could not create AEPClient receiver thread");
    }
    if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
      *log << Log::NOTICE << "AEPClient::start created receiver thread "
           << receiverThread << "." << ENDLOG;
    }
    pthread_mutex_unlock(&receiverQueueMutex);
  } else {
    if (pthread_create(&earThread, NULL, earMain, (void*)this) != 0) {
      throw runtime_error("Could not create AEPClient ear thread");
    }
    if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
      *log << Log::NOTICE << "AEPClient::start created ear thread "
           << earThread << "." << ENDLOG;
    }
    wait();
  }
  stopped = false;
  unlock();
}


AEPClient::~AEPClient
()
{
  stop();
  if (usingReceiverThread) {
    while (receiverQueue->size() > 0) {
      delete receiverQueue->front();
      receiverQueue->pop();
    }
    delete receiverQueue;
    pthread_mutex_destroy(&receiverQueueMutex);
    pthread_cond_destroy(&receiverQueueCondition);
  }
}


long AEPClient::assign
(const Context& context, const CompoundDimension* dim, int flags,
 pthread_cond_t* blockCondition)
{
  lock();
  return transaction(
    new AEPServer::AssignToken(
      nextSequence(), -1, (Context&)context, (CompoundDimension*)dim, 0
    ),
    true, false, blockCondition
  );
}


long AEPClient::apply
(const ContextOp& op, const CompoundDimension* dim, int flags,
 pthread_cond_t* blockCondition)
{
  lock();
  return transaction(
    new AEPServer::ApplyToken(
      nextSequence(), -1, (ContextOp&)op, (CompoundDimension*)dim, flags
    ),
    true, false, blockCondition
  );
}


long AEPClient::clear
(const CompoundDimension* dim, int flags, pthread_cond_t* blockCondition)
{
  lock();
  return transaction(
    new AEPServer::ClearToken(
      nextSequence(), -1, (CompoundDimension*)dim, flags
    ),
    true, false, blockCondition
  );
}


long AEPClient::synch
(pthread_cond_t* blockCondition)
{
  lock();
  return transaction(
    new AEPServer::SynchToken(nextSequence()), true, false, blockCondition
  );
}


void AEPClient::synch
(long serverSequence, pthread_cond_t* blockCondition)
{
  lock();
  serverSequenceBinderMap.wait(serverSequence, blockCondition);
  unlock();
}


long AEPClient::disconnect
(pthread_cond_t* blockCondition)
{
  lock();
  return transaction(
    new AEPServer::ClientDisconnectToken(nextSequence()), true, true,
    blockCondition
  );
}


void AEPClient::stop
()
{
  bool runningFlag;

  lock();
  sendLock();
  if (stopped) {
    unlock();
    return;
  }
  stopped = true;
  // We can't lock later on in stop(), since the ear is using deferred
  // cancellation and we won't be able to cancel it if it has called
  // client->lock():
  unlock();
  if (usingReceiverThread) {
    if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
      *log << Log::NOTICE << "AEPClient::stop cancelling and joining "
           << "receiver thread " << receiverThread << "..." << ENDLOG;
    }
    pthread_cancel(receiverThread);
    pthread_join(receiverThread, NULL);
    if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
      *log << Log::NOTICE << "AEPClient::stop joined receiver thread "
           << receiverThread << "; sending ClientTerminateToken to and "
           << "joining ear thread " << earThread << "..." << ENDLOG;
    }
    receiverQueue->push(new ClientTerminateToken(
      *(new string("Terminated via AEPClient::stop()"))
    ));
    // Wait for the ear to finish running:
    pthread_join(earThread, NULL);
    if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
      *log << Log::NOTICE << "AEPClient::stop joined ear thread "
           << earThread << "." << ENDLOG;
    }
  } else {
    if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
      *log << Log::NOTICE << "AEPClient::stop cancelling and joining "
           << "ear thread " << earThread << "..." << ENDLOG;
    }
    pthread_cancel(earThread);
    pthread_join(earThread, NULL);
    if ((log != NULL)&&(logLevel >= Log::NOTICE)) {
      *log << Log::NOTICE << "AEPClient::stop joined ear thread "
           << earThread << "." << ENDLOG;
    }
  }
  sendUnlock();
}


AEPClient::ReplyToken* AEPClient::waitForReply
(long clientSequence, const char* location, bool terminate,
 pthread_cond_t* cond)
{
  ReplyToken* reply;

  if ((reply = clientSequenceBinderMap.wait(clientSequence, terminate, cond))
      == NULL) {
    ostringstream oss;

    oss << "ERROR: NULL reply Token for " << location
        << " (perhaps the client was killed with active participants joined)";
    throw AEPException(oss.str(), Log::CRITICAL);
  }
  return reply;
}


void AEPClient::earCleanupHandler
(void* clientAsVoidPointer)
{
  AEPClient* client = (AEPClient*)clientAsVoidPointer;

  if ((client->log != NULL)&&(client->logLevel >= Log::NOTICE)) {
    *(client->log) << Log::NOTICE << "AEPClient ear thread " << pthread_self()
                   << ": cancelled." << ENDLOG;
  }
}


void* AEPClient::earMain
(void* clientAsVoidPointer)
{
  AEPClient* client = (AEPClient*)clientAsVoidPointer;
  bool running = true;
  Token* token;
  bool haveLock;

  try {
    pthread_cleanup_push(earCleanupHandler, clientAsVoidPointer);
    if ((client->log != NULL)&&(client->logLevel >= Log::NOTICE)) {
      *(client->log) << Log::NOTICE << "AEPClient ear thread "
                     << pthread_self() << ": starting execution..." << ENDLOG;
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    try {
      // Wake up the start caller:
      if (client->usingReceiverThread) {
        pthread_mutex_lock(&(client->receiverQueueMutex));
        pthread_cond_signal(&(client->receiverQueueCondition));
      } else {
        client->lock();
        client->signal();
        client->unlock();
      }
      while (running) {
        if (client->usingReceiverThread) {
          while (client->receiverQueue->size() == 0) {
            // Ear waits for receiver to populate queue:
            pthread_cond_wait(
              &(client->receiverQueueCondition), &(client->receiverQueueMutex)
            );
          }
          if (client->receiverQueue->size() >= client->maxReceiveSize) {
            // Wake up the receiver:
            pthread_cond_signal(&(client->receiverQueueCondition));
          }
          token = client->receiverQueue->front();
          client->receiverQueue->pop();
          pthread_mutex_unlock(&(client->receiverQueueMutex));
          running = client->processToken(token);
        } else {
          if (!client->blockForAvailableData()) {
            pthread_mutex_lock(&(client->receptionMutex));
          }
          token = client->receive();
          if (token == NULL) {
            throw AEPException(
              "INTERNAL ERROR: Received NULL AEPClient::Token", Log::FATAL
            );
          }
          pthread_mutex_unlock(&(client->receptionMutex));
          running = client->processToken(token);
        }
      }
    } catch (exception& e) {
      if ((client->log != NULL)&&(client->logLevel >= Log::FATAL)) {
        *(client->log)
          << Log::FATAL
          << "FATAL ERROR: Caught exception at top of AEPClient ear thread "
          << pthread_self() << ": "
          << e.what() << ENDLOG;
      }
    } catch (...) {
      if ((client->log != NULL)&&(client->logLevel >= Log::FATAL)) {
        *(client->log)
          << Log::FATAL
          << "FATAL ERROR: Caught unknown type at top of AEPClient ear thread "
          << pthread_self() << ENDLOG;
      }
    }
    if ((client->log != NULL)&&(client->logLevel >= Log::NOTICE)) {
      *(client->log) << Log::NOTICE << "AEPClient ear thread "
                     << pthread_self() << ": END OF LINE." << ENDLOG;
    }
    pthread_cleanup_pop(0);
  } catch (...) {
  }
}


void AEPClient::receiverCleanupHandler
(void* clientAsVoidPointer)
{
  AEPClient* client = (AEPClient*)clientAsVoidPointer;

  if ((client->log != NULL)&&(client->logLevel >= Log::NOTICE)) {
    *(client->log) << Log::NOTICE << "AEPClient receiver thread "
                   << pthread_self() << ": cancelled." << ENDLOG;
  }
}


void* AEPClient::receiverMain
(void* clientAsVoidPointer)
{
  AEPClient* client = (AEPClient*)clientAsVoidPointer;
  bool running = true;

  try {
    pthread_cleanup_push(receiverCleanupHandler, clientAsVoidPointer);
    if ((client->log != NULL)&&(client->logLevel >= Log::NOTICE)) {
      *(client->log) << Log::NOTICE << "AEPClient receiver thread "
                     << pthread_self() << ": starting execution..." << ENDLOG;
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    while (running) {
      AEPClient::Token* token;

      if (!client->blockForAvailableData()) {
        pthread_mutex_lock(&(client->receptionMutex));
      }
      try {
        token = client->receive();
        if (token == NULL) {
          throw AEPException(
            "INTERNAL ERROR: Received NULL AEPClient::Token", Log::FATAL
          );
        }
      } catch (exception& e) {
        ostringstream oss;

        oss << "AEPClient receiver thread caught exception: " << e.what();
        client->receiverQueue->push(new ClientTerminateToken(
          *(new string(oss.str()))
        ));
        if ((client->log != NULL)&&(client->logLevel >= Log::ERROR)) {
          *(client->log) << Log::ERROR << oss.str() << ENDLOG;
        }
        running = false;
      } catch (...) {
        char* message = "AEPClient receiver thread caught unknown exception";

        client->receiverQueue->push(new ClientTerminateToken(
          *(new string(message))
        ));
        if ((client->log != NULL)&&(client->logLevel >= Log::ERROR)) {
          *(client->log) << Log::ERROR << message << ENDLOG;
        }
        running = false;
      }
      pthread_mutex_unlock(&(client->receptionMutex));
      if (running) {
        pthread_mutex_lock(&(client->receiverQueueMutex));
        client->receiverQueue->push(token);
        // Wake up the ear, if it's waiting:
        pthread_cond_signal(&(client->receiverQueueCondition));
        while (client->receiverQueue->size() >= client->maxReceiveSize) {
          // We wait for the ear to signal us that it has processed the queue,
          // and we log the "overflow":
          if ((client->log != NULL)&&(client->logLevel >= Log::WARN)) {
            *(client->log)
              << Log::WARN << "Maximum receiver queue size ("
              << client->maxReceiveSize
              << ") reached... suspending receiver..." << ENDLOG;
          }
          pthread_cond_wait(
            &(client->receiverQueueCondition), &(client->receiverQueueMutex)
          );
          if ((client->log != NULL)&&(client->logLevel >= Log::WARN)) {
            *(client->log) << Log::WARN << "Receiver resumed." << ENDLOG;
          }
        }
        pthread_mutex_unlock(&(client->receiverQueueMutex));
      }
    }
    pthread_cleanup_pop(0);
  } catch (...) {
  }
}


bool AEPClient::processToken
(Token* token)
{
  bool keepRunning = false;

  lock();
  // AEPExceptions thrown during token handling may not terminate the session,
  // if the client is error tolerant:
  try {
    token->setClient(*this);
    if ((log != NULL)&&(logLevel >= Log::DEBUG)) {
      *log << Log::DEBUG << "AEPClient: Received token: ";
      token->print(*log, logLevel);
      *log << ENDLOG;
    }
    try {
      serverSequence = token->getServerSequence();
      switch (token->getType()) {
      case AEPClient::Token::VOID:
        throw AEPException(
          "INTERNAL ERROR: Received VOID AEPClient::Token", Log::FATAL
        );
      case AEPClient::Token::ACK:
      case AEPClient::Token::DENY:
      case AEPClient::Token::ERROR:
        keepRunning = clientSequenceBinderMap.resume(*((ReplyToken*)token));
        serverSequenceBinderMap.
          resumeUntil(((ReplyToken*)token)->getServerSequence());
        break;
      case AEPClient::Token::CLIENT_TERMINATE:
      case AEPClient::Token::SERVER_DISCONNECT:
      case AEPClient::Token::NOTIFY:
        keepRunning = token->handle();
        delete token;
        break;
      default:
        {
          ostringstream oss;

          oss << "INTERNAL ERROR: Got bad AEPClient::Token type int "
              << token->getType();
          throw AEPException(oss.str(), Log::FATAL);
        }
      }
    } catch (AEPException& e) {
      if ((log != NULL)&&(logLevel >= e.getPriority())) {
        *log << e.getPriority() << "Caught AEPException in processing of "
             << "AEPClient::Token: " << e.what() << ENDLOG;
      }
      if ((!errorTolerant)||(e.getPriority() <= Log::FATAL)) {
        throw e;
      }
    } catch (IntenseException& e) {
      if ((log != NULL)&&(logLevel >= Log::CRITICAL)) {
        *log << Log::CRITICAL
             << "Caught IntenseException in processing of "
             << "AEPClient::Token: " << e.what() << ENDLOG;
      }
      throw e;
    } catch (exception& e) {
      if ((log != NULL)&&(logLevel >= Log::CRITICAL)) {
        *log << Log::CRITICAL
             << "Caught std::exception in processing of "
             << "AEPClient::Token: " << e.what() << ENDLOG;
      }
      throw e;
    } catch (...) {
      if ((log != NULL)&&(logLevel >= Log::CRITICAL)) {
        *log << Log::CRITICAL
             << "Caught unknown object in processing of "
             << "AEPClient::Token" << ENDLOG;
      }
      throw;
    }
    unlock();
    return keepRunning;
  } catch (...) {
    unlock();
  }
}


bool AEPStreamClient::blockForAvailableData
()
{
  // Linuxthreads non-POSIX read block cancellation point workaround:
  pthread_testcancel();
  in->peek();
  pthread_testcancel();
  return false;
}


AEPStreamClient* AEPStreamClient::connect
(istream& is, ostream& os, Mode mode, Log* log, Log::Priority logLevel)
{
  AEPStreamClient* client = NULL;
  char buffer[128];
  vector<string> tokens;
  const char* serverBinaryArch =
    BinaryBaseSerialiser::compatibleArchitectureString();

  os << "AEP 2.0 CONNECT " << getModeTypeString(mode);
  if (mode == BINARY) {
    os << ' ' << BinaryBaseSerialiser::compatibleArchitectureString();
  }
  os << "\n\n";
  os.flush();
  is.getline(buffer, 128, '\n');
  stringtok(tokens, buffer);
  // We are looking for an initial line of the form:
  // AEP x.y CONNECT BINARY
  // AEP x.y CONNECT XDR
  // AEP x.y CONNECT TEXT
  // ...followed by an empty line:
  is.getline(buffer, 128, '\n');
  if ((tokens.size() != 4)||(tokens[0] != "AEP")||
      (tokens[2] != "CONNECT")) {
    ostringstream oss;

    oss << "Bad AEP connect string \"" << buffer
        << "\" in AEP connect response";
    throw AEPException(oss.str());
  } else if (tokens[1] != "2.0") {
    // This should really be a version number check:
    ostringstream oss;

    oss << "Bad AEP version string \"" << tokens[1]
        << "\" in AEP connect response";
    throw AEPException(oss.str());
  } else if (tokens[3] == modeTypeStrings[BINARY]) {
    client = new AEPBinaryClient(is, os, log, logLevel, true);
  } else if (tokens[3] == modeTypeStrings[XDR]) {
    client = new AEPXDRClient(is, os, log, logLevel, true);
  } else if (tokens[3] == modeTypeStrings[TEXT]) {
    client = new AETPClient(is, os, log, logLevel, true);
  } else {
    // This should really be a version number check:
    ostringstream oss;

    oss << "Bad AEP mode string \"" << tokens[3]
        << "\" in AEP connect response";
    throw AEPException(oss.str());
  }
  client->start();
  return client;
}


AEPStreamClient* AEPStreamClient::connect
(istream& in, ostream& out, Mode mode, AEPClient::Participant& participant,
 const CompoundDimension* dimension, pthread_cond_t* optionalBlockCondition,
 Log* log, Log::Priority logLevel)
{
  AEPStreamClient* client;

  client = connect(in, out, mode, log, logLevel);
  participant.join(*client, true, dimension, optionalBlockCondition);
  return client;
}


// Non-NULL dimension string:
AEPStreamClient* AEPStreamClient::connect
(istream& in, ostream& out, Mode mode, AEPClient::Participant& participant,
 const char* compoundDimensionString, pthread_cond_t* optionalBlockCondition,
 Log* log, Log::Priority logLevel)
{
  AEPStreamClient* client;
  CompoundDimension compoundDimension(compoundDimensionString);

  client = connect(in, out, mode, log, logLevel);
  participant.join(*client, true, &compoundDimension, optionalBlockCondition);
  return client;
}


// Connects to the given (aep://host:port/full:compound:dimension) AEP
// URL, using the given AEPClient::Participant.
AEPStreamClient* AEPStreamClient::urlConnect
(const char* aepURL, FileStream& destStream,
 AEPClient::Participant& participant, pthread_cond_t* optionalBlockCondition,
 Log* log, Log::Priority logLevel)
{
  Mode mode;
  string host;
  int port;
  CompoundDimension dimension;
  FileStream* stream;
  AEPStreamClient* client;

  parseURL(aepURL, mode, host, port, dimension);
  streamConnect(destStream, host, port);
  client = connect(destStream, destStream, mode, log, logLevel);
  participant.join(*client, true, &dimension, optionalBlockCondition);
  return client;
}


// Connects to the given (aep://host:port/ignored:dimension) AEP
// URL (ignoring any dimension component of the URL):
AEPStreamClient* AEPStreamClient::urlConnect
(const char* aepURL, FileStream& destStream, Log* log, Log::Priority logLevel)
{
  Mode mode;
  string host;
  int port;
  CompoundDimension dimension;
  AEPStreamClient* client;

  parseURL(aepURL, mode, host, port, dimension);
  streamConnect(destStream, host, port);
  client = connect(destStream, destStream, mode, log, logLevel);
  return client;
}


void AEPStreamClient::streamConnect
(FileStream& destStream, string& host, int port)
{
  struct sockaddr_in localAddress, serverAddress;
  struct hostent* serverHostent;
  int sock;

  serverHostent = gethostbyname(host.data());
  if (serverHostent == NULL) {
    ostringstream oss;

    oss << "AEPStreamClient::streamConnect: gethostbyname_r() error: "
        << strerror(errno);
    throw AEPException(oss.str());
  }
  serverAddress.sin_family = serverHostent->h_addrtype;
  memcpy(
    (char *)&serverAddress.sin_addr.s_addr, serverHostent->h_addr_list[0],
    serverHostent->h_length
  );
  serverAddress.sin_port = htons(port);
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    ostringstream oss;
    oss << "AEPStreamClient::streamConnect: socket() error: "
        << strerror(errno);
    throw AEPException(oss.str());
  }
  localAddress.sin_family = AF_INET;
  localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddress.sin_port = htons(0);
  if (bind(sock, (struct sockaddr*)&localAddress, sizeof(localAddress)) < 0) {
    ostringstream oss;

    oss << "AEPStreamClient::connect: bind() error: " << strerror(errno);
    throw AEPException(oss.str());
  }
  if (::connect(sock, (sockaddr*)&serverAddress, sizeof(serverAddress)) != 0) {
    ostringstream oss;

    oss << "AEPStreamClient::connect: connect() error: " << strerror(errno);
    oss << "\nhost = \"" << host << "\"; port = " << port;
    throw AEPException(oss.str());
  }
  destStream.attach(sock);
}


AEPStreamClient::~AEPStreamClient
()
{
}


AEPSerialClient::AEPSerialClient
(istream& in, ostream& out, BaseSerialiser& serialiser_, Log* log,
 Log::Priority logLevel, bool errorTolerant, bool useReceiverThread,
 int maxReceiveSize)
  : AEPStreamClient(
      in, out, log, logLevel, errorTolerant, useReceiverThread, maxReceiveSize
    ),
    serialiser(&serialiser_)
{}


AEPSerialClient::~AEPSerialClient
()
{}


void AEPSerialClient::send
(AEPCommon::Token& token)
{
  Token::factorySerialise((Token*)&token, *serialiser, *out);
}


AEPClient::Token* AEPSerialClient::receive
()
{
  return Token::factoryDeserialise(*serialiser, *in);
}


AEPBinaryClient::AEPBinaryClient
(istream& in, ostream& out, Log* log, Log::Priority logLevel,
 bool errorTolerant, bool useReceiverThread, int maxReceiveSize)
  : AEPSerialClient(
      in, out, *(new BinaryBaseSerialiser), log, logLevel, errorTolerant,
      useReceiverThread, maxReceiveSize
    )
{}


AEPBinaryClient::~AEPBinaryClient
()
{
  // Created in AEPBinaryClient constructor:
  delete serialiser;
}


AEPXDRClient::AEPXDRClient
(istream& in, ostream& out, Log* log, Log::Priority logLevel,
 bool errorTolerant, bool useReceiverThread, int maxReceiveSize)
  : AEPSerialClient(
      in, out, *(new XDRBaseSerialiser), log, logLevel, errorTolerant,
      useReceiverThread, maxReceiveSize
    )
{}


AEPXDRClient::~AEPXDRClient
()
{
  // Created in AEPXDRClient constructor:
  delete serialiser;
}
