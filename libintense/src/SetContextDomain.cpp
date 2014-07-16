// ****************************************************************************
//
// SetContextDomain.cpp : ContextDomain implemented as an STL set of Contexts.
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
#include <set>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <intense-io.hpp>
#include "BoundManager.hpp"
#include "IntenseException.hpp"
#include "Context.hpp"
#include "ContextDomain.hpp"
#include "SetContextDomain.hpp"


using namespace std;
using namespace intense;


/**
 * The destructor for SetContextDomain deletes all Contexts (probably
 * Versions, along with their BaseValues), by pointer.
 */
SetContextDomain::~SetContextDomain
()
{
  ContextSet::iterator itr = begin();

  while (itr != end()) {
    delete *itr++;
  }
}


// Returns NULL on failure; failure means a non-unique or non-existent best
// fit:
Context* SetContextDomain::best
(const Context& requested, bool canThrow) const
{
  SetDomainElement firstDomainElement(*this);

  return ContextDomain::best(&firstDomainElement, requested, canThrow);
}


// Returns a list of pointers to near-fit Contexts in the domain.  If the list
// has only one element, that is a pointer to the best-fit Context.  No Context
// in the returned set refines to any other Context in the return set:
list<Context*> SetContextDomain::near
(const Context& requested) const
{
  SetDomainElement firstDomainElement(*this);

  return ContextDomain::near(&firstDomainElement, requested);
}


bool SetContextDomain::insert
(const Context* context, bool canReplace)
{
  ContextSet::iterator findRc;

  if ((findRc = ContextSet::find((Context*)context)) != ContextSet::end()) {
    if (!canReplace) {
      return false;
    } else {
      // Delete the context - and possibly a bound item, if it happens to be a
      // Version...
      delete *findRc;
      ContextSet::erase(findRc);
    }
  }
  ContextSet::insert((Context*)context);
  return true;
}
