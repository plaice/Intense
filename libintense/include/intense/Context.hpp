// ****************************************************************************
//
// Context.hpp : An intensional context class hierarchy.
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


#ifndef __CONTEXT_HPP__
#define __CONTEXT_HPP__


#include "intense_config.hpp"


namespace intense {


#if DEFINITELY_HAVE_POSIX_SPINLOCKS


  // This belongs elsewhere - but hey - a lot of things belong elsewhere:
  class Referenced {

  private:

    unsigned int refCount;

    pthread_spinlock_t spinLock;

  public:

    Referenced
    (unsigned int newRefCount)
      : refCount(newRefCount)
    {
      pthread_spin_init(&spinLock, PTHREAD_PROCESS_SHARED);
    }

    Referenced
    ()
      : refCount(0)
    {
      pthread_spin_init(&spinLock, PTHREAD_PROCESS_SHARED);
    }

    virtual ~Referenced
    ()
    {
      if (refCount > 0) {
        std::ostringstream oss;

        oss << "Non-zero refCount " << refCount << " in ~Referenced()";
        throw IntenseException(oss.str());
      }
      pthread_spin_destroy(&spinLock);
    }

    void lock
    ()
    {
      int rc;

      if ((rc = pthread_spin_lock(&spinLock)) != 0) {
        std::ostringstream oss;

        oss << "non-zero spin lock rc " << rc << " (" << strerror(rc) << ").";
        throw IntenseException(oss.str());
      }
    }

    void unlock
    ()
    {
      int rc;

      if ((rc = pthread_spin_unlock(&spinLock)) != 0) {
        std::ostringstream oss;

        oss << "non-zero spin unlock rc " << rc
            << " (" << strerror(rc) << ").";
        throw IntenseException(oss.str());
      }
    }

    int decrementRefCount
    ()
    {
      int returnValue;

      lock();
      returnValue = --refCount;
      unlock();
      if (returnValue == 0) {
        delete this;
        // Note, in the case where someone was waiting on the spinlock to
        // increment, they will now have a invalid reference, and "delete this"
        // calls pthread_spin_destroy which is undefined if another thread
        // holds the lock - however, it also means they were fooling around
        // trying to create a reference to a Referenced that was in the process
        // of being deallocated, so this is fair.  There's probably a better
        // way...
      }
      return returnValue;
    }

    int incrementRefCount
    ()
    {
      int returnValue;

      lock();
      returnValue = ++refCount;
      unlock();
      return returnValue;
    }

    int getRefCount
    ()
    {
      int returnValue;

      lock();
      returnValue = refCount;
      unlock();
      return returnValue;
    }

  };


#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS


  // Abstract base class for context base values:
#if DEFINITELY_HAVE_POSIX_SPINLOCKS
  class BaseValue
    : protected Referenced {
#else // DEFINITELY_HAVE_POSIX_SPINLOCKS
  class BaseValue {
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS

    friend class Context;
    friend class ContextOp;

  protected:

#if DEFINITELY_HAVE_POSIX_SPINLOCKS
    BaseValue
    ()
      : Referenced()
    {}
#else // DEFINITELY_HAVE_POSIX_SPINLOCKS
    BaseValue
    ()
    {}
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS

  public:

    typedef enum {
      VOID, ALPHA, OMEGA, NUMBER, STRING, BINARYSTRING, BOUND
    } Type;

    virtual ~BaseValue
    ()
    {}

    // Canonical form generator:
    virtual std::string canonical() const = 0;

    // Get the content of the base value as a string:
    virtual std::string toString() const = 0;

    virtual BaseValue* clone() const = 0;

    virtual bool equals
    (const BaseValue&) const = 0;

    bool operator==
    (const BaseValue& b) const
    {
      return equals(b);
    }

    virtual bool refinesTo
    (const BaseValue&) const = 0;

    bool operator<=
    (const BaseValue& b) const
    {
      return refinesTo(b);
    }

    virtual bool lessThan
    (const BaseValue&) const = 0;

    bool operator<
    (const BaseValue& b) const
    {
      return lessThan(b);
    }

    virtual BaseValue& assign
    (const BaseValue&) = 0;

    BaseValue& operator=
    (const BaseValue& b)
    {
      return assign(b);
    }

    virtual Type getType
    () const = 0;

    // A parametrized factory for all subclasses:
    static BaseValue* factory
    (Type objectType);

    virtual void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os) const = 0;

    virtual void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is) = 0;

    static void factorySerialise
    (BaseValue* bv, io::BaseSerialiser& baseSerialiser, std::ostream& os,
     BoundManager* manager);

    static BaseValue* factoryDeserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is,
     BoundManager* manager);

  };


  class Context;
  class ContextOp;
  class ContextOpLexer;
  class DimensionRef;


  // Pure interface:
#if DEFINITELY_HAVE_POSIX_SPINLOCKS
  class Dimension
    : protected Referenced {
#else // DEFINITELY_HAVE_POSIX_SPINLOCKS
  class Dimension {
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS

    friend class DimensionRef;

  public:

    typedef enum { VOID, STRING, INTEGER } Type;

    Dimension
    ()
#if DEFINITELY_HAVE_POSIX_SPINLOCKS
      : Referenced()
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS
    {}

    virtual ~Dimension
    ()
    {}

    virtual bool lessThan
    (const Dimension& d) const = 0;

    bool operator<
    (const Dimension& d) const
    {
      return lessThan(d);
    }

    virtual bool equals
    (const Dimension& d) const = 0;

    virtual bool operator==
    (const Dimension& d) const
    {
      return equals(d);
    }

    virtual bool operator!=
    (const Dimension& d) const
    {
      return !equals(d);
    }

    // We'll leave this as a non-reference for now, given than numeric
    // and unicode-based dimensions will not be directly castable to strings.
    // And really, it should be a std::string<wchar_t> (later)...
    virtual std::string canonical
    () const = 0;

    virtual Type getType
    () const = 0;

    virtual Dimension* clone
    () const = 0;

    static Dimension* factory
    (Type type);

    virtual void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os) const = 0;

    virtual void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is) = 0;

    static void factorySerialise
    (Dimension* d, io::BaseSerialiser& baseSerialiser, std::ostream& os);

    static Dimension* factoryDeserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is);

  };


  class StringDimension
    : public Dimension {

    const static int maxLength = 1024;

    std::string value;

    bool requiresQuotes;

  public:

    StringDimension
    ()
      : Dimension(), value(), requiresQuotes(false)
    {}

    StringDimension
    (const std::string& newString)
      : Dimension(), value(newString), requiresQuotes(true)
    {}

    StringDimension
    (const std::string& newString, bool requiresQuotes_)
      : Dimension(), value(newString), requiresQuotes(requiresQuotes_)
    {}

    StringDimension
    (const char* newString)
      : Dimension(), value(newString), requiresQuotes(true)
    {}

    StringDimension
    (const char* newString, bool requiresQuotes_)
      : Dimension(), value(newString), requiresQuotes(requiresQuotes_)
    {}

    virtual ~StringDimension
    ()
    {}

    bool lessThan
    (const Dimension& d) const
    {
      switch (((Dimension&)d).getType()) {
      case STRING:
        return value < ((StringDimension&)d).value;
      case INTEGER:
        return false;
      }
    }

    virtual bool equals
    (const Dimension& d) const
    {
      switch (((Dimension&)d).getType()) {
      case STRING:
        return value == ((StringDimension&)d).value;
      case INTEGER:
        return false;
      }
    }

    std::string canonical
    () const;

    Type getType
    () const
    {
      return STRING;
    }

    Dimension* clone
    () const
    {
      return new StringDimension(*this);
    }

    void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

    void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is);

  };


  class IntegerDimension
    : public Dimension {

    int value;

  public:

    IntegerDimension
    ()
      : Dimension(), value(0)
    {}

    IntegerDimension
    (const IntegerDimension& src)
      : Dimension(), value(src.value)
    {}

    IntegerDimension
    (int newValue)
      : Dimension(), value(newValue)
    {}

    virtual ~IntegerDimension
    ()
    {}

    bool lessThan
    (const Dimension& d) const
    {
      switch (((Dimension&)d).getType()) {
      case INTEGER:
        return value < ((IntegerDimension&)d).value;
      case STRING:
        return true;
      }
    }

    virtual bool equals
    (const Dimension& d) const
    {
      switch (((Dimension&)d).getType()) {
      case INTEGER:
        return value == ((IntegerDimension&)d).value;
      case STRING:
        return false;
      }
    }

    std::string canonical
    () const
    {
      std::ostringstream oss;

      oss << value;
      return oss.str();
    }

    Type getType
    () const
    {
      return INTEGER;
    }

    Dimension* clone
    () const
    {
      return new IntegerDimension(*this);
    }

    void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os) const
    {
      baseSerialiser.out(value, os);
    }

    void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is)
    {
      baseSerialiser.in(value, is);
    }

  };


  // For use in maps.  Ideally, this is a non-cloning true reference to a
  // shared Dimension*, with reference counting in Dimension, but for that we
  // need spinlocks in every dimension as well, since multiple threads would
  // end up incrementing and decrementing the Dimension refcounts of the same
  // Dimensions used in different Contexts and ContextOps.  If we don't have
  // POSIX pthreads spinlocks, we'll clone and waste the memory.
  class DimensionRef {

    friend class CompoundDimension;
    friend class Context;

  private:

    Dimension* dimension;

    DimensionRef
    ()
      : dimension(NULL)
    {
      throw IntenseException(
        "INTERNAL ERROR: DimensionRef::DimensionRef() called"
      );
    }

    DimensionRef
    (io::BaseSerialiser& baseSerialiser, std::istream& is)
      : dimension(Dimension::factoryDeserialise(baseSerialiser, is))
    {
#if DEFINITELY_HAVE_POSIX_SPINLOCKS
      dimension->incrementRefCount();
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS
    }

  public:

    DimensionRef
    (const Dimension& d)
      : dimension((Dimension*)&d)
    {
#if DEFINITELY_HAVE_POSIX_SPINLOCKS
      dimension->incrementRefCount();
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS
    }

#if DEFINITELY_HAVE_POSIX_SPINLOCKS
    DimensionRef
    (const DimensionRef& r)
      : dimension(r.dimension)
    {
      dimension->incrementRefCount();
    }
#else // DEFINITELY_HAVE_POSIX_SPINLOCKS
    DimensionRef
    (const DimensionRef& r)
      : dimension(r.dimension->clone())
    {}
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS

    ~DimensionRef
    ()
    {
#if DEFINITELY_HAVE_POSIX_SPINLOCKS
      dimension->decrementRefCount();
#else // DEFINITELY_HAVE_POSIX_SPINLOCKS
      delete dimension;
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS
    }

#if DEFINITELY_HAVE_POSIX_SPINLOCKS
    void setDimension
    (Dimension& d)
    {
      dimension->decrementRefCount();
      dimension = &d;
      dimension->incrementRefCount();
    }
#else // DEFINITELY_HAVE_POSIX_SPINLOCKS
    void setDimension
    (const Dimension& d)
    {
      delete dimension;
      dimension = &((Dimension&)d);
    }
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS

    bool lessThan
    (const DimensionRef& d) const
    {
      return dimension->lessThan(*(d.dimension));
    }

    bool operator<
    (const DimensionRef& d) const
    {
      return lessThan(d);
    }

    virtual bool equals
    (const DimensionRef& d)
    {
      return dimension->equals(*(((DimensionRef&)d).dimension));
    }

    virtual bool operator==
    (const DimensionRef& d) const
    {
      return dimension->equals(*(((DimensionRef&)d).dimension));
    }

    virtual bool operator!=
    (const DimensionRef& d) const
    {
      return !dimension->equals(*(((DimensionRef&)d).dimension));
    }

    std::string canonical
    () const
    {
      return dimension->canonical();
    }

    Dimension& dereference
    () const
    {
      return *dimension;
    }

    Dimension& operator*
    () const
    {
      return dereference();
    }

    DimensionRef& operator=
    (const DimensionRef& ref)
    {
#if DEFINITELY_HAVE_POSIX_SPINLOCKS
      dimension->incrementRefCount();
      dimension = ref.dimension;
#else // DEFINITELY_HAVE_POSIX_SPINLOCKS
      delete dimension;
      dimension = ref.dimension->clone();
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS
    }

    virtual void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os) const
    {
      Dimension::factorySerialise(dimension, baseSerialiser, os);
    }

    virtual void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is)
    {
      if (dimension != NULL) {
#if DEFINITELY_HAVE_POSIX_SPINLOCKS
        dimension->decrementRefCount();
#else // DEFINITELY_HAVE_POSIX_SPINLOCKS
        delete dimension;
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS
      }
      dimension = Dimension::factoryDeserialise(baseSerialiser, is);
#if DEFINITELY_HAVE_POSIX_SPINLOCKS
      if (dimension != NULL) {
        dimension->incrementRefCount();
      }
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS
    }

  };


  class CompoundDimension
    : public std::list<DimensionRef>, public io::Serialisable {

    static const int maxCompoundDimensionSize = 1024;

  public:

    CompoundDimension
    ()
      : std::list<DimensionRef>(), Serialisable()
    {}

    CompoundDimension
    (const char* src)
      : std::list<DimensionRef>(), Serialisable()
    {
      parse(src);
    }

    CompoundDimension
    (const std::string& src)
      : std::list<DimensionRef>(), Serialisable()
    {
      parse(src);
    }

    CompoundDimension
    (const CompoundDimension& src)
      : std::list<DimensionRef>(src), Serialisable()
    {}

    // Add a dimension:
    void append
    (const DimensionRef& d)
    {
      push_back(d);
    }

    // Add a dimension:
    void append
    (Dimension& d)
    {
      DimensionRef ref(d);

      push_back(ref);
    }

    // Add a dimension:
    void prepend
    (const DimensionRef& d)
    {
      push_front(d);
    }

    // Add a dimension:
    void prepend
    (Dimension& d)
    {
      DimensionRef ref(d);

      push_front(ref);
    }

    std::string canonical
    () const;

  private:

    void parse
    (std::istream& src);

  public:

    void parse
    (const char* input);

    void parse
    (const std::string& src)
    {
      return parse(src.data());
    }

    virtual void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

    virtual void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is);

  };


  // An interface base for Participant and other sources of AEther operations:
  class Origin {

  protected:

    Origin
    ()
    {}

    virtual ~Origin
    ()
    {}

  };


  struct LTDimensionRefPtr {

    bool operator()
    (const DimensionRef& d1, const DimensionRef& d2) const
    {
      return d1.lessThan(d2);
    }

  };


  // Note this is a map of pointers to Contexts, so that
  // subclasses of Context can aggregate themselves, and not just more
  // Contexts:
  typedef std::map<DimensionRef, Context*, LTDimensionRefPtr> ContextMap;


  class IntenseLexer;


  class Context
    : protected ContextMap, public io::Serialisable {

    friend class ContextOp;

  public:

    typedef enum { VOID, CONTEXT, CONTEXTOP, AETHER } Type;

    static const char* typeStrings[];

    const char* getTypeString
    ()
    {
      return typeStrings[getType()];
    }

    static const char* getTypeString
    (Type type)
    {
      return typeStrings[type];
    }

    virtual Type getType
    ()
    {
      return CONTEXT;
    }

    static Context* factory
    (Type type);

    static void factorySerialise
    (const Context* context, io::BaseSerialiser& baseSerialiser,
     std::ostream& os);

    static Context* factoryDeserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is);

    Context* findNode
    (CompoundDimension& dimension);

    Context* findNode
    (const char* compoundDimension);

    Context* findDescendant
    (int dimension);

  protected:

    // Simple struct for ContextOp applyHelper return types:
    struct OpHelperRC {
      int deltaBlankcount;
      int deltaBasecount;
    };

    const Context* parent;

    const DimensionRef* parentDimension;

    std::string* canonicalString;

    int basecount;

    void deleteBasePtr
    ()
    {
      if (base != NULL) {
#if DEFINITELY_HAVE_POSIX_SPINLOCKS
        base->decrementRefCount();
#else // DEFINITELY_HAVE_POSIX_SPINLOCKS
        delete base;
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS
        base = NULL;
      }
    }

    // N.B. Does NOT deleteBasePtr(), first:
    void dupBasePtr
    (BaseValue* newBase)
    {
#if DEFINITELY_HAVE_POSIX_SPINLOCKS
      base = newBase;
      if (newBase != NULL) {
        base->incrementRefCount();
      }
#else // DEFINITELY_HAVE_POSIX_SPINLOCKS
      if (newBase != NULL) {
        base = newBase->clone();
      } else {
        base = NULL;
      }
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS
    }

    // N.B. Does NOT deleteBasePtr(), first:
    void setBasePtr
    (BaseValue* newBase)
    {
      base = newBase;
#if DEFINITELY_HAVE_POSIX_SPINLOCKS
      if (base != NULL) {
        base->incrementRefCount();
      }
#endif // DEFINITELY_HAVE_POSIX_SPINLOCKS
    }

    void assignHelper
    (const Context& c, Origin* origin);

    virtual void checkedClear
    (ContextMap::iterator& itr, Origin* origin);

    virtual void applyHelper
    (const ContextOp& o, Origin* origin);

    static Context::OpHelperRC recordDeltaHelper
    (const Context* c, const ContextOp& op, ContextOp& delta);

    // For the simplification of Context/AEther co-implementation:

    virtual void applyNotify
    (const ContextOp& o, Origin* origin)
    {}

    virtual void assignNotify
    (const Context& context, Origin* origin)
    {}

    // Adds a delta to the basecounts of all ancestors of this Context:
    void propagateBasecount
    (int basecountDelta);

    virtual Context& valueRef
    (const DimensionRef& dim);

    virtual Context* valueRefFactory
    (const DimensionRef& dim);

    std::string canonicalHelper
    (bool retain);

    void clearCanonicalString
    ();

    virtual void clearNoPropagate
    ();

    BaseValue* base;

  public:

    Context
    ()
      : ContextMap(), io::Serialisable(), parent(NULL), parentDimension(NULL),
        canonicalString(NULL), basecount(0), base(NULL)
    {}

    Context
    (const std::string& canonicalForm)
      : ContextMap(), io::Serialisable(), parent(NULL), parentDimension(NULL),
        canonicalString(NULL), basecount(0), base(NULL)
    {
      parse(canonicalForm);
    }

    Context
    (const char* canonicalForm)
      : ContextMap(), io::Serialisable(), parent(NULL), parentDimension(NULL),
        canonicalString(NULL), basecount(0), base(NULL)
    {
      parse(canonicalForm);
    }

    Context
    (const Context* _parent, const DimensionRef* _parentDimension)
      : ContextMap(), parent(_parent),
        parentDimension(_parentDimension),
        canonicalString(NULL), basecount(0), base(NULL)
    {}

    Context
    (const Context* _parent, const DimensionRef* _parentDimension,
     const Context& c)
      : ContextMap(), io::Serialisable(), parent(_parent),
        parentDimension(_parentDimension), canonicalString(NULL),
        basecount(c.basecount), base(NULL)
    {
      assign(c);
    }

    Context
    (const Context& c)
      : ContextMap(), io::Serialisable(), parent(NULL), parentDimension(NULL),
        canonicalString(NULL), basecount(c.basecount), base(NULL)
    {
      assign(c);
    }

    virtual ~Context
    ();

    virtual bool isEmpty
    () const
    {
      return basecount == 0;
    }

    virtual ContextMap& mapRef
    ()
    {
      return *this;
    }

    virtual Context* clone
    () const
    {
      return new Context(*this);
    }

    const Context* getParent
    () const
    {
      return parent;
    }

    const DimensionRef* getParentDimension
    () const
    {
      return parentDimension;
    }

    int getDepth
    ();

    Context& leastCommonRoot
    (Context& node);

    // Context equality:
    virtual bool equals
    (const Context&) const;

    // Context equality:
    bool operator==
    (const Context& c) const
    {
      return equals(c);
    }

    // Intensional refinement:
    virtual bool refinesTo
    (const Context&) const;

    // Intensional refinement:
    bool operator<=
    (const Context& c) const
    {
      return refinesTo(c);
    }

    // Lexicographic comparison:
    virtual int compare
    (const Context&) const;

    // Lexicographic comparison:
    virtual bool lessThan
    (const Context& c) const
    {
      return compare(c) == -1;
    }

    // Lexicographic comparison:
    bool operator<
    (const Context& c) const
    {
      return lessThan(c);
    }

    // Non-vanilla copy (vset):
    virtual Context& assign
    (const Context& c);

    // Non-vanilla copy (vset):
    Context& operator=
    (const Context& c)
    {
      return assign(c);
    }

    // Apply operation:
    virtual Context& apply
    (const ContextOp& o);

    // Apply operation:
    Context& operator=
    (const ContextOp& o)
    {
      return apply(o);
    }

    // Record total delta resulting from the application of op to this context,
    // as a minimal op, into delta (i.e., applying the resulting delta to *this
    // yields the same context as applying op).  Note, this is really just op,
    // minus any unnecessary blanking or pruning:
    virtual void recordDelta
    (const ContextOp& op, ContextOp& delta) const;

    // Map of subcontexts:
    virtual Context& value
    (const DimensionRef& dim)
    {
      return valueRef(dim);
    }

    virtual Context& operator[]
    (const DimensionRef& dim)
    {
      return valueRef(dim);
    }

    virtual Context& value
    (const CompoundDimension&);

    virtual Context& value
    (const CompoundDimension*);

    virtual Context& operator[]
    (const CompoundDimension& dim)
    {
      return value(dim);
    }

    virtual Context& value
    (const char* dim)
    {
      CompoundDimension compoundDimension(dim);

      return value(compoundDimension);
    }

    void findPathTo
    (const Context& descendant, CompoundDimension& result);

    // Returns -1 if this node is a descendant of the argument node, 0 if this
    // node is the same node as the argument node, and 1 if this node is an
    // ancestor of the argument node.  If path is non-NULL, it is cleared and
    // populated with the path from the ancestor to the descendant.
    int getAncestry
    (const Context& node, CompoundDimension* path);

    virtual Context& operator[]
    (const char* dim)
    {
      return value(dim);
    }

    virtual Context& value
    (const std::string& dim)
    {
      return value(dim.data());
    }

    virtual Context& operator[]
    (const std::string& dim)
    {
      return value(dim);
    }

    virtual Context& value
    (const int dim)
    {
      IntegerDimension dimension(dim);

      return value(dim);
    }

    virtual Context& operator[]
    (const int dim)
    {
      return value(dim);
    }

    virtual std::string canonical
    (bool retain = false, bool retainInChildren = false);

    virtual std::string shortCanonical
    (bool retain = false, bool retainInChildren = false)
    {
      return canonical(retain, retainInChildren);
    }

    // BaseValue Assignment:

    virtual void setBase
    (const BaseValue& bv);

    virtual Context& operator=
    (const BaseValue& bv)
    {
      setBase(bv);
      return *this;
    }

    virtual void setBase
    (const BaseValue* bv);

    virtual Context& operator=
    (const BaseValue* bv)
    {
      setBase(*bv);
      return *this;
    }

    void setBase
    (const std::string& s);

    virtual Context& operator=
    (const std::string& s)
    {
      setBase(s);
      return *this;
    }

    void setBase
    (const char* s);

    virtual Context& operator=
    (const char *s)
    {
      setBase(s);
      return *this;
    }

    // BaseValue retrieval:
    virtual BaseValue* getBase
    () const
    {
      return base;
    }

    virtual void clearBase
    ();

    virtual BaseValue* getChildBase
    (const Dimension& nonCompoundDimension) const;

    virtual void clear
    ();

    // Clear a dimension:
    virtual void clear
    (const CompoundDimension& dim);

    int baseCount
    () const
    {
      return basecount;
    }

    Context& root
    ();

    bool isAncestor
    (Context& context);


    class Iterator
      : public ContextMap::iterator {

      Context* node;

    public:

      Iterator
      (Context& node_, ContextMap::iterator itr)
        : ContextMap::iterator(itr), node(&node_)
      {}

      Iterator& operator++
      ();

      Iterator& operator++
      (int);

    };


    Iterator beginPopulated
    ();

    ContextMap::iterator begin
    ()
    {
      return ContextMap::begin();
    }

    ContextMap::iterator end
    ()
    {
      return ContextMap::end();
    }


    // Recursive-descent parsing methods and structures:


    class Token {

      friend class Context;
      friend class ContextOp;
      friend class CompoundDimension;

      static const char* typeStrings[];

    public:

      typedef enum {
        VOID, DIMENSION, BASEVALUE, DASH, DASHDASH, DASHDASHDASH, PLUS,
        LANGLE, RANGLE, LSQUARE, RSQUARE, ERROR, ENDOFFILE
      } Type;

      int type;

      union {
        BaseValue* baseValue;
        Dimension* dimension;
        std::string* errorText;
      } value;

      Token
      ()
        : type(VOID)
      {}

      ~Token
      ();

      virtual const char** getTypeStrings
      ()
      {
        return typeStrings;
      }

      int getType
      () const
      {
        return type;
      }

      virtual const char* getTypeString
      ()
      {
        return getTypeStrings()[getType()];
      }

      static const char* getTypeString
      (Type type)
      {
        return typeStrings[type];
      }

      int setVoid
      ()
      {
        return type = VOID;
      }

      int setStringDimension
      (const char* dimension, bool requiresQuotes)
      {
        value.dimension = new StringDimension(dimension, requiresQuotes);
        return type = DIMENSION;
      }

      int setIntegerDimension
      (int dimension)
      {
        value.dimension = new IntegerDimension(dimension);
        return type = DIMENSION;
      }

      int setBaseValue
      (const BaseValue& bv)
      {
        value.baseValue = &((BaseValue&)bv);
        return type = BASEVALUE;
      }

      int setDash
      ()
      {
        return type = DASH;
      }

      int setDashDash
      ()
      {
        return type = DASHDASH;
      }

      int setDashDashDash
      ()
      {
        return type = DASHDASHDASH;
      }

      int setPlus
      ()
      {
        return type = PLUS;
      }

      int setLAngle
      ()
      {
        return type = LANGLE;
      }

      int setRAngle
      ()
      {
        return type = RANGLE;
      }

      int setLSquare
      ()
      {
        return type = LSQUARE;
      }

      int setRSquare
      ()
      {
        return type = RSQUARE;
      }

      int setError
      (std::string& errorText)
      {
        value.errorText = &errorText;
        return type = ERROR;
      }

      int setEOF
      ()
      {
        return type = ENDOFFILE;
      }

    };


    void parse
    (std::istream& is)
    {
      parseStream(is);
    }

    void parse
    (const char* input);

    void parse
    (const std::string& s)
    {
      return parse(s.data());
    }

  protected:

    virtual void parseStream
    (std::istream& is);

    virtual const char* className
    ()
    {
      return "Context";
    }

  public:

    virtual void recogniseNode
    (IntenseLexer& lexer)
    {
      recogniseNode(lexer, NULL);
    }

    virtual void recogniseNode
    (IntenseLexer& lexer, Origin* origin);

    virtual void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os,
     BoundManager* boundSerialiser) const;

    virtual void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os) const
    {
      serialise(baseSerialiser, os, NULL);
    }

    static void serialise
    (Context* context, io::BaseSerialiser& baseSerialiser, std::ostream& os);

    virtual void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is,
     BoundManager* boundSerialiser);

    virtual void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is)
    {
      deserialise(baseSerialiser, is, NULL);
    }

    static Context* deserialise
    (Context* context, io::BaseSerialiser& baseSerialiser, std::istream& is);

  protected:

    virtual void deserialiseHelper
    (io::BaseSerialiser& baseSerialiser, std::istream& is,
     BoundManager* boundSerialiser, Origin* origin);

  public:

    static void dimTok
    (std::list<std::string>& dimParts, const std::string& dim);

    static void dimTok
    (std::vector<std::string>& dimParts, const std::string& dim);

  };


  // stream canonical form:
  inline std::ostream& operator<<
  (std::ostream& os, const Context& c)
  {
    return os << ((Context&)c).canonical();
  }


  class ContextOp
    : public Context {

    friend class Context;

  public:

    virtual Type getType
    ()
    {
      return CONTEXTOP;
    }

  protected:

    // flags:
    bool clearBaseFlag;

    bool clearDimsFlag;

    int blankcount;

    // ContextOp application recursive helper:
    void applyHelper
    (const ContextOp& o);

    // Adds a delta to all the blankcounts of all ancestors of this ContextOp:
    void propagateBlankcount
    (int delta);

    void assignHelper
    (const ContextOp& o);

    int applyHelper
    (const Context& c);

  protected:

    virtual ContextOp& valueRef
    (const DimensionRef& dimension)
    {
      return (ContextOp&)Context::valueRef(dimension);
    }

    // Overridden new-ContextOp/Context constructor for operator[] helper:
    virtual Context* valueRefFactory
    (const DimensionRef& dim);

    // A child factory that sets -+-- in the child if -- is set in the parent:
    virtual ContextOp* valueRefFactoryRetainPruning
    (const DimensionRef& dim);

  public:

    ContextOp
    ()
      : Context(), clearBaseFlag(0), clearDimsFlag(0), blankcount(0)
    {}

    ContextOp
    (const std::string& canonicalForm);

    ContextOp
    (const char* canonicalForm);

  protected:

    ContextOp
    (const Context* parent, const DimensionRef* parentDimension)
      : Context(parent, parentDimension), clearBaseFlag(0), clearDimsFlag(0),
        blankcount(0)
    {}

  public:

    virtual ~ContextOp
    ()
    {}

    // Exact copy constructor (minus parent):
    ContextOp
    (const ContextOp& o)
      : Context()
    {
      assign(o);
    }

    ContextOp& assign
    (const ContextOp& o);

    virtual bool isEmpty
    () const
    {
      return ((blankcount == 0)&&(basecount == 0));
    }

    virtual ContextOp* clone
    () const
    {
      return new ContextOp(*this);
    }

    // ContextOp equality:
    virtual bool equals
    (const ContextOp&) const;

    // ContextOp equality:
    virtual bool operator==
    (const ContextOp& o) const
    {
      return equals(o);
    }

    // Applying a Context to a ContextOp as an operator turns the ContextOp
    // into a pure "vset" to the argument Context (note the covariant return
    // type):
    virtual ContextOp& apply
    (const Context& c);

    virtual ContextOp& operator=
    (const Context& c)
    {
      return apply(c);
    }

    // Record total delta resulting from the application of op to this op, as a
    // minimal op, into delta (i.e., applying the resulting delta to *this
    // yields the same operator as applying op to *this):
    virtual void recordDelta
    (const ContextOp& op, ContextOp& delta) const;

    // Apply operation:
    virtual ContextOp& apply
    (const ContextOp&);

    // Apply operation:
    virtual ContextOp& operator=
    (const ContextOp& o)
    {
      return apply(o);
    }

    // Map of subcontextops:
    virtual ContextOp& value
    (const DimensionRef& dim)
    {
      return (ContextOp&)(Context::value(dim));
    }

    virtual ContextOp& operator[]
    (const DimensionRef& dim)
    {
      return value(dim);
    }

    // Map of subcontexts:
    virtual ContextOp& value
    (const CompoundDimension& dim)
    {
      return (ContextOp&)(Context::value(dim));
    }

    // Map of subcontexts:
    virtual ContextOp& value
    (const CompoundDimension* dim)
    {
      return (ContextOp&)(Context::value(dim));
    }

    virtual ContextOp& operator[]
    (const CompoundDimension& dim)
    {
      return value(dim);
    }

    virtual ContextOp& value
    (const char* dim)
    {
      return (ContextOp&)(Context::value(dim));
    }

    virtual ContextOp& operator[]
    (const char* dim)
    {
      return value(dim);
    }

    // Map of subcontexts:
    virtual ContextOp& value
    (const std::string& dim)
    {
      return (ContextOp&)(Context::value(dim));
    }

    virtual ContextOp& operator[]
    (const std::string& dim)
    {
      return value(dim);
    }

    virtual ContextOp& value
    (int dim)
    {
      return (ContextOp&)(Context::value(dim));
    }

    virtual ContextOp& operator[]
    (int dim)
    {
      return value(dim);
    }

    // Map of subcontextops that optionally retains pruning (sets -+-- in a
    // child, if -- is set in the child's parent):
    virtual ContextOp& value
    (const CompoundDimension& dim, bool retainPruning);

    virtual ContextOp& value
    (const char* dim, bool retainPruning)
    {
      CompoundDimension compoundDimension(dim);

      return value(compoundDimension, retainPruning);
    }

    virtual ContextOp& value
    (const std::string& dim, bool retainPruning)
    {
      CompoundDimension compoundDimension(dim.data());

      return value(compoundDimension, retainPruning);
    }

    virtual std::string canonical
    (bool retain = false, bool retainInChildren = false);

    virtual std::string shortCanonical
    (bool retain = false, bool retainInChildren = false);

    // BaseValue Assignment:

    virtual ContextOp& operator=
    (const BaseValue& bv)
    {
      return (ContextOp&)Context::operator=(bv);
    }

    virtual ContextOp& operator=
    (const BaseValue* bv)
    {
      return (ContextOp&)Context::operator=(bv);
    }

    virtual ContextOp& operator=
    (const std::string& s)
    {
      return (ContextOp&)Context::operator=(s);
    }

    virtual ContextOp& operator=
    (const char *s)
    {
      return (ContextOp&)Context::operator=(s);
    }

    virtual void setBase
    (const BaseValue& bv);

    bool getClearBaseFlag
    () const
    {
      return clearBaseFlag;
    }

    void setClearBaseFlag
    ();

    void resetClearBaseFlag
    ();

    bool getClearDimsFlag
    () const
    {
      return clearDimsFlag;
    }

    void setClearDimsFlag
    ();

    void resetClearDimsFlag
    ();

    int blankCount
    () const
    {
      return blankcount;
    }

  protected:

    virtual void parseStream
    (std::istream& is);

    virtual const char* className
    () const
    {
      return "ContextOp";
    }

  public:

    virtual void recogniseNode
    (IntenseLexer& lexer, Origin* origin)
    {
      recogniseNode(lexer);
    }

    virtual void recogniseNode
    (IntenseLexer& lexer);

    void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os,
     BoundManager* boundSerialiser) const;

    virtual void serialise
    (io::BaseSerialiser& baseSerialiser, std::ostream& os) const
    {
      serialise(baseSerialiser, os, NULL);
    }

    void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is,
     BoundManager* boundSerialiser);

    virtual void deserialise
    (io::BaseSerialiser& baseSerialiser, std::istream& is)
    {
      deserialise(baseSerialiser, is, NULL);
    }

  protected:

    virtual void deserialiseHelper
    (io::BaseSerialiser& baseSerialiser, std::istream& is,
     BoundManager* boundSerialiser, Origin* THIS_PARAM_IGNORED);

  };


  class IntenseLexer {

  public:

    virtual int getToken
    (Context::Token& token) = 0;

  };


}


#endif //  __CONTEXT_HPP__
