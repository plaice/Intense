// ****************************************************************************
//
// SetContextDomain.hpp : ContextDomain implemented as an STL set of Contexts.
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


#ifndef __SETCONTEXTDOMAIN_HPP__
#define __SETCONTEXTDOMAIN_HPP__


#include <iostream>


namespace intense {


  struct LessThanContextPtr {

    bool operator()
    (const Context* c1, const Context* c2)
    {
      return c1->lessThan(*c2);
    }

  };


  typedef std::set<Context*, LessThanContextPtr> ContextSet;


  class SetContextDomain
    : public ContextDomain, public ContextSet {

    class SetDomainElement
      : public DomainElement {

      ContextSet* contextSetPtr;

      ContextSet::iterator itr;

    public:

      SetDomainElement
      (const ContextSet& contextSet)
        : contextSetPtr(&((ContextSet&)contextSet)), itr(contextSet.begin())
      {
      }

      virtual DomainElement* getNext
      ()
      {
        // Filthy, rotten iterator trickery:
        return (++itr == contextSetPtr->end()) ? NULL : this;
      }

      virtual Context* getContext
      ()
      {
        return *itr;
      }

    };

  public:

    SetContextDomain
    ()
      : ContextDomain(), ContextSet()
    {}

    virtual ~SetContextDomain
    ();

    virtual const DomainElement* getFirstDomainElement
    () const
    {
      return new SetDomainElement(*this);
    }

    // Returns NULL on failure; failure means a non-unique or non-existent
    // best fit:
    virtual Context* best
    (const Context& requested, bool canThrow = true) const;

    // Returns a list of pointers to near-fit Contexts in the domain.
    // If the list has only one element, that is a pointer to the best-fit
    // Context.  No Context in the returned set refines to any other
    // Context in the return set:
    virtual std::list<Context*> near
    (const Context& requested) const;

    // Insertion:
    // Returns true if we actually inserted the new item:
    virtual bool insert
    (const Context* context, bool canReplace = true);

    virtual void remove
    (Context* context)
    {
      erase((Context*)context);
    }

    virtual void clear
    ()
    {
      ContextSet::clear();
    }

  };


}


#endif //  __SETCONTEXTDOMAIN_HPP__
