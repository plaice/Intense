// ****************************************************************************
//
// AEther.cpp : Contexts with participants.
//
// Copyright 2001, 2002 Paul Swoboda.
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
#include <pthread.h>
#include <list>
#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <intense-io.hpp>
#include "IntenseException.hpp"
#include "BoundManager.hpp"
#include "Context.hpp"
#include "AEther.hpp"
#include "StringBaseValue.hpp"
#include "BinaryBaseValue.hpp"
#include "BoundBaseValue.hpp"
#include "correct_FlexLexer.h"
#include "ContextOpLexer.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;


extern Context emptyContext;


AEtherParticipant::~AEtherParticipant
()
{
  leave();
}


void AEtherParticipant::leave
()
{
  if (aether != NULL) {
    aether->detach(location);
    aether = NULL;
  }
}


void AEtherParticipant::join
(AEther& a)
{
  leave();
  aether = &a;
  location = a.attach(*this);
}


long long int AEtherParticipant::assign
(const Context& context, const CompoundDimension* dim)
{
  if (aether != NULL) {
    if (dim == NULL) {
      aether->assign(context, this);
    } else {
      (*aether)[*dim].assign(context, this);
    }
  } else {
    throw IntenseException("Attempt to assign in unjoined Participant");
  }
  return 0;
}


long long int AEtherParticipant::apply
(const ContextOp& op, const CompoundDimension* dim)
{
  if (aether == NULL) {
    if (dim == NULL) {
      aether->apply(op, this);
    } else {
      (*aether)[*dim].apply(op, this);
    }
  } else {
    throw IntenseException("Attempt to apply in unjoined Participant");
  }
  return 0;
}


long long int AEtherParticipant::clear
(const CompoundDimension* dim)
{
  if (aether == NULL) {
    if (dim == NULL) {
      aether->clear(this);
    } else {
      (*aether)[*dim].clear(this);
    }
  } else {
    throw IntenseException("Attempt to clear in unjoined Participant");
  }
  return 0;
}


AEther& AEtherParticipant::getNode
()
{
  if (aether == NULL) {
    throw IntenseException(
      "Attempt to retrieve reference to NULL AEther node in "
      "AEtherParticipant::getNode"
    );
  }
  return *aether;
}


// Depth-first deletion with participant "kick" notification and deletion:
AEther::~AEther
()
{
  kick(false);
}


list<AEtherParticipant*>::iterator AEther::attach
(AEtherParticipant& p)
{
  AEther* aetherMarker;

  participants.push_front(&p);
  for (aetherMarker = this; aetherMarker;
       aetherMarker = (AEther*)aetherMarker->parent) {
    aetherMarker->headcount++;
  }
  return participants.begin();
}


void AEther::detach
(list<AEtherParticipant*>::iterator itr)
{
  AEther* aetherMarker;

  participants.erase(itr);
  for (aetherMarker = this; aetherMarker;
       aetherMarker = (AEther*)aetherMarker->parent) {
    aetherMarker->headcount--;
  }
}


AEther& AEther::assign
(const Context& c, Origin* origin)
{
  int deltaBasecount;
  Context* contextMarker;

  deltaBasecount = ((AEther&)c).basecount - basecount;
  clearNoPropagate(origin);
  for (contextMarker = (Context*)parent; contextMarker != NULL;
       contextMarker = (Context*)(((AEther*)contextMarker)->parent)) {
    ((AEther*)contextMarker)->basecount += deltaBasecount;
    ((AEther*)contextMarker)->clearCanonicalString();
  }
  propagateAssignNotify(c, origin);
  assignHelper(c, origin);
  return *this;
}


AEther& AEther::apply
(const ContextOp& op, Origin* origin)
{
  Context* contextMarker;
  int deltaBasecount;

  deltaBasecount = basecount;
  propagateApplyNotify(op, origin);
  applyHelper(op, origin);
  deltaBasecount = basecount - deltaBasecount;
  for (contextMarker = (Context*)parent; contextMarker != NULL;
       contextMarker = (Context*)(((AEther*)contextMarker)->parent)) {
    ((AEther*)contextMarker)->basecount += deltaBasecount;
    ((AEther*)contextMarker)->clearCanonicalString();
  }
  clearCanonicalString();
  return *this;
}


void AEther::clear
(Origin* origin)
{
  bool hadBasecount = basecount > 0;
  int tempBasecount = basecount;

  // Redefined in AEther:
  clearNoPropagate(origin);
  if (hadBasecount) {
    propagateBasecount(-tempBasecount);
    propagateClearNotify(origin);
  }
}


// Recursive selective aether pruner, that clears out all base values in and
// beneath *this.  Ancestors are not notified of the basecount delta.  All
// branches without participants are pruned.
void AEther::clearNoPropagate
(Origin* origin)
{
  bool hadBasecount = basecount > 0;

  if (headcount == 0) {
    Context::clearNoPropagate();
    return;
  } else {
    ContextMap::iterator itr = begin();

    basecount = 0;
    clearCanonicalString();
    if (base != NULL) {
      deleteBasePtr();
    }
    while (itr != end()) {
      if (((AEther*)(itr->second))->headCount() > 0) {
        ((AEther*)(itr++->second))->clearNoPropagate(origin);
      } else {
        ContextMap::iterator tempItr = itr;

        tempItr++;
        delete itr->second;
        erase(itr);
        itr = tempItr;
      }
    }
    if (hadBasecount&&(participants.size() > 0)) {
      clearNotify(origin);
    }
  }
}


void AEther::clear
(const CompoundDimension& dim, Origin* origin)
{
  if (dim.size() > 0) {
    list<DimensionRef>::iterator dimItr, lastItr;
    ContextMap::iterator itr;
    Context* marker = this;
    int deltaBasecount;

    dimItr = ((CompoundDimension&)dim).begin();
    lastItr = ((CompoundDimension&)dim).end();
    lastItr--;
    while (dimItr != lastItr) {
      itr = ((AEther*)marker)->ContextMap::find(*dimItr);
      if (itr == ((AEther*)marker)->ContextMap::end()) {
        return;
      }
      marker = &(((AEther*)marker)->valueRef(*dimItr++));
    }
    itr = ((AEther*)marker)->ContextMap::find(*dimItr);
    if (itr != ((AEther*)marker)->ContextMap::end()) {
      deltaBasecount = -itr->second->baseCount();
      basecount += deltaBasecount;
      propagateBasecount(deltaBasecount);
      checkedClearAndPropagateNotify(itr, origin);
    }
  } else {
    // Perhaps EVIL, but it's essentially what they asked for:
    clear(origin);
  }
}


// Utility, for use by Context::applyHelper, to clear a dimension behind a
// given iterator.  In a subclass such as AEther, this can be overridden, to
// avoid wiping out certain dimensions (ie those with a headcount, for
// AEthers).
void AEther::checkedClear
(ContextMap::iterator& itr, Origin* origin)
{
  if (((AEther*)(itr->second))->headcount > 0) {
    ((AEther*)(itr++->second))->clearNoPropagate(origin);
  } else {
    delete itr->second;
    erase(itr);
  }
}


void AEther::checkedClearAndPropagateNotify
(ContextMap::iterator& itr, Origin* origin)
{
  int initialBasecount = ((AEther*)itr->second)->basecount;

  if (((AEther*)(itr->second))->headcount > 0) {
    ((AEther*)(itr->second))->clearNoPropagate(origin);
    if (initialBasecount != 0) {
      ((AEther*)itr->second)->propagateClearNotify(origin);
    }
  } else {
    if (initialBasecount != 0) {
      ContextMap::iterator childItr;

      for (childItr = itr->second->begin(); childItr != itr->second->end();
           childItr++) {
        delete childItr->second;
      }
      ((AEther*)itr->second)->ContextMap::clear();
      if (((AEther*)(itr->second))->base != NULL) {
        ((AEther*)(itr->second))->deleteBasePtr();
      }
      ((AEther*)itr->second)->basecount = 0;
      ((AEther*)itr->second)->propagateClearNotify(origin);
    }
    delete itr->second;
    erase(itr);
  }
}


void AEther::applyNotify
(const ContextOp& o, Origin* origin)
{
  list<AEtherParticipant*>::iterator participantMarker;

  for (participantMarker = participants.begin();
       participantMarker != participants.end();
       participantMarker++) {
    (*participantMarker)->applyNotify(o, NULL, origin);
  }
}


void AEther::propagateApplyNotify
(const ContextOp& o, Origin* origin)
{
  if (parent != NULL) {
    AEther* aetherMarker;
    CompoundDimension totalDim;
    list<AEtherParticipant*>::iterator participantMarker;

    totalDim.append(*parentDimension);
    for (aetherMarker = (AEther*)this->parent; aetherMarker;
         aetherMarker = (AEther*)aetherMarker->parent) {
      for (participantMarker = aetherMarker->participants.begin();
           participantMarker != aetherMarker->participants.end();
           participantMarker++) {
        (*participantMarker)->applyNotify(o, &totalDim, origin);
      }
      if (aetherMarker->parent) {
        totalDim.prepend(*(aetherMarker->parentDimension));
      }
    }
  }
}


void AEther::assignNotify
(const Context& context, Origin* origin)
{
  list<AEtherParticipant*>::iterator participantMarker;

  for (participantMarker = participants.begin();
       participantMarker != participants.end();
       participantMarker++) {
    assignNotify(**participantMarker, context, NULL, origin);
  }
}


void AEther::assignNotify
(Participant& participant, const Context& context,
 const CompoundDimension* dimension, Origin* origin)
{
  if (participant.isPure()) {
    // Participant notification via operator:
    ContextOp op;

    op.apply(context);
    participant.applyNotify(op, dimension, origin);
  } else {
    participant.assignNotify(context, dimension, origin);
  }
}


void AEther::propagateAssignNotify
(const Context& context, Origin* origin)
{

  if (parent != NULL) {
    AEther* aetherMarker;
    CompoundDimension totalDim;
    list<AEtherParticipant*>::iterator participantMarker;

    totalDim.append(*parentDimension);
    for (aetherMarker = (AEther*)this->parent; aetherMarker;
         aetherMarker = (AEther*)aetherMarker->parent) {
      for (participantMarker = aetherMarker->participants.begin();
           participantMarker != aetherMarker->participants.end();
           participantMarker++) {
        assignNotify(**participantMarker, context, &totalDim, origin);
      }
      if (aetherMarker->parent) {
        totalDim.prepend(*(aetherMarker->parentDimension));
      }
    }
  }
}


void AEther::clearNotify
(Origin* origin)
{
  list<AEtherParticipant*>::iterator participantMarker;

  for (participantMarker = participants.begin();
       participantMarker != participants.end();
       participantMarker++) {
    clearNotify(**participantMarker, NULL, origin);
  }
}


void AEther::clearNotify
(Participant& participant, const CompoundDimension* dimension, Origin* origin)
{
  if (participant.isPure()) {
    // Participant notification via operator:
    static ContextOp clearOp("[---]");

    participant.applyNotify(clearOp, dimension, origin);
  } else {
    participant.clearNotify(dimension, origin);
  }
}


void AEther::propagateClearNotify
(Origin* origin)
{
  if (parent != NULL) {
    AEther* aetherMarker;
    CompoundDimension totalDim;
    list<AEtherParticipant*>::iterator participantMarker;

    totalDim.append(*parentDimension);
    for (aetherMarker = (AEther*)this->parent; aetherMarker;
         aetherMarker = (AEther*)aetherMarker->parent) {
      for (participantMarker = aetherMarker->participants.begin();
           participantMarker != aetherMarker->participants.end();
           participantMarker++) {
        clearNotify(**participantMarker, &totalDim, origin);
      }
      if (aetherMarker->parent) {
        totalDim.prepend(*(aetherMarker->parentDimension));
      }
    }
  }
}


void AEther::setBase
(const BaseValue& bv, Origin* origin)
{
  ContextOp tempOp;

  Context::setBase(bv);
  tempOp.setBase(bv);
  propagateApplyNotify(tempOp, origin);
  applyNotify(tempOp, origin);
#if DEFINITELY_HAVE_POSIX_SPINLOCKS != 1
  // This is a trick that we can avoid in the Java implementation - we don't
  // want tempOp to delete bv when its destructor is called.  UPDATE: Only
  // necessary if this is a basevalue-cloning build:
  ((AEther&)tempOp).base = NULL;
  ((AEther&)tempOp).basecount = 0;
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS != 1
}


void AEther::setBase
(const BaseValue* bv, Origin* origin)
{
  if (bv == NULL) {
    throw IntenseException("Attempt to set NULL base value in Context");
  }
  setBase(*bv, origin);
}


void AEther::clearBase
(Origin* origin)
{
  bool hadBase = base != NULL;

  Context::clearBase();
  if (hadBase) {
    ContextOp tempOp;

    tempOp.setClearBaseFlag();
    propagateApplyNotify(tempOp, origin);
    applyNotify(tempOp, origin);
  }
}


// Kick participants in this AEther.
void AEther::kick
(bool kickInChildNodes, Origin* origin)
{
  int tempHeadcount = headcount;
  AEther* marker;

  if (headcount == 0) {
    return;
  }
  if (participants.size() > 0) {
    list<AEtherParticipant*>::iterator participantMarker;

    for (participantMarker = participants.begin();
         participantMarker != participants.end();
         participantMarker++) {
      AEtherParticipant& participant = **participantMarker;

      participant.aether = NULL;
      participant.kickNotify(origin);
    }
  }
  if (kickInChildNodes) {
    ContextMap::iterator itr = begin();

    while (itr != end()) {
      if (((AEther*)(itr->second))->headCount() > 0) {
        ((AEther*)(itr++->second))->kick(true, origin);
      }
      itr++;
    }
  }
  headcount = 0;
  for (marker = (AEther*)parent; marker != NULL;
       marker = (AEther*)((AEther*)marker)->parent) {
    ((AEther*)marker)->headcount -= tempHeadcount;
  }
}


void AEther::parseStream
(istream& is, Origin* origin)
{
  ContextOpLexer lexer(is);
  int initialBasecount = basecount;

  clearNoPropagate(origin);
  recogniseNode(lexer);
  if (basecount != initialBasecount) {
    propagateBasecount(basecount - initialBasecount);
  }
  propagateAssignNotify(*this, origin);
}


void AEther::parse
(const char* input, Origin* origin)
{
  istringstream inputStream(input);

  parse(inputStream, origin);
}


void AEther::deserialise
(BaseSerialiser& baseSerialiser, istream& is, BoundManager* boundManager,
 Origin* origin)
{
  int initialBasecount = basecount;

  clear(origin);
  deserialiseHelper(baseSerialiser, is, boundManager, origin);
  propagateBasecount(basecount - initialBasecount);
  propagateAssignNotify(*this, origin);
}
