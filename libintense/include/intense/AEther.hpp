// ****************************************************************************
//
// AEther.hpp : Contexts with participants.
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


#ifndef __AETHER_HPP__
#define __AETHER_HPP__


namespace intense {


  class AEther;


  // Pure interface for participants in AEP 2:
  class Participant
    : public Origin {

  protected:

    Participant
    ()
    {}

    virtual ~Participant
    ()
    {}

  public:

    // Receive, and possibly handle, notification of a context assignment to
    // the aether under which this participant is registered, under a given
    // subdimension (NULL, if no subdimension).
    virtual void assignNotify
    (const Context& context, const CompoundDimension* dim, Origin* origin) = 0;

    // Receive, and possibly handle, notification of a context operation
    // applied to the aether under which this participant is registered, under
    // a given subdimension (NULL, if no subdimension):
    virtual void applyNotify
    (const ContextOp& o, const CompoundDimension* dim, Origin* origin) = 0;

    // Receive, and possibly handle, notification of the clearing of the aether
    // under which this participant is registered, under a given subdimension
    // (NULL, if no subdimension):
    virtual void clearNotify
    (const CompoundDimension* dim, Origin* origin) = 0;

    // This is to notify the Participant that it has been removed from
    // the calling AEther's list of Participants.  It should be assumed
    // that the caller will delete this Participant shortly after
    // kickNotify() returns.
    virtual void kickNotify
    (Origin* origin) = 0;

    // This method is called prior to notification of an assign or a clear -
    // if it returns true, the participant is considered a "pure" participant,
    // i.e., one which receives only context operations and kick notifications,
    // requiring that assignments and clears be converted to context operations
    // prior to notification via applyNotify().
    virtual bool isPure
    () = 0;

  };


  class AEtherParticipant
    : virtual public Participant
  {

    friend class AEther;

  protected:

    AEther* aether;

    std::list<AEtherParticipant*>::iterator location;

  public:

    AEtherParticipant
    ()
      : Participant(), aether(NULL)
    {}

    AEtherParticipant
    (AEther& a)
      : Participant(), aether(NULL)
    {
      join(a);
    }

    virtual ~AEtherParticipant
    ();

    virtual void leave
    ();

    virtual void join
    (AEther& a);

    virtual long assign
    (const Context& context, const CompoundDimension* dim);

    virtual long apply
    (const ContextOp& op, const CompoundDimension* dim);

    virtual long clear
    (const CompoundDimension* dim);

    AEther& getNode
    ();

  };


  class AEther
    : public Context {

    friend class AEtherParticipant;

    std::list<AEtherParticipant*> participants;

    short unsigned int headcount;

  public:

    virtual Type getType
    ()
    {
      return AETHER;
    }

  protected:

    AEther
    (AEther& _parent, const DimensionRef& _parentDim)
      : Context(&_parent, &_parentDim), headcount(0)
    {}

    virtual void checkedClear
    (ContextMap::iterator& itr, Origin* origin);

    virtual void checkedClearAndPropagateNotify
    (ContextMap::iterator& itr, Origin* origin);

    // Notifies only our participants:
    virtual void applyNotify
    (const ContextOp& o, Origin* origin);

    virtual void propagateApplyNotify
    (const ContextOp& o, Origin* origin);

    virtual void assignNotify
    (const Context& context, Origin* origin);

    // Potential conversion of context to an operation:
    virtual void assignNotify
    (Participant& participant, const Context& context,
     const CompoundDimension* dimension, Origin* origin);

    virtual void propagateAssignNotify
    (const Context& context, Origin* origin);

    virtual void clearNotify
    (Origin* origin);

    // Potential conversion of clear to an operation:
    virtual void clearNotify
    (Participant& participant, const CompoundDimension* dimension,
     Origin* origin);

    virtual void propagateClearNotify
    (Origin* origin);

    virtual AEther& valueRef
    (const DimensionRef& nonCompoundDimension)
    {
      return (AEther&)Context::valueRef(nonCompoundDimension);
    }

    // Overridden new-AEther/Context constructor for operator[] helper:
    virtual Context* valueRefFactory
    (const DimensionRef& dim)
    {
      return new AEther(*this, dim);
    }

    const unsigned short int& headCount
    ()
    {
      return headcount;
    }

    // Clear this context, without propagating the loss of basecount to
    // ancestors.  In the case of AEther, clearNoPropagate() is a recursive
    // selective pruner, that clears out all base values in and beneath *this.
    // Ancestors are not notified of the basecount delta.  All branches without
    // participants are pruned:
    virtual void clearNoPropagate
    (Origin* origin);

  public:

    inline AEther
    ()
      : Context(), participants(), headcount(0)
    {}

    inline AEther
    (ContextOp& v)
      : Context(), participants(), headcount(0)
    {
      apply(v);
    }

    inline AEther
    (AEther& a)
      : Context(), participants(), headcount(0)
    {
      assign(a);
    }

    // Depth-first deletion with participant "kick" notification and deletion:
    virtual ~AEther
    ();

    virtual std::list<AEtherParticipant*>::iterator attach
    (AEtherParticipant& p);

    virtual void detach
    (std::list<AEtherParticipant*>::iterator i);

    virtual AEther& value
    (const DimensionRef& dim)
    {
      return (AEther&)(Context::value(dim));
    }

    virtual AEther& operator[]
    (const DimensionRef& dim)
    {
      return value(dim);
    }

    virtual AEther& value
    (const CompoundDimension& dim)
    {
      return (AEther&)(Context::value(dim));
    }

    virtual AEther& value
    (const CompoundDimension* dim)
    {
      return (AEther&)(Context::value(dim));
    }

    virtual AEther& operator[]
    (const CompoundDimension& dim)
    {
      return value(dim);
    }

    virtual AEther& value
    (const char* dim)
    {
      return (AEther&)(Context::value(dim));
    }

    virtual AEther& operator[]
    (const char* dim)
    {
      return value(dim);
    }

    virtual AEther& value
    (const std::string& dim)
    {
      return (AEther&)(Context::value(dim));
    }

    virtual AEther& operator[]
    (const std::string& dim)
    {
      return value(dim);
    }

    virtual AEther& value
    (int dim)
    {
      return (AEther&)(Context::value(dim));
    }

    virtual AEther& assign
    (const Context& c)
    {
      return assign(c, NULL);
    }

    virtual AEther& assign
    (const Context& c, Origin* origin);

    virtual AEther& apply
    (const ContextOp& o)
    {
      return apply(o, NULL);
    }

    virtual AEther& apply
    (const ContextOp& o, Origin* origin);

    virtual void clear
    ()
    {
      clear(NULL);
    }

    virtual void clear
    (Origin* origin);

    virtual void clear
    (const CompoundDimension& dim)
    {
      clear(dim, NULL);
    }

    virtual void clear
    (const CompoundDimension& dim, Origin* origin);

    virtual AEther& operator[]
    (int dim)
    {
      return value(dim);
    }

    virtual void setBase
    (const BaseValue& bv, Origin* origin = NULL);

    virtual void setBase
    (const BaseValue* bv)
    {
      setBase(bv, NULL);
    }

    virtual void setBase
    (const BaseValue* bv, Origin* origin);

    virtual AEther& operator=
    (const BaseValue& bv)
    {
      return (AEther&)Context::operator=(bv);
    }

    virtual AEther& operator=
    (const BaseValue* bv)
    {
      return (AEther&)Context::operator=(bv);
    }

    virtual AEther& operator=
    (const std::string& s)
    {
      return (AEther&)Context::operator=(s);
    }

    virtual AEther& operator=
    (const char *s)
    {
      return (AEther&)Context::operator=(s);
    }

    virtual void clearBase
    (Origin* origin = NULL);

    virtual void kick
    (bool kickInChildNodes = true, Origin* origin = NULL);

    void parse
    (std::istream& is)
    {
      parseStream(is, NULL);
    }

    void parse
    (std::istream& is, Origin* origin)
    {
      parseStream(is, origin);
    }

    void parse
    (const char* input)
    {
      parse(input, NULL);
    }

    void parse
    (const char* input, Origin* origin);

    void parse
    (const std::string& s)
    {
      return parse(s.data(), NULL);
    }

    void parse
    (const std::string& s, Origin* origin)
    {
      return parse(s.data(), origin);
    }

    virtual void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is,
     BoundManager* boundSerialiser)
    {
      deserialise(baseSerialiser, is, boundSerialiser, NULL);
    }

    virtual void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is)
    {
      deserialise(baseSerialiser, is, NULL, NULL);
    }

    virtual void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is,
     BoundManager* boundSerialiser, Origin* origin);

  protected:

    virtual void parseStream
    (std::istream& is)
    {
      parseStream(is, NULL);
    }

    virtual void parseStream
    (std::istream& is, Origin* origin);

    virtual const char* className
    () const
    {
      return "AEther";
    }

  };


}


#endif //  __AETHER_HPP__
