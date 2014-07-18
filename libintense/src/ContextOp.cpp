// ****************************************************************************
//
// ContextOp.cpp : Context operators.
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


#include <list>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <intense-io.hpp>
#include "intense_config.hpp"
#include "BoundManager.hpp"
#include "IntenseException.hpp"
#include "Context.hpp"
#include "StringBaseValue.hpp"
#include "BinaryBaseValue.hpp"
#include "correct_FlexLexer.h"
#include "ContextOpLexer.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;


ContextOp::ContextOp
(const string& canonicalForm)
  : Context(), clearBaseFlag(0), clearDimsFlag(0),
    blankcount(0)
{
  parse(canonicalForm);
}


ContextOp::ContextOp
(const char* canonicalForm)
  : Context(), clearBaseFlag(0), clearDimsFlag(0),
    blankcount(0)
{
  parse(canonicalForm);
}


ContextOp& ContextOp::assign
(const ContextOp& o)
{
  int deltaBasecount, deltaBlankcount;
  ContextOp* contextMarker;

  deltaBasecount = o.basecount - basecount;
  deltaBlankcount = o.blankcount - blankcount;
  clearNoPropagate();
  for (contextMarker = (ContextOp*)parent; contextMarker;
       contextMarker = (ContextOp*)(contextMarker->parent)) {
    contextMarker->basecount += deltaBasecount;
    contextMarker->blankcount += deltaBlankcount;
    contextMarker->clearCanonicalString();
  }
  assignHelper(o);
  return *this;
}


void ContextOp::assignHelper
(const ContextOp& o)
{
  ContextMap::iterator oItr;

  basecount = o.basecount;
  blankcount = o.blankcount;
  clearBaseFlag = o.clearBaseFlag;
  clearDimsFlag = o.clearDimsFlag;
  dupBasePtr(o.base);
  for (oItr = ((ContextOp&)o).begin(); oItr != ((ContextOp&)o).end(); oItr++) {
    if (((oItr->second->basecount > 0)||
         (((ContextOp*)(oItr->second))->blankcount > 0))||
        // Take care of <...+dim:[(empty)]+...>:
        (clearDimsFlag)) {
      pair<ContextMap::iterator, bool> insertRC;

      insertRC = insert(
        ContextMap::value_type(oItr->first, ContextMap::mapped_type())
      );
      if (insertRC.second) {
        ((ContextOp*)
         (insertRC.first->second =
          new ContextOp(this, &(insertRC.first->first))))->
          assignHelper(*((ContextOp*)(oItr->second)));
      } else {
        ((ContextOp*)(insertRC.first->second))->
          assignHelper(*((ContextOp*)(oItr->second)));
      }
    }
  }
}


// ContextOp equality test:
bool ContextOp::equals
(const ContextOp& op) const
{
  ContextMap::iterator itr, opItr;

  // ContextOps must test for equality of flags, as well as equality of
  // BaseValues and subcontexts.
  //
  // First, return (something) unless both contexts are non-vanilla:
  if (isEmpty()) {
    if (op.isEmpty()) {
      return 1;
    } else {
      return 0;
    }
  } else {
    if (op.isEmpty()) {
      return 0;
    }
  }
  // *** This might not be good, but if it is, it saves time:
  if (basecount != op.basecount) {
    return 0;
  }
  // If we have equal flags and, either neither of us have BaseValues,
  // or we have equal BaseValues:
  if (((clearBaseFlag == op.clearBaseFlag)&&
       (clearDimsFlag == op.clearDimsFlag))&&
      (((base == NULL)&&(op.base == NULL))||
       ((base != NULL)&&(op.base != NULL)&&(*base == *(op.base))))) {
    itr = ((ContextOp*)this)->begin();
    opItr = ((ContextOp&)op).begin();
    while (itr != ((ContextOp*)this)->end()) {
      while ((opItr != ((ContextOp&)op).end())&&(opItr->first != itr->first)) {
        // Hmmm.  We're not in synch, so either this branch in o is vanilla, or
        // this is a shooting offense:
        if (!opItr->second->isEmpty()) {
          return 0;
        }
        opItr++;
      }
      if (itr->second->isEmpty()) {
        if (op.count(itr->first)) {
          if (!((ContextOp&)op).mapRef()[itr->first]->isEmpty()) {
            return 0;
          }
        }
      } else if ((!(((ContextOp&)op).count(itr->first)))||
                 (!(itr->second->equals(*(((ContextOp&)op).
                                          mapRef()[itr->first]))))) {
        return 0;
      }
      itr++;
      if (opItr != ((ContextOp&)op).end()) {
        opItr++;
      }
    }
    // Now we have to see if any remaining dimensions in o are vanilla:
    while (opItr != ((ContextOp&)op).end()) {
      if (!(opItr++)->second->isEmpty()) {
        return 0;
      }
    }
    return 1;
  }
  // One of us has a BaseValue and the other doesn't, or, we have unequal
  // BaseValues:
  return 0;
}


// Applying a Context to a ContextOp as an operator turns the ContextOp into a
// pure vset to the argument Context (note the covariant return type):
ContextOp& ContextOp::apply
(const Context& c)
{
  int deltaBasecount;
  int deltaBlankcount;
  ContextOp* contextMarker;

  deltaBasecount = ((Context&)c).basecount - basecount;
  clearNoPropagate();
  deltaBlankcount = applyHelper(c);
  for (contextMarker = (ContextOp*)parent; contextMarker != NULL;
       contextMarker = (ContextOp*)(contextMarker->parent)) {
    contextMarker->basecount += deltaBasecount;
    contextMarker->blankcount += deltaBlankcount;
    contextMarker->clearCanonicalString();
  }
  return *this;
}


int ContextOp::applyHelper
(const Context& c)
{
  ContextMap::iterator cItr;
  int deltaBlankcount = 1;

  clearDimsFlag = true;
  basecount = c.basecount;
  dupBasePtr(((Context&)c).base);
  if (base == NULL) {
    clearBaseFlag = true;
    deltaBlankcount++;
  }
  for (cItr = ((Context&)c).begin(); cItr != ((Context&)c).end(); cItr++) {
    if (cItr->second->basecount > 0) {
      pair<ContextMap::iterator, bool> insertRC;

      insertRC = insert(
        ContextMap::value_type(cItr->first, ContextMap::mapped_type())
      );
      if (insertRC.second) {
        deltaBlankcount +=
          ((ContextOp*)
           (insertRC.first->second =
            new ContextOp(this, &(insertRC.first->first))))->
          applyHelper(*(cItr->second));
      } else {
        deltaBlankcount +=
          ((ContextOp*)(insertRC.first->second))->applyHelper(*(cItr->second));
      }
    }
  }
  return deltaBlankcount;
}


// Apply v as a context operation (determine whether it's a vmod or a
// vset, and go from there (return type is covariant, from Context):
ContextOp& ContextOp::apply
(const ContextOp& op)
{
  ContextOp* contextOpMarker;
  int deltaBasecount = basecount;
  int deltaBlankcount = blankcount;

  applyHelper(op);
  deltaBasecount = basecount - deltaBasecount;
  deltaBlankcount = blankcount - deltaBlankcount;
  for (contextOpMarker = (ContextOp*)parent; contextOpMarker;
       contextOpMarker = (ContextOp*)(contextOpMarker->parent)) {
    contextOpMarker->basecount += deltaBasecount;
    contextOpMarker->blankcount += deltaBlankcount;
    contextOpMarker->clearCanonicalString();
  }
  return *this;
}


// recursive operation helper:
void ContextOp::applyHelper
(const ContextOp& op)
{
  ContextMap::iterator opItr, itr, tempItr;
  pair<ContextMap::iterator, bool> insertRC;
  ContextOp* contextOpMarker;
  bool mustClearCurrentDimension;
  int initialChildBasecount;
  int initialChildBlankcount;

  if (((ContextOp&)op).clearBaseFlag) {
    if (base != NULL) {
      deleteBasePtr();
      basecount--;
      clearBaseFlag = true;
      blankcount++;
    } else if (!clearBaseFlag) {
      clearBaseFlag = true;
      blankcount++;
    }
  } else if (((ContextOp&)op).base != NULL) {
    if (base != NULL) {
      deleteBasePtr();
    } else {
      if (clearBaseFlag) {
        clearBaseFlag = false;
        blankcount--;
      }
      basecount++;
    }
    dupBasePtr(((ContextOp&)op).base);
  }
  if (((ContextOp&)op).clearDimsFlag) {
    itr = begin();
    for (opItr = ((ContextOp&)op).begin(); opItr != ((ContextOp&)op).end();
         opItr++) {
      while ((itr != end())&&(itr->first != opItr->first)) {
        // This is a dimension that is in *this, but not in op, so we
        // have to delete it:
        basecount -= itr->second->basecount;
        blankcount -= ((ContextOp*)(itr->second))->blankcount;
        tempItr = itr;
        tempItr++;
        delete itr->second;
        erase(itr);
        itr = tempItr;
      }
      mustClearCurrentDimension = false;
      if (itr != end()) {
        // This dimension already exists in *this.
        contextOpMarker = (ContextOp*)itr->second;
        initialChildBasecount = contextOpMarker->basecount;
        initialChildBlankcount = contextOpMarker->blankcount;
        contextOpMarker->applyHelper(*((ContextOp*)opItr->second));
        basecount += contextOpMarker->basecount - initialChildBasecount;
        blankcount += contextOpMarker->blankcount - initialChildBlankcount;
      } else {
        // This dimension did not already exist in *this.
        insertRC = this->insert(value_type(opItr->first, mapped_type()));
        itr = insertRC.first;
        itr->second = contextOpMarker = new ContextOp(this, &(itr->first));
        // If the clearDimsFlag is true in *this or we are already pruning from
        // above here, any new dimensions added from op have to contain actual
        // basevalues:
        if (clearDimsFlag) {
          contextOpMarker->clearBaseFlag = true;
          contextOpMarker->clearDimsFlag = true;
          contextOpMarker->blankcount = 2;
          initialChildBasecount = contextOpMarker->basecount;
          initialChildBlankcount = contextOpMarker->blankcount;
          contextOpMarker->applyHelper(*((ContextOp*)(opItr->second)));
          if (contextOpMarker->basecount == 0) {
            // *contextOpMarker contains no actual base values.  It's toast:
            mustClearCurrentDimension = true;
          } else {
            basecount += contextOpMarker->basecount - initialChildBasecount;
            blankcount += 2 +
              contextOpMarker->blankcount - initialChildBlankcount;
          }
        } else {
          initialChildBasecount = contextOpMarker->basecount;
          initialChildBlankcount = contextOpMarker->blankcount;
          contextOpMarker->applyHelper(*((ContextOp*)opItr->second));
          basecount += contextOpMarker->basecount - initialChildBasecount;
          blankcount += contextOpMarker->blankcount - initialChildBlankcount;
        }
      }
      // Prune any extraneous dim:<> sub-obs we created, if this ContextOp has
      // its clearDimsFlag set:
      if ((!mustClearCurrentDimension)&&(contextOpMarker->basecount == 0)&&
          (contextOpMarker->blankcount == 2)&&
          (contextOpMarker->clearBaseFlag)&&(contextOpMarker->clearDimsFlag)) {
        blankcount -= 2;
        mustClearCurrentDimension = true;
      }
      if (mustClearCurrentDimension) {
        delete itr->second;
        tempItr = itr;
        tempItr++;
        erase(itr);
        itr = tempItr;
      } else {
        itr++;
      }
    }
    // Now we have to vanillafy any remaining dimensions in *this:
    while (itr != end()) {
      basecount -= itr->second->basecount;
      blankcount -= ((ContextOp*)itr->second)->blankcount;
      delete itr->second;
      tempItr = itr;
      tempItr++;
      erase(itr);
      itr = tempItr;
    }
    // Now, since we applied a -- to *this, *this must get one.
    if (!clearDimsFlag) {
      blankcount++;
      clearDimsFlag = true;
    }
  } else { // op.clearDimsFlag is false
    for (opItr = ((ContextOp&)op).begin(); opItr != ((ContextOp&)op).end();
         opItr++) {
      mustClearCurrentDimension = false;
      itr = this->find(opItr->first);
      if (itr != end()) {
        contextOpMarker = (ContextOp*)itr->second;
        initialChildBasecount = contextOpMarker->basecount;
        initialChildBlankcount = contextOpMarker->blankcount;
        contextOpMarker->applyHelper(*((ContextOp*)opItr->second));
        basecount += contextOpMarker->basecount - initialChildBasecount;
        blankcount += contextOpMarker->blankcount - initialChildBlankcount;
      } else {
        insertRC = this->insert(value_type(opItr->first, mapped_type()));
        itr = insertRC.first;
        itr->second = contextOpMarker = new ContextOp(this, &(itr->first));
        // If the clearDimsFlag is true in *this, any new dimensions added from
        // op have to contain actual basevalues:
        if (clearDimsFlag) {
          contextOpMarker->clearBaseFlag = true;
          contextOpMarker->clearDimsFlag = true;
          contextOpMarker->blankcount = 2;
          initialChildBasecount = contextOpMarker->basecount;
          initialChildBlankcount = contextOpMarker->blankcount;
          contextOpMarker->applyHelper(*((ContextOp*)(opItr->second)));
          if (contextOpMarker->basecount == 0) {
            // *contextOpMarker contains no actual base values.  It's toast:
            mustClearCurrentDimension = true;
          } else {
            basecount += contextOpMarker->basecount - initialChildBasecount;
            blankcount +=
              2 + contextOpMarker->blankcount - initialChildBlankcount;
          }
        } else {
          initialChildBasecount = contextOpMarker->basecount;
          initialChildBlankcount = contextOpMarker->blankcount;
          contextOpMarker->applyHelper(*((ContextOp*)(opItr->second)));
          basecount += contextOpMarker->basecount - initialChildBasecount;
          blankcount += contextOpMarker->blankcount - initialChildBlankcount;
        }
      }
      if ((!clearDimsFlag)&&(contextOpMarker->isEmpty())) {
        // This is a vmod, and we have a vanilla vmod under a subdimension; we
        // have to delete it:
        mustClearCurrentDimension = true;
      } else if ((!mustClearCurrentDimension)&&
                 clearDimsFlag&&
                 (contextOpMarker->basecount == 0)&&
                 (contextOpMarker->blankcount == 2)&&
                 (contextOpMarker->clearBaseFlag)&&
                 (contextOpMarker->clearDimsFlag)) {
        // Prune any extraneous dim:<> sub-obs we created, if
        // this ContextOp has its clearDimsFlag set:
        mustClearCurrentDimension = true;
        blankcount -= 2;
      }
      if (mustClearCurrentDimension) {
        delete itr->second;
        erase(itr);
      }
    }
  }
  clearCanonicalString();
}


// Record total delta resulting from the application of op to this op, as a
// minimal op, into delta (i.e., applying the resulting delta to *this yields
// the same operator as applying op to *this):
void ContextOp::recordDelta
(const ContextOp& op, ContextOp& delta) const
{
  // META: To be implemented
}


// Overridden new-ContextOp/Context constructor for operator[] helper:
Context* ContextOp::valueRefFactory
(const DimensionRef& dim)
{
  ContextOp* newOp = new ContextOp(this, &dim);

  return newOp;
}


// A child factory that sets -+-- in the child if -- is set in the parent:
ContextOp* ContextOp::valueRefFactoryRetainPruning
(const DimensionRef& dim)
{
  ContextOp* newOp = new ContextOp(this, &dim);

  if (clearDimsFlag) {
    // Not the most efficient, but it works...
    newOp->setClearBaseFlag();
    newOp->setClearDimsFlag();
  }
  return newOp;
}


// A child factory that optionally retains pruning (set -+-- in a
// previously-nonexisting child if -- is set in its parent):
ContextOp& ContextOp::value
(const CompoundDimension& dim, bool retainPruning)
{
  if (retainPruning) {
    list<DimensionRef>::iterator itr = ((CompoundDimension&)dim).begin();
    ContextOp* marker = this;

    while (itr != ((CompoundDimension&)dim).end()) {
      pair<ContextMap::iterator, bool> insertRC =
        insert(ContextMap::value_type(*itr++, ContextMap::mapped_type()));

      if (insertRC.second) {
        insertRC.first->second =
          valueRefFactoryRetainPruning(insertRC.first->first);
      }
      marker = (ContextOp*)(insertRC.first->second);
    }
    return *marker;
  } else {
    return value(dim);
  }
}


// The (full) canonical form string.  Vsets are kept in [] delimeters.
string ContextOp::canonical
(bool retain, bool retainInChildren)
{
  if (canonicalString != NULL) {
    return *canonicalString;
  } else {
    ContextMap::iterator itr = begin(), endItr = end();
    string result = "[";

    if (clearBaseFlag) {
      if (clearDimsFlag) {
        result += "---";
      } else {
        result += '-';
      }
    } else if (base != NULL) {
      if (clearDimsFlag) {
        result += "--+" + base->canonical();
      } else {
        result += base->canonical();
      }
    } else if (clearDimsFlag) {
      result += "--";
    } else if (itr != endItr) {
      // This section is just to put in at least one term, before using a '+':
      result += itr->first.canonical() + ":" +
        itr->second->canonical(retainInChildren, retainInChildren);
      itr++;
    }
    while (itr != endItr) {
      result += '+' + itr->first.canonical() + ":" +
        itr->second->canonical(retainInChildren, retainInChildren);
      itr++;
    }
    result += ']';
    if (retain) {
      if (canonicalString != NULL) {
        delete canonicalString;
      }
      canonicalString = new string(result);
    }
    return result;
  }
}


// The shorthand canonical form string.  Vsets are written in <> delimeters,
// and the - (unless there is a base) and -- flag delimeters from the
// corresponding vmod are removed.
string ContextOp::shortCanonical
(bool retain, bool retainInChildren)
{
  if (canonicalString != NULL) {
    return *canonicalString;
  } else {
    ContextMap::iterator itr = begin(), endItr = end();
    bool isVset = false;
    string result;

    if (clearBaseFlag) {
      if (clearDimsFlag) {
        result = "<";
        isVset = true;
        if (itr != endItr) {
          result += itr->first.canonical() + ':' +
            itr->second->shortCanonical(retainInChildren, retainInChildren);
          itr++;
        }
      } else {
        result = "[-";
      }
    } else if (base != NULL) {
      if (clearDimsFlag) {
        result = "<" + base->canonical();
        isVset = true;
      } else {
        result = "[" + base->canonical();
      }
    } else if (clearDimsFlag) {
      result = "[--";
    } else if (itr != endItr) {
      result = "[" + itr->first.canonical() + ':' +
        itr->second->shortCanonical(retainInChildren, retainInChildren);
      itr++;
    } else {
      result = "[";
    }
    while (itr != endItr) {
      result += '+' + itr->first.canonical() + ":" +
        itr->second->shortCanonical(retainInChildren, retainInChildren);
      itr++;
    }
    if (isVset) {
      result += '>';
    } else {
      result += ']';
    }
    if (retain) {
      if (canonicalString != NULL) {
        delete canonicalString;
      }
      canonicalString = new string(result);
    }
    return result;
  }
}


void ContextOp::setClearBaseFlag
()
{
  if (base != NULL) {
    deleteBasePtr();
    basecount--;
    propagateBasecount(-1);
  }
  if (!clearBaseFlag) {
    blankcount++;
    propagateBlankcount(1);
  }
  clearBaseFlag = true;
  clearCanonicalString();
}


void ContextOp::resetClearBaseFlag
()
{
  if (clearBaseFlag) {
    blankcount--;
    propagateBlankcount(-1);
  }
  clearBaseFlag = false;
  clearCanonicalString();
}


void ContextOp::setClearDimsFlag
()
{
  if (!clearDimsFlag) {
    blankcount++;
    propagateBlankcount(1);
  }
  clearDimsFlag = true;
  clearCanonicalString();
}


void ContextOp::resetClearDimsFlag
()
{
  if (clearDimsFlag) {
    blankcount--;
    propagateBlankcount(-1);
  }
  clearDimsFlag = false;
  clearCanonicalString();
}


void ContextOp::setBase
(const BaseValue& newBase)
{
  if (clearBaseFlag) {
    resetClearBaseFlag();
  }
  Context::setBase(newBase);
}


// This adds a delta to all the blankcounts of all ancestors of this ContextOp:
void ContextOp::propagateBlankcount
(int delta)
{
  ContextOp* contextOpMarker;

  for (contextOpMarker = (ContextOp*)parent; contextOpMarker;
       contextOpMarker = (ContextOp*)(contextOpMarker->parent)) {
    contextOpMarker->blankcount += delta;
    clearCanonicalString();
  }
}


void ContextOp::parseStream
(istream& is)
{
  ContextOpLexer lexer(is);
  int initialBasecount = basecount;
  int initialBlankcount = blankcount;

  clear();
  recogniseNode(lexer);
  if (basecount != initialBasecount) {
    propagateBasecount(basecount - initialBasecount);
  }
  if (blankcount != initialBlankcount) {
    propagateBasecount(blankcount - initialBlankcount);
  }
}


// A simple in-place recursive descent parser for ContextOp nodes, to replace
// inefficient bison/JavaCUP implementations in C++ and Java, respectively.
// The minimal state machine looks like this, where "term" refers to a base
// value, a dimension:<subcontext>, '-', a '--', or a '---'.  The '('
// transition refers to a '<' or a '[' and ')' refers to a '>' or a ']', where
// angle brackets are balanced with angle brackets and square brackets are
// balanced with square brackets.
//
//                term
//       START/1--------->END/0<---+
//         |               ^       | ')'
//         | '('           | ')'   |
//         |               |       |  '+'
//         +-------------->2------>3------>4
//                           term  ^       |
//                                 |       |
//                                 +-------+
//                                   term
//
// I.e., with the same effect as (but with three less states than):
//
//                                   term
//                                 +-------+
//                                 |       |
//                           term  v       |
//         +-------------->5------>6------>7
//         |               |       |
//         | '['           | ']'   | ']'
//         |      term     v       |
//       START/1--------->END/0<---+
//         |               ^       |
//         | '<'           | '>'   | '>'
//         |               |       |  '+'
//         +-------------->2------>3------>4
//                           term  ^       |
//                                 |       |
//                                 +-------+
//                                   term
//
void ContextOp::recogniseNode
(IntenseLexer& lexer)
{
  Token token;
  int state = 1;
  bool inLAngles = false;
  Context* tempParent = (Context*)parent;
  parent = NULL;

  while (state != 0) {
    switch (lexer.getToken(token)) {
    case Token::BASEVALUE:
      switch (state) {
      case 1:
      case 2:
      case 4:
        if (clearBaseFlag) {
          clearBaseFlag = false;
          blankcount--;
        }
        if (base == NULL) {
          basecount++;
        } else {
          deleteBasePtr();
        }
        setBasePtr(token.value.baseValue);
        if (state == 1) {
          state = 0;
        } else {
          state = 3;
        }
        break;
      case 3:
        {
          ostringstream oss;

          oss << "Base value found after preceding term "
              << "(with no preceding '+') in " << className() << " parse";
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
          ContextOp* child;
          int initialChildBasecount;
          int initialChildBlankcount;

          insertRC = insert(
            ContextMap::value_type(
              *(token.value.dimension), ContextMap::mapped_type()
            )
          );
          if (insertRC.second) {
            insertRC.first->second = valueRefFactory(insertRC.first->first);
          }
          child = (ContextOp*)insertRC.first->second;
          initialChildBasecount = child->basecount;
          initialChildBlankcount = child->blankcount;
          child->recogniseNode(lexer);
          basecount += child->basecount - initialChildBasecount;
          blankcount += child->blankcount - initialChildBlankcount;
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
        inLAngles = true;
        if (base != NULL) {  
          deleteBasePtr();
          basecount--;
        }
        if (!clearBaseFlag) {
          blankcount++;
        }
        clearBaseFlag = true;
        if (!clearDimsFlag) {
          blankcount++;
        }
        clearDimsFlag = true;
        state = 2;
        break;
      case 2:
        {
          ostringstream oss;

          oss << "'<' found after initial delimiter in " << className()
              << " parse";
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
        if (inLAngles) {
          // Match:
          state = 0;
        } else {
          {
            ostringstream oss;

            oss << "'>' found in context expression which started with '[' in "
                << className() << " parse";
            throw IntenseException(oss.str());
          }
        }
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

          oss << "'+' found after initial delimiter in " << className()
              << " parse";
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
      switch (state) {
      case 1:
        state = 2;
        break;
      case 2:
        {
          ostringstream oss;

          oss << "'[' found after initial delimiter in " << className()
              << " parse";
          throw IntenseException(oss.str());
        }
      case 3:
        {
          ostringstream oss;

          oss << "'[' found after term in " << className() << " parse";
          throw IntenseException(oss.str());
        }
      case 4:
        {
          ostringstream oss;

          oss << "'[' found after '+' in " << className() << " parse";
          throw IntenseException(oss.str());
        }
      }
      break;
    case Token::RSQUARE:
      switch (state) {
      case 1:
        {
          ostringstream oss;

          oss << "']' found at start of context expression in " << className()
              << " parse";
          throw IntenseException(oss.str());
        }
      case 2:
      case 3:
        if (!inLAngles) {
          // Match:
          state = 0;
        } else {
          {
            ostringstream oss;

            oss << "']' found in context expression which started with '<' in "
                << className() << " parse";
            throw IntenseException(oss.str());
          }
        }
        break;
      case 4:
        {
          ostringstream oss;

          oss << "']' found after '+' in " << className() << " parse";
          throw IntenseException(oss.str());
        }
      }
      break;
    case Token::DASH:
      switch (state) {
      case 1:
      case 2:
      case 4:
        if (base != NULL) {  
          deleteBasePtr();
          basecount--;
        }
        if (!clearBaseFlag) {
          blankcount++;
        }
        clearBaseFlag = true;
        if (state == 1) {
          state = 0;
        } else {
          state = 3;
        }
        break;
      case 3:
        {
          ostringstream oss;

          oss << "'-' found after preceding term (with no preceding '+') in "
              << className() << " parse";
          throw IntenseException(oss.str());
        }
      }
      break;
    case Token::DASHDASH:
      switch (state) {
      case 1:
      case 2:
      case 4:
        if (!clearDimsFlag) {
          blankcount++;
        }
        clearDimsFlag = true;
        if (state == 1) {
          state = 0;
        } else {
          state = 3;
        }
        break;
      case 3:
        {
          ostringstream oss;

          oss << "'--' found after preceding term (with no preceding '+') in "
              << className() << " parse";
          throw IntenseException(oss.str());
        }
      }
      break;
    case Token::DASHDASHDASH:
      switch (state) {
      case 1:
      case 2:
      case 4:
        if (base != NULL) {  
          deleteBasePtr();
          basecount--;
        }
        if (!clearBaseFlag) {
          blankcount++;
        }
        clearBaseFlag = true;
        if (!clearDimsFlag) {
          blankcount++;
        }
        clearDimsFlag = true;
        if (state == 1) {
          state = 0;
        } else {
          state = 3;
        }
        state = 3;
        break;
      case 3:
        {
          ostringstream oss;

          oss << "'---' found after preceding term (with no preceding '+') in "
              << className() << " parse";
          throw IntenseException(oss.str());
        }
      }
      break;
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
  // This is so the base value isn't deleted by the token destructor (we don't
  // have to worry about this, in the case where there is an opening [ or <,
  // since the closing ] or > will stop any deletion under token.value, so this
  // is really just for dim:barebase.  However, we'll leave it at the end:
  token.setVoid();
  parent = tempParent;
}


void ContextOp::serialise
(BaseSerialiser& baseSerialiser, ostream& os,
 BoundManager* boundSerialiser) const
{
  int flags = 0;

  if (clearBaseFlag) {
    flags |= 0x02;
  }
  if (clearDimsFlag) {
    flags |= 0x01;
  }
  baseSerialiser.out(flags, os);
  baseSerialiser.out(blankcount, os);
  Context::serialise(baseSerialiser, os, boundSerialiser);
}


void ContextOp::deserialise
(BaseSerialiser& baseSerialiser, istream& is, BoundManager* boundSerialiser)
{
  int initialBasecount = basecount;
  int initialBlankcount = blankcount;
  int tempInt;

  clear();
  deserialiseHelper(baseSerialiser, is, boundSerialiser, NULL);
  propagateBasecount(basecount - initialBasecount);
  if (initialBlankcount != blankcount) {
    propagateBlankcount(blankcount - initialBlankcount);
  }
}


void ContextOp::deserialiseHelper
(BaseSerialiser& baseSerialiser, istream& is, BoundManager* boundSerialiser,
 Origin* THIS_PARAM_IGNORED)
{
  int tempInt;

  baseSerialiser.in(tempInt, is);
  clearBaseFlag = ((tempInt&0x02) != 0x00);
  clearDimsFlag = ((tempInt&0x01) != 0x00);
  baseSerialiser.in(tempInt, is);
  if (tempInt < 0) {
    ostringstream oss;

    oss << "Negative blankcount " << tempInt << " in ContextOp.deserialise()";
    throw IntenseException(oss.str());
  }
  blankcount = tempInt;
  Context::deserialiseHelper(baseSerialiser, is, boundSerialiser, NULL);
}
