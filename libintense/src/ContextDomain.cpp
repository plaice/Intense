// ****************************************************************************
//
// ContextDomain.cpp : Sets of Contexts comprising the basis for an intensional
// domain (not really the domain itself), primarily for use with best fits.
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
#include <vector>
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


using namespace std;
using namespace intense;


inline const Context* bestReturn
(const Context& requested, bool canThrow, const Context* bestFit)
{
  if ((canThrow)&&(bestFit == NULL)) {
    ostringstream oss;
    oss << "Failed best fit for requested context " << requested;
    throw BestFitException(oss.str());
  }
  return bestFit;
}


// This code was taken (and modified) from the old libintensional ContextSpace
// class in ISE 1.  Returns either a best-fit context, or populates a vector of
// pointers to near-fit Contexts in this domain.  In the latter case, the
// returned Contexts are "maximally refined" with respect to the requested
// context (i.e., none of them refine to one another).  If there is exactly
// one, this is the best fit.
Context* ContextDomain::fit
(const ContextDomain::DomainElement* firstDomainElement,
 const Context& requested, list<Context*>* nearFits, bool canThrow)
{
  DomainElement* element = (DomainElement*)firstDomainElement;
  // The best-fit context:
  Context* bestFitContext = NULL;
  vector<Context*> secondBestFits;
  vector<Context*>::iterator secondBestFitsItr;
  // A flag for determining if it is OK to return a value:
  bool haveBestFit = true;

  // If there are no contexts in our space upon which to do a best-fit,
  // then just return a failed fit (NULL):
  if (element == NULL) {
    if (nearFits != NULL) {
      return NULL;
    } else {
      return (Context*)bestReturn(requested, canThrow, NULL);
    }
  }
  // Now chew through all of the contexts, looking for a best fit:
  while (element != NULL) {
    Context* thisContext;

    if ((thisContext = element->getContext()) == NULL) {
      throw IntenseException("NULL Context in ContextDomain::fit()");
    }
    // Test to see if this context refines to the requested context:
    if (*thisContext <= requested) {
      // If there is already a best-fit context, then test to see if it
      // refines to this context; otherwise, set the best-fit context to
      // this context, directly:
      if (bestFitContext != NULL) {
        // There is a best-fit context already; set the best-fit
        // context to this context:
        if (*bestFitContext <= *thisContext) {
          bestFitContext = thisContext;
          if (nearFits != NULL) {
            secondBestFits.push_back(thisContext);
          }
        }
        // Test to see if this context does not refine to the
        // best-fit; if so, we have to store this context as a
        // second-best fit:
        else if (!(*thisContext <= *bestFitContext)) {
          secondBestFits.push_back(thisContext);
        }
      } else {
        // There is no best-fit context yet; just set the best-fit
        // context to this context, directly:
        bestFitContext = thisContext;
        if (nearFits != NULL) {
          secondBestFits.push_back(thisContext);
        }
      }
    }
    element = element->getNext();
  }
  // Reset the best-fit marker to the beginning of the second-best-fits
  // array:
  secondBestFitsItr = secondBestFits.begin();
  // Now just loop through the second-best-fits array, to see if there are
  // any second-best-fits which don't refine to the best-fit:
  while (secondBestFitsItr != secondBestFits.end()) {
    if (!(**secondBestFitsItr <= *bestFitContext)) {
      haveBestFit = false;
    }
    secondBestFitsItr++;
  }
  if (haveBestFit) {
    if (nearFits != NULL) {
      if (bestFitContext != NULL) {
        nearFits->push_front(bestFitContext);
      }
    }
    return (Context*)bestReturn(requested, canThrow, bestFitContext);
  } else {
    if (nearFits != NULL) {
      // There was no unique best fit - we have to do (n^2) refinement
      // comparisons:
      for (secondBestFitsItr = secondBestFits.begin();
           secondBestFitsItr != secondBestFits.end();
           secondBestFitsItr++) {
        bool doesNotRefine = true;
        vector<Context*>::iterator itr = secondBestFitsItr;

        while (++itr != secondBestFits.end()) {
          if (**secondBestFitsItr <= **itr) {
            doesNotRefine = false;
            break;
          }
        }
        if (doesNotRefine) {
          nearFits->push_back(*secondBestFitsItr);
        }
      }
    }
    return (Context*)bestReturn(requested, canThrow, NULL);
  }
}


Context* ContextDomain::best
(const ContextDomain::DomainElement* firstDomainElement,
 const Context& requested, bool canThrow)
{
  return fit(firstDomainElement, requested, NULL, canThrow);
}


list<Context*> ContextDomain::near
(const ContextDomain::DomainElement* firstDomainElement,
 const Context& requested)
{
  list<Context*> returnValue;

  fit(firstDomainElement, requested, &returnValue, false);
  return returnValue;
}


Context* ContextDomain::near
(const ContextDomain::DomainElement* firstDomainElement,
 list<Context*>& target, const Context& requested)
{
  return fit(firstDomainElement, requested, &target, false);
}
