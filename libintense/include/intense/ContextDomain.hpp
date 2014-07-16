// ****************************************************************************
//
// ContextDomain.hpp : An abstract list of Contexts comprising the basis for an
// intensional context domain, for use with best fits, near fits, etc.  Can be
// used statically, provided the actual domain is supplied in the form of
// objects of some concrete ContextDomain::DomainElement implementation.
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


#ifndef __CONTEXTDOMAIN_HPP__
#define __CONTEXTDOMAIN_HPP__


namespace intense {


  // A generic template "binder" for attaching objects to contexts.
  // i.e., "Version" as in "a version of _something_".
  template <class Bound>
  class Version
    : public Context {

  private:

    Bound* bound;

    // No creating empty versions!
    Version
    ()
    {}

  public:

    ~Version
    ()
    {
      delete bound;
    }

    Version
    (Bound* bound_)
      : Context(), bound(bound_)
    {}

    template <class ContextConstructorArg>
    Version
    (Bound* bound_, ContextConstructorArg& contextConstructorArg)
      :
      Context(contextConstructorArg), bound(bound_)
    {}

    Bound& getBound
    ()
    {
      return *bound;
    }

  };


  class ContextDomain {

  public:

    class DomainElement {

    public:

      DomainElement
      ()
      {
      }

      // Returns NULL when end of iteration is reached (subsequent calls must
      // also return NULL):
      virtual DomainElement* getNext
      () = 0;

      virtual Context* getContext
      () = 0;

    };

  private:

    // Returns NULL on failure; failure means a non-unique or non-existent
    // best fit:
    static Context* fit
    (const DomainElement* firstDomainElement, const Context& requested,
     std::list<Context*>* nearTarget, bool canThrow = true);

  public:

    ContextDomain
    ()
    {}

    virtual ~ContextDomain
    ()
    {
    }

    virtual const DomainElement* getFirstDomainElement
    () const = 0;

    static Context* best
    (const ContextDomain::DomainElement* firstDomainElement,
     const Context& requested, bool canThrow);

    // Returns NULL on failure; failure means a non-unique or non-existent best
    // fit:
    virtual Context* best
    (const Context& requested, bool canThrow = true) const
    {
      return ((ContextDomain*)this)->best(
        getFirstDomainElement(), requested, canThrow
      );
    }

    // Returns a list of pointers to near-fit Contexts in the domain.  If the
    // list has only one element, that is a pointer to the best-fit Context.
    // No Context in the returned set refines to any other Context in the
    // return set:
    static std::list<Context*> near
    (const DomainElement* firstDomainElement, const Context& requested);

    // Populates a list of pointers to near-fit Contexts in the domain.  If the
    // list has only one element, that is a pointer to the best-fit Context.
    // No Context in the returned set refines to any other Context in the
    // return set.  Returns a best-fit context, if any, in which case the size
    // of the result target should be one (i.e., the target contains one
    // element, being the best fit):
    static Context* near
    (const DomainElement* firstDomainElement, std::list<Context*>& target,
     const Context& requested);

    // Returns a list of pointers to near-fit Contexts in the domain.  If the
    // list has only one element, that is a pointer to the best-fit Context.
    // No Context in the returned set refines to any other Context in the
    // return set:
    virtual std::list<Context*> near
    (const Context& requested) const
    {
      return ((ContextDomain*)this)->near(getFirstDomainElement(), requested);
    }

    virtual Context* near
    (std::list<Context*>& target, const Context& requested) const
    {
      return ((ContextDomain*)this)->near(
        getFirstDomainElement(), target, requested
      );
    }

    // Insertion / Removal:
    // Returns true if we actually inserted the new item:
    virtual bool insert
    (const Context* context, bool canReplace = true) = 0;

    // Returns true if we actually removed an item with the given context:
    virtual void remove
    (const Context* context) = 0;

    virtual void clear
    () = 0;

  };


}


#endif //  __CONTEXTDOMAIN_HPP__
