// ****************************************************************************
//
// Context.cpp : An intensional versioning class hierarchy.
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
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <intense-io.hpp>
#include "intense_config.hpp"
#include "IntenseException.hpp"
#include "BoundManager.hpp"
#include "Context.hpp"
#include "AEther.hpp"
#include "ContextDomain.hpp"
#include "container_stringtok.hpp"
#include "StringBaseValue.hpp"
#include "BinaryBaseValue.hpp"
#include "BoundBaseValue.hpp"
#include "correct_FlexLexer.h"
#include "ContextOpLexer.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;


const Context emptyContext();


string StringDimension::canonical
() const
{
  if (requiresQuotes) {
    ostringstream oss;
    string& value = ((StringDimension*)this)->value;
    string::iterator itr;

    oss << '"';
    for (itr = value.begin(); itr != value.end(); itr++) {
      switch (*itr) {
      case '\\':
        oss << "\\\\";
        break;
      case '\n':
        oss << "\\n";
        break;
      case '\t':
        oss << "\\t";
        break;
      case '\v':
        oss << "\\v";
        break;
      case '\b':
        oss << "\\b";
        break;
      case '\r':
        oss << "\\r";
        break;
      case '\f':
        oss << "\\f";
        break;
      case '\a':
        oss << "\\a";
        break;
      case '"':
        oss << "\\\"";
        break;
      default:
        oss << *itr;
        break;
      }
    }
    oss << '"';
    return oss.str();
  } else {
    return value;
  }
}


void StringDimension::serialise
(BaseSerialiser& baseSerialiser, ostream& os) const
{
  baseSerialiser.out(requiresQuotes, os);
  baseSerialiser.out(value, os);
}


void StringDimension::deserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  baseSerialiser.in(requiresQuotes, is);
  baseSerialiser.in(value, is, maxLength);
}



const char* Context::typeStrings[] = {
  "VOID", "CONTEXT", "CONTEXTOP", "AETHER"
};


Context* Context::factory
(Type type)
{
  switch (type) {
  case VOID:
    return NULL;
  case CONTEXT:
    return new Context();
  case CONTEXTOP:
    return new ContextOp();
  case AETHER:
    return new AEther();
  default:
    ostringstream oss;

    oss << "INTERNAL ERROR: Bad Context type " << type;
    throw IntenseException(oss.str());
  }
}


void Context::factorySerialise
(const Context* context, BaseSerialiser& baseSerialiser, ostream& os)
{
  int type;

  if (context != NULL) {
    type = ((Context*)context)->getType();
  } else {
    type = Context::VOID;
  }
  baseSerialiser.out(type, os);
  if (context != NULL) {
    context->serialise(baseSerialiser, os);
  }
}


Context* Context::factoryDeserialise
(BaseSerialiser& baseSerialiser, istream& is)
{
  int type;
  Context* context;

  baseSerialiser.in(type, is);
  context = Context::factory((Type)type);
  if (context != NULL) {
    context->deserialise(baseSerialiser, is);
  }
  return context;
}


Context* Context::findNode
(CompoundDimension& dimension)
{
  Context* marker = this;
  std::list<DimensionRef>::iterator dimensionItr;

  for (dimensionItr = dimension.begin(); dimensionItr != dimension.end();
       dimensionItr++) {
    ContextMap::iterator itr = ((ContextMap*)marker)->find(*dimensionItr);

    if (itr != ((ContextMap*)marker)->end()) {
      marker = itr->second;
    } else {
      return NULL;
    }
  }
  return marker;
}


Context* Context::findNode
(const char* dimension)
{
  CompoundDimension compoundDimension(dimension);

  return findNode(compoundDimension);
}


void Context::clear
()
{
  bool hadBasecount = basecount > 0;
  int tempBasecount = basecount;

  clearNoPropagate();
  if (hadBasecount) {
    propagateBasecount(-tempBasecount);
  }
}


Context::~Context
()
{
  ContextMap::iterator itr;

  clearCanonicalString();
  if (base != NULL) {
    deleteBasePtr();
  }
  for (itr = begin(); itr != end(); itr++) {
    if (itr->second) {
      delete itr->second;
    }
  }
}


int Context::getDepth
()
{
  Context* marker;
  int depth = 0;
  
  for (marker = this; marker != NULL; marker = (Context*)(marker->parent)) {
    depth++;
  }
  return depth;
}


Context& Context::leastCommonRoot
(Context& node)
{
  int depth = getDepth();
  int nodeDepth = node.getDepth();
  int minDepth;
  Context* marker = this;
  Context* nodeMarker = &node;
  int i;

  if (depth < nodeDepth) {
    minDepth = depth;
    for (int i = 0; i < (nodeDepth - depth); i++) {
      nodeMarker = (Context*)(nodeMarker->parent);
    }
  } else {
    minDepth = nodeDepth;
    for (int i = 0; i < (depth - nodeDepth); i++) {
      marker = (Context*)(marker->parent);
    }
  }
  while (marker != NULL) {
    if (marker == nodeMarker) {
      return *marker;
    }
    marker = (Context*)(marker->parent);
    nodeMarker = (Context*)(nodeMarker->parent);
  }
  throw IntenseException(
    "Attempt to find common ancestor of unrelated Context nodes"
  );
}


void Context::clearNoPropagate
()
{
  ContextMap::iterator itr;

  basecount = 0;
  clearCanonicalString();
  if (base != NULL) {
    deleteBasePtr();
  }
  for (itr = begin(); itr != end(); itr++) {
    delete itr->second;
  }
  ContextMap::clear();
}


void Context::dimTok
(list<string>& dimList, const string& dim)
{
  stringtok(dimList, dim, ":");
}


void Context::dimTok
(vector<string>& dimList, const string& dim)
{
  stringtok(dimList, dim, ":");
}


void Context::clear
(const CompoundDimension& dim)
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
      itr = marker->ContextMap::find(*dimItr);
      if (itr == marker->ContextMap::end()) {
        return;
      }
      marker = &marker->valueRef(*dimItr++);
    }
    itr = marker->ContextMap::find(*dimItr);
    if (itr != marker->ContextMap::end()) {
      deltaBasecount = -itr->second->baseCount();
      basecount += deltaBasecount;
      propagateBasecount(deltaBasecount);
      checkedClear(itr, NULL);
    }
  } else {
    // Perhaps EVIL, but it's essentially what they asked for:
    clear();
  }
}


// context equality test:
bool Context::equals
(const Context& c) const
{
  if (basecount != c.basecount) {
    return false;
  } else {
    return compare(c) == 0;
  }
}


// Refines-to partial ordering:
bool Context::refinesTo
(const Context& c) const
{
  ContextMap::iterator itr;

  if (isEmpty()) {
    return true;
  } else if (c.isEmpty()) {
    return false;
  }
  if ((base != NULL)&&((c.base == NULL)||((c.base != NULL)&&
                                          (!(*base <= *(c.base)))))) {
    return false;
  }
  for (itr = ((Context*)this)->begin(); itr != ((Context*)this)->end();
       itr++) {
    if (itr->second->isEmpty()) {
      continue;
    } else if ((((Context&)c).mapRef().count(itr->first) == 0)||
               (!(*(((Context*)this)->mapRef()[itr->first]) <=
                  *(((Context&)c).mapRef()[itr->first])))) {
      return false;
    }
  }
  return true;
}


// Lexicographic comparison:
// Returns:
// -1 : *this < c
// 0  : *this == c
// 1 : *this > c
int Context::compare
(const Context& c) const
{
  ContextMap::iterator itr, cItr;
  int rc;

  // Probable speed enhancement:
  /*
    if (isEmpty()) {
    if (c.isEmpty()) return 0;
    else return -1;
    } else if (c.isEmpty())
    return 1;
  */
  if (base != NULL) {
    if (c.base != NULL) {
      if (*base < *c.base) {
        return -1;
      } else if (*c.base < *base) {
        return 1;
      }
      // *base == *c.base, so carry on to subdimensions...
    } else {
      // *base > empty:
      return 1;
    }
  } else if (c.base != NULL) {
    // empty < *c.base
    return -1;
  }
  itr = ((Context*)this)->begin();
  // Advance the iterators to the first non-empty dimensions:
  while ((itr != ((Context*)this)->end())&&(itr->second->isEmpty())) {
    itr++;
  }
  cItr = ((Context&)c).begin();
  while ((cItr != ((Context&)c).end())&&(cItr->second->isEmpty())) {
    cItr++;
  }
  while (itr != ((Context*)this)->end()) {
    if (cItr == ((Context&)c).end()) {
      return 1; // c < *this
    }
    if (itr->first < cItr->first) {
      return -1; // c < *this
    }
    if (cItr->first < itr->first) {
      return 1; // *this < c
    }
    // Recurse and terminate on non-equality:
    if ((rc = itr->second->compare(*(cItr->second)))) {
      return rc;
    }
    // Advance the iterators to the next non-empty dimensions:
    while ((++itr != ((Context*)this)->end())&&(itr->second->isEmpty()));
    while ((++cItr != ((Context&)c).end())&&(cItr->second->isEmpty()));
  }
  // Everything's been the same so far:
  if ((cItr != ((Context&)c).end())&&(basecount != c.basecount)) {
    return -1;
  }
  return 0;
}


// Context assignment, copying all flags and including branches without
// basecounts (resulting lvalue should be identical in structure and
// content to rvalue):
Context& Context::assign
(const Context& c)
{
  int deltaBasecount;
  Context* contextMarker;

  deltaBasecount = c.basecount - basecount;
  clearNoPropagate();
  for (contextMarker = (Context*)parent; contextMarker != NULL;
       contextMarker = (Context*)(contextMarker->parent)) {
    contextMarker->basecount += deltaBasecount;
    contextMarker->clearCanonicalString();
  }
  assignHelper(c, NULL);
  return *this;
}


void Context::assignHelper
(const Context& c, Origin* origin)
{
  ContextMap::iterator cItr;

  basecount = c.basecount;
  dupBasePtr(c.base);
  assignNotify(c, origin);
  for (cItr = ((Context&)c).begin(); cItr != ((Context&)c).end(); cItr++) {
    if (cItr->second->basecount > 0) {
      valueRef(cItr->first).assignHelper(*(cItr->second), origin);
    }
  }
}


// Apply op as a context operation:
Context& Context::apply
(const ContextOp& op)
{
  Context* contextMarker;
  int deltaBasecount;

  deltaBasecount = basecount;
  applyHelper(op, NULL);
  deltaBasecount = basecount - deltaBasecount;
  for (contextMarker = (Context*)parent; contextMarker != NULL;
       contextMarker = (Context*)(contextMarker->parent)) {
    contextMarker->basecount += deltaBasecount;
    contextMarker->clearCanonicalString();
  }
  clearCanonicalString();
  return *this;
}


// Record total delta resulting from the application of op to this context, as
// a minimal op, into delta (i.e., applying the resulting delta to *this yields
// the same context as applying op).  Note, this is really just op, minus any
// unnecessary blanking or pruning.
void Context::recordDelta
(const ContextOp& op, ContextOp& delta) const
{
  delta.clear();
  recordDeltaHelper(this, op, delta);
}


Context::OpHelperRC Context::recordDeltaHelper
(const Context* c, const ContextOp& op, ContextOp& delta)
{
  ContextMap::iterator cItr;
  ContextMap::iterator opItr;
  ContextOp::OpHelperRC countDeltas;
  bool thisContainsDimensionsNotInOp = false;
  ContextOp::OpHelperRC returnValue;
  ContextOp::OpHelperRC rc;

  returnValue.deltaBasecount = returnValue.deltaBlankcount = 0;
  if (c == NULL) {
    // First of all, if there is no context at this point, all we care about
    // are basevalues in op:
    if (op.base != NULL) {
      delta.dupBasePtr(op.base);
      delta.basecount = returnValue.deltaBasecount = 1;
    }
    for (opItr = ((ContextOp&)op).begin(); opItr != ((ContextOp&)op).end();
         opItr++) {
      if (opItr->second->basecount > 0) {
        recordDeltaHelper(
          NULL, *((ContextOp*)(opItr->second)), delta[opItr->first]
        );
        delta.basecount += opItr->second->basecount;
        returnValue.deltaBasecount += opItr->second->basecount;
      }
    }
    return returnValue;
  }
  if (((ContextOp&)op).base != NULL) {
    delta.dupBasePtr(((ContextOp&)op).base);
    delta.basecount++;
    returnValue.deltaBasecount++;
  } else if (((ContextOp&)op).clearBaseFlag&&(c->base != NULL)) {
    delta.clearBaseFlag = true;
    delta.blankcount++;
    returnValue.deltaBlankcount++;
  }
  if (((ContextOp&)op).clearDimsFlag) {
    int unionCBasecount = 0;
    list<ContextMap::iterator> emptySubDeltaDimensions;

    // Op prunes, so we need to determine if it will do any actual pruning:
    for (opItr = ((ContextOp&)op).begin(); opItr != ((ContextOp&)op).end();
         opItr++) {
      cItr = ((Context*)c)->find(opItr->first);
      rc.deltaBasecount = rc.deltaBlankcount = 0;
      if (cItr == ((Context*)c)->end()) {
        if (opItr->second->basecount > 0) {
          rc = recordDeltaHelper(
            NULL, *((ContextOp*)(opItr->second)), delta[opItr->first]
          );
        }
      } else {
        unionCBasecount += cItr->second->basecount;
        if (cItr->second->isEmpty()) {
          if (opItr->second->basecount > 0) {
            rc = recordDeltaHelper(
              NULL, *((ContextOp*)(opItr->second)), delta[opItr->first]
            );
          }
        } else {
          pair<ContextMap::iterator, bool> insertRC =
            delta.insert(ContextMap::value_type(
              opItr->first, ContextMap::mapped_type()
            ));

          if (insertRC.second) {
            insertRC.first->second =
              new ContextOp(&delta, &(insertRC.first->first));
          } else {
            // This dimension should not exist in delta already:
            ostringstream oss;

            oss << "Dimension \"" << opItr->first.canonical() << "\" already "
                << "existed in recording delta";
            throw IntenseException(oss.str());
          }

          rc = recordDeltaHelper(
            cItr->second, *((ContextOp*)(opItr->second)), delta[opItr->first]
          );
          if ((rc.deltaBasecount == 0)&&(rc.deltaBlankcount == 0)) {
            emptySubDeltaDimensions.push_front(insertRC.first);
          }
        }
      }
      returnValue.deltaBasecount += rc.deltaBasecount;
      returnValue.deltaBlankcount += rc.deltaBlankcount;
    }
    if (c->basecount > unionCBasecount) {
      delta.clearDimsFlag = true;
      delta.blankcount++;
      returnValue.deltaBlankcount++;
    } else {
      // We didn't do any pruning, so any empty delta sub-ops are
      // irrelevant:
      list<ContextMap::iterator>::iterator emptySubDeltaDimensionsItr;

      for (emptySubDeltaDimensionsItr = emptySubDeltaDimensions.begin();
           emptySubDeltaDimensionsItr != emptySubDeltaDimensions.end();
           emptySubDeltaDimensionsItr++) {
        delete (*emptySubDeltaDimensionsItr)->second;
        delta.erase(*emptySubDeltaDimensionsItr);
      }
    }
  } else {
    for (opItr = ((ContextOp&)op).begin(); opItr != ((ContextOp&)op).end();
         opItr++) {
      cItr = ((Context*)c)->find(opItr->first);
      rc.deltaBasecount = rc.deltaBlankcount = 0;
      if (cItr == ((Context*)c)->end()) {
        if (opItr->second->basecount > 0) {
          rc = recordDeltaHelper(
            NULL, *((ContextOp*)(opItr->second)), delta[opItr->first]
          );
        }
      } else {
        if (cItr->second->isEmpty()) {
          if (opItr->second->basecount > 0) {
            rc = recordDeltaHelper(
              NULL, *((ContextOp*)(opItr->second)), delta[opItr->first]
            );
          }
        } else if (!(opItr->second->isEmpty())) {
          pair<ContextMap::iterator, bool> insertRC =
            delta.insert(ContextMap::value_type(
              opItr->first, ContextMap::mapped_type()
            ));

          if (insertRC.second) {
            insertRC.first->second =
              new ContextOp(&delta, &(insertRC.first->first));
          } else {
            ostringstream oss;

            oss << "Dimension \"" << opItr->first.canonical() << "\" already "
                << "existed in recording delta";
            throw IntenseException(oss.str());
          }

          rc = recordDeltaHelper(
            cItr->second, *((ContextOp*)(opItr->second)), delta[opItr->first]
          );
          if ((rc.deltaBasecount == 0)&&(rc.deltaBlankcount == 0)) {
            delete insertRC.first->second;
            delta.erase(insertRC.first);
          }
        }
      }
      returnValue.deltaBasecount += rc.deltaBasecount;
      returnValue.deltaBlankcount += rc.deltaBlankcount;
    }
  }
  return returnValue;
}


// Utility, for use by applyHelper, to empty a dimension behind a given
// iterator.  In a subclass such as AEther, this can be overridden, to avoid
// wiping out certain dimensions (ie those with a headcount, for AEthers).
void Context::checkedClear
(ContextMap::iterator& itr, Origin* origin)
{
  delete itr->second;
  erase(itr);
}


// Recursive ContextOp operation helper:
void Context::applyHelper
(const ContextOp& op, Origin* origin)
{
  ContextMap::iterator opItr, itr, tempItr;
  pair<ContextMap::iterator, bool> insertRC;
  int initialChildBasecount;

  if (op.getClearBaseFlag()) {
    if (base != NULL) {
      deleteBasePtr();
      basecount--;
    }
  } else if (op.base != NULL) {
    if (base != NULL) {
      deleteBasePtr();
    } else {
      basecount++;
    }
    dupBasePtr(op.base);
  }
  applyNotify(op, origin);
  if (op.getClearDimsFlag()) {
    itr = begin();
    for (opItr = ((ContextOp&)op).begin(); opItr != ((ContextOp&)op).end();
         opItr++) {
      while ((itr != ((Context*)this)->end())&&(itr->first != opItr->first)) {
        // This is a dimension that is in *this, but not in c, so we
        // have to delete it:
        basecount -= itr->second->basecount;
        tempItr = itr;
        tempItr++;
        checkedClear(itr, origin);
        itr = tempItr;
      }
      if (itr != end()) {
        // This dimension already exists in *this.
        initialChildBasecount = itr->second->basecount;
        itr->second->applyHelper(*((ContextOp*)(opItr->second)), origin);
        basecount += itr->second->basecount - initialChildBasecount;
      } else {
        // This dimension did not already exist in *this.
        insertRC = this->insert(value_type(opItr->first, mapped_type()));
        itr = insertRC.first;
        itr->second = valueRefFactory(itr->first);
        initialChildBasecount = itr->second->basecount;
        itr->second->applyHelper(*((ContextOp*)(opItr->second)), origin);
        basecount += itr->second->basecount - initialChildBasecount;
      }
      // Now, if that vmod resulted in an empty subcontext, we just prune it
      // here and now:
      if (itr->second->isEmpty()) {
        tempItr = itr;
        tempItr++;
        checkedClear(itr, origin);
        itr = tempItr;
      } else {
        itr++;
      }
    }
    // Now we have to empty any remaining dimensions in *this:
    while (itr != ((Context*)this)->end()) {
      basecount -= itr->second->baseCount();
      tempItr = itr;
      tempItr++;
      checkedClear(itr, origin);
      itr = tempItr;
    }
  } else { // op.clear_dims_flag is false
    for (opItr = ((ContextOp&)op).begin(); opItr != ((ContextOp&)op).end();
         opItr++) {
      itr = this->find(opItr->first);
      if (itr != end()) {
        initialChildBasecount = itr->second->basecount;
        itr->second->applyHelper(*((ContextOp*)(opItr->second)), origin);
        basecount += itr->second->basecount - initialChildBasecount;
      } else {
        // This dimension did not already exist in *this.
        insertRC = this->insert(value_type(opItr->first, mapped_type()));
        itr = insertRC.first;
        itr->second = valueRefFactory(itr->first);
        initialChildBasecount = itr->second->basecount;
        itr->second->applyHelper(*((ContextOp*)(opItr->second)), origin);
        basecount += itr->second->basecount - initialChildBasecount;
      }
      // As above, if that vmod resulted in an empty subcontext, we
      // just prune it here and now:
      if (itr->second->isEmpty()) {
        checkedClear(itr, origin);
      }
    }
  }
  clearCanonicalString();
}


// This adds a delta to the basecounts of all ancestors of this Context:
void Context::propagateBasecount
(int basecountDelta)
{
  Context* contextMarker;

  if (basecountDelta == 0) {
    return;
  }
  for (contextMarker = (Context*)parent; contextMarker != NULL;
       contextMarker = (Context*)contextMarker->parent) {
    contextMarker->basecount += basecountDelta;
    contextMarker->clearCanonicalString();
  }
}


Context& Context::value
(const CompoundDimension& dim)
{
  list<DimensionRef>::iterator itr = ((CompoundDimension&)dim).begin();
  Context* marker = this;

  while (itr != ((CompoundDimension&)dim).end()) {
    marker = &marker->valueRef(*itr++);
  }
  return *marker;
}


Context& Context::value
(const CompoundDimension* dim)
{
  if (dim == NULL) {
    return *this;
  } else {
    return value(*dim);
  }
}


Context& Context::valueRef
(const DimensionRef& dim)
{
  pair<ContextMap::iterator, bool> insertRC =
    insert(ContextMap::value_type(dim, ContextMap::mapped_type()));

  if (insertRC.second) {
    // The argument is solely for the benefit of subclasses like
    // ContextOp and AEther, which override valueRefFactory:
    insertRC.first->second = valueRefFactory(insertRC.first->first);
  }
  return *(insertRC.first->second);
}


Context* Context::valueRefFactory
(const DimensionRef& dim)
{
  return new Context(this, &dim);
}


void Context::findPathTo
(const Context& descendant, CompoundDimension& result)
{
  for (Context* marker = &((Context&)descendant); marker != this;
       marker = (Context*)(marker->parent)) {
    if (marker->parent == NULL) {
      throw IntenseException(
        "Attempt to find path of non-descendant node in Context::findPathTo"
      );
    }
    result.prepend(*(marker->parentDimension));
  }
}


// Returns -1 if this node is a descendant of the argument node, 0 if this node
// is the same node as the argument node, and 1 if this node is an ancestor of
// the argument node.  If path is non-NULL, it is cleared and populated with
// the path from the ancestor to the descendant.
int Context::getAncestry
(const Context& node, CompoundDimension* path)
{
  if (path != NULL) {
    path->clear();
  }
  if (this == &node) {
    return 0;
  } else if (((Context&)node).isAncestor(*this)) {
    if (path != NULL) {
      ((Context&)node).findPathTo(*this, *path);
    }
    return -1;
  } else if (!(this->isAncestor((Context&)node))) {
    ostringstream oss;

    oss << "node " << node << " is not an ancestor of node " << *this
        << ", or vice versa, and the two nodes are not the same";
    throw IntenseException(oss.str());
  }
  if (path != NULL) {
    findPathTo(node, *path);
  }
  return 1;
}


// The canonical form string:
// This used to be canonical_helper(), for collapsed canonical form:
string Context::canonical
(bool retain, bool retainInChildren)
{
  if (canonicalString != NULL) {
    return *canonicalString;
  } else {
    ContextMap::iterator itr = begin(), endItr = end();
    string result = "<";

    if (!(isEmpty())) {
      if (base != NULL) {
        result += base->canonical();
      } else if (itr != endItr) {
        // This section is just to put in at least one term, before using a
        // '+':
        while ((itr != endItr)&&(itr->second->isEmpty())) {
          itr++;
        }
        if (itr != endItr) {
          result += itr->first.canonical() + ":" +
            itr->second->canonical(retainInChildren, retainInChildren);
          itr++;
        }
      }
      while (itr != endItr) {
        if (!(itr->second->isEmpty())) {
          result += '+' + itr->first.canonical() + ":" +
            itr->second->canonical(retainInChildren, retainInChildren);
        }
        itr++;
      }
    }
    result += '>';
    if (retain) {
      if (canonicalString != NULL) {
        delete canonicalString;
      }
      canonicalString = new string(result);
    }
    return result;
  }
}


void Context::setBase
(const BaseValue& bv)
{
  Context* contextMarker;

  if (base == NULL) {
    for (contextMarker = this; contextMarker != NULL;
         contextMarker = (Context*)(contextMarker->parent)) {
      contextMarker->basecount++;
      contextMarker->clearCanonicalString();
    }
  } else {
    for (contextMarker = this; contextMarker != NULL;
         contextMarker = (Context*)(contextMarker->parent)) {
      contextMarker->clearCanonicalString();
    }
    deleteBasePtr();
  }
  setBasePtr(&((BaseValue&)bv));
}


void Context::setBase
(const BaseValue* bv)
{
  if (bv == NULL) {
    throw IntenseException("Attempt to set NULL base value in Context");
  }
  setBase(*bv);
}


void Context::setBase
(const string& s)
{
  setBase(new StringBaseValue(s));
}


void Context::setBase
(const char* s)
{
  setBase(new StringBaseValue(s));
}


BaseValue* Context::getChildBase
(const Dimension& nonCompoundDimension) const
{
  ContextMap::iterator contextItr =
    ((Context*)this)->find(nonCompoundDimension);

  if (contextItr == ((Context*)this)->end()) {
    return NULL;
  } else {
    return contextItr->second->getBase();
  }
}


void Context::clearBase
()
{
  if (base != NULL) {
    Context* contextMarker;

    for (contextMarker = this; contextMarker != NULL;
         contextMarker = (Context*)(contextMarker->parent)) {
      contextMarker->basecount--;
      contextMarker->clearCanonicalString();
    }
    deleteBasePtr();
  }
}


void Context::clearCanonicalString
()
{
  if (canonicalString != NULL) {
    delete canonicalString;
    canonicalString = NULL;
  }
}


Context& Context::root
()
{
  Context* marker = this;

  while (marker->parent != NULL) {
    marker = (Context*)(marker->parent);
  }
  return *marker;
}


bool Context::isAncestor
(Context& context)
{
  Context* marker = (Context*)(context.parent);

  while (marker != NULL) {
    if (marker == this) {
      return true;
    }
    marker = (Context*)(marker->parent);
  }
  return false;
}


Context::Iterator& Context::Iterator::operator++
()
{
  ContextMap::iterator::operator++();
  while ((*this != node->end())&&
         ((((ContextMap::iterator)*this))->second->isEmpty())) {
    ContextMap::iterator::operator++();
  }
}


Context::Iterator& Context::Iterator::operator++
(int)
{
  return ++(*this);
}


Context::Iterator Context::beginPopulated
()
{
  Iterator itr = Iterator(*this, ContextMap::begin());

  if ((itr != ContextMap::end()&&(itr->second->isEmpty()))) {
    // Advance to the next non-empty element:
    ++itr;
  }
  return itr;
}


const char* Context::Token::typeStrings[] = {
  "VOID", "DIMENSION", "BASEVALUE", "DASH", "DASHDASH", "DASHDASHDASH", "PLUS",
  "LANGLE", "RANGLE", "LSQUARE", "RSQUARE", "ERROR", "ENDOFFILE"
};


Context::Token::~Token
()
{
  return;
  switch (type) {
  case DIMENSION:
    delete value.dimension;
    break;
  case BASEVALUE:
    delete value.baseValue;
    break;
  case ERROR:
    delete value.errorText;
  case VOID:
    return;
  default:
    return;
  }
}


void Context::parseStream
(istream& is)
{
  ContextOpLexer lexer(is);
  int initialBasecount = basecount;

  clearNoPropagate();
  recogniseNode(lexer);
  if (basecount != initialBasecount) {
    propagateBasecount(basecount - initialBasecount);
  }
}


void Context::parse
(const char* input)
{
  istringstream inputStream(input);

  parse(inputStream);
}


// A simple in-place recursive descent parser for Context nodes, to replace
// inefficient bison/JavaCUP implementations in C++ and Java, respectively.
// The minimal state machine looks like this, where "term" refers to a base
// value or a dimension:<subcontext>.
//
//                term
//       START/1--------->END/0<---+
//         |               ^       | '>'
//         | '<'           | '>'   |
//         |               |       |  '+'
//         +-------------->2------>3------>4
//                           term  ^       |
//                                 |       |
//                                 +-------+
//                                   term
//
void Context::recogniseNode
(IntenseLexer& lexer, Origin* origin)
{
  Token token;
  int state = 1;
  Context* tempParent = (Context*)parent;
  parent = NULL;

  while (state != 0) {
    switch (lexer.getToken(token)) {
    case Token::BASEVALUE:
      switch (state) {
      case 1:
        // MATCH:
        if (base == NULL) {
          basecount++;
        } else {
          deleteBasePtr();
        }
        setBasePtr(token.value.baseValue);
        state = 0;
        break;
      case 2:
      case 4:
        if (base == NULL) {
          basecount++;
        } else {
          deleteBasePtr();
        }
        setBasePtr(token.value.baseValue);
        state = 3;
        break;
      case 3:
        {
          ostringstream oss;

          oss << "Base value found after preceding term in " << className()
              << " parse";
          throw IntenseException(oss.str());
        }
      }
      break;
    case Token::DIMENSION:
      switch (state) {
      case 1:
      case 2:
      case 4:
        {
          pair<ContextMap::iterator, bool> insertRC;
          Context* child;
          int initialChildBasecount;

          insertRC = insert(
            ContextMap::value_type(
              *(token.value.dimension), ContextMap::mapped_type()
            )
          );
          if (insertRC.second) {
            insertRC.first->second = valueRefFactory(insertRC.first->first);
          }
          child = insertRC.first->second;
          initialChildBasecount = child->basecount;
          child->recogniseNode(lexer);
          basecount += child->basecount - initialChildBasecount;
        }
        if (state == 1) {
          state = 0;
        } else {
          state = 3;
        }
        break;
      case 3:
        {
          ostringstream oss;

          oss << "Dimension \"" << token.value.dimension->canonical()
              << "\" found after term (with no preceding '+') in "
              << className() << " parse";
          throw IntenseException(oss.str());
        }
      }
      break;
    case Token::LANGLE:
      switch (state) {
      case 1:
        state = 2;
        break;
      case 2:
        {
          ostringstream oss;

          oss << "'<' found after initial '<' in " << className() << " parse";
          throw IntenseException(oss.str());
        }
      case 3:
        {
          ostringstream oss;

          oss << "'<' found after term in " << className() << " parse";
          throw IntenseException(oss.str());
        }
      case 4:
        {
          ostringstream oss;

          oss << "'<' found after '+' in " << className() << " parse";
          throw IntenseException(oss.str());
        }
      }
      break;
    case Token::RANGLE:
      switch (state) {
      case 1:
        {
          ostringstream oss;

          oss << "'>' found at start of context expression in " << className()
              << " parse";
          throw IntenseException(oss.str());
        }
      case 2:
      case 3:
        // Match:
        state = 0;
        break;
      case 4:
        {
          ostringstream oss;

          oss << "'>' found after '+' in " << className() << " parse";
          throw IntenseException(oss.str());
        }
      }
      break;
    case Token::PLUS:
      switch (state) {
      case 1:
        {
          ostringstream oss;

          oss << "'+' found at the start of context expression in "
              << className() << " parse";
          throw IntenseException(oss.str());
        }
      case 2:
        {
          ostringstream oss;

          oss << "'+' found after initial '<' in " << className() << " parse";
          throw IntenseException(oss.str());
        }
      case 3:
        state = 4;
        break;
      case 4:
        {
          ostringstream oss;

          oss << "'+' found after preceiding '+' in " << className()
              << " parse";
          throw IntenseException(oss.str());
        }
      }
      break;
    case Token::LSQUARE:
      {
        ostringstream oss;

        oss << "ContextOp '[' encountered in " << className() << " parse";
        throw IntenseException(oss.str());
      }
    case Token::RSQUARE:
      {
        ostringstream oss;

        oss << "ContextOp ']' encountered in " << className() << " parse";
        throw IntenseException(oss.str());
      }
    case Token::DASH:
      {
        ostringstream oss;

        oss << "ContextOp '-' encountered in " << className() << " parse";
        throw IntenseException(oss.str());
      }
    case Token::DASHDASH:
      {
        ostringstream oss;

        oss << "ContextOp '--' encountered in " << className() << " parse";
        throw IntenseException(oss.str());
      }
    case Token::DASHDASHDASH:
      {
        ostringstream oss;

        oss << "ContextOp '---' encountered in " << className() << " parse";
        throw IntenseException(oss.str());
      }
    case Token::ERROR:
      {
        ostringstream oss;

        oss << "Lexical error \"" << *(token.value.errorText) << "\" in "
            << className() << " parse";
        throw IntenseException(oss.str());
      }
    case Token::ENDOFFILE:
      {
        ostringstream oss;

        oss << "End of input reached in " << className() << " parse";
        throw IntenseException(oss.str());
      }
    default:
      {
        ostringstream oss;

        oss << "INTERNAL ERROR: Unknown token type " << token.getType()
            << " found in " << className() << " parse";
        throw IntenseException(oss.str());
      }
    }
  }
  // For the benefit of AEthers - does nothing in Context:
  assignNotify(*this, origin);
  // This is so the base value isn't deleted by the token destructor (we don't
  // have to worry about this, in the case where there is an opening <, since
  // the closing > will stop any deletion under token.value, so this is really
  // just for dim:barebase.  However, we'll leave it at the end:
  token.setVoid();
  parent = tempParent;
}


void Context::serialise
(BaseSerialiser& baseSerialiser, ostream& os,
 BoundManager* boundManager) const
{
  ContextMap::iterator itr;
  int tempInt;

  baseSerialiser.out(basecount, os);
  BaseValue::factorySerialise(base, baseSerialiser, os, boundManager);
  tempInt = ((Context*)this)->size();
  baseSerialiser.out(tempInt, os);
  for (itr = ((Context*)this)->begin(); itr != ((Context*)this)->end();
       itr++) {
    itr->first.serialise(baseSerialiser, os);
    itr->second->serialise(baseSerialiser, os, boundManager);
  }
  os.flush();
}


void Context::deserialise
(BaseSerialiser& baseSerialiser, istream& is, BoundManager* boundManager)
{
  int initialBasecount = basecount;

  clear();
  deserialiseHelper(baseSerialiser, is, boundManager, NULL);
  propagateBasecount(basecount - initialBasecount);
}


void Context::deserialiseHelper
(BaseSerialiser& baseSerialiser, istream& is, BoundManager* boundManager,
 Origin* origin)
{
  int tempInt;

  baseSerialiser.in(tempInt, is);
  if (tempInt < 0) {
    ostringstream oss;

    oss << "Negative base count " << tempInt
        << " in Context::deserialiseHelper()";
    throw IntenseException(oss.str());
  }
  basecount = tempInt;
  setBasePtr(BaseValue::factoryDeserialise(baseSerialiser, is, boundManager));
  // The number of dimensions:
  baseSerialiser.in(tempInt, is);
  if (tempInt < 0) {
    ostringstream oss;

    oss << "Negative dimension count " << tempInt
        << " in Context::deserialise()";
    throw IntenseException(oss.str());
  }
  // Next, read in the dimension names and (recursively) their values:
  for (int i = 0; i < tempInt; i++) {
    DimensionRef dimensionRef(baseSerialiser, is);

    valueRef(dimensionRef).deserialiseHelper(
      baseSerialiser, is, boundManager, origin
    );
  }
  assignNotify(*this, origin);
}
