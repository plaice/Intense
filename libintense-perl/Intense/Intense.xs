// ****************************************************************************
//
// Intense.xs - Intense XS module
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

#ifdef __cplusplus
extern "C" {
#endif

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#ifdef __cplusplus
}
#endif


#undef context
#undef list
#undef do_open
#undef do_close
#undef ref
#undef apply


using namespace std;


#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <intense.hpp>


using namespace intense;


class XSContext
  : public Context
{

public:

  int refCount;

  XSContext& root
  ();

protected:

  XSContext
  (const Context* _parent, const DimensionRef* parentDimension);

  virtual Context* valueRefFactory
  (const DimensionRef& dim)
  {
    return new XSContext(this, &dim);
  }

  virtual void checkedClear
  (ContextMap::iterator& itr);

  virtual void clear
  ();

public:

  XSContext
  ();

  ~XSContext
  ();

  virtual char* className
  ()
  {
    return "Context";
  }

  void refCountDecrement
  ();

  void refCountIncrement
  ();

};


class XSContextOp
  : public ContextOp
{

public:

  int refCount;

  XSContextOp& root
  ();

protected:

  XSContextOp
  (const Context* parent, const DimensionRef* parentDimension);

  virtual Context* valueRefFactory
  (const DimensionRef& dim)
  {
    return new XSContextOp(this, &dim);
  }

  virtual void checkedClear
  (ContextMap::iterator& itr);

  virtual void clear
  ();

public:

  XSContextOp
  ();

  ~XSContextOp
  ();

  virtual char* className
  ()
  {
    return "ContextOp";
  }

  void refCountDecrement
  ();

  void refCountIncrement
  ();

};


class XSContextBinder
  : public Context
{

public:

  SV* bound;

  int refCount;

  XSContextBinder
  (SV* bound);

  ~XSContextBinder
  ();

  virtual char* className
  ()
  {
    return "ContextBinder";
  }

  void refCountDecrement
  ()
  {
    refCount--;
  }

  void refCountIncrement
  ()
  {
    refCount++;
  }

};


void XSContext::checkedClear
(ContextMap::iterator& itr)
{
  if (((XSContext*)(itr->second))->refCount > 0) {
    itr++->second->clear();
  } else {
    delete itr->second;
    erase(itr);
  }
}


void XSContext::clear
()
{
  ContextMap::iterator itr = begin(), tempItr;

  if (basecount == 0) {
    return;
  }
  if (base) {
    delete base;
  }
  base = NULL;
  propagateBasecount(-basecount);
  basecount = 0;
  while (itr != end()) {
    if (((XSContext*)(itr->second))->refCount > 0) {
      itr++->second->clear();
    } else {
      tempItr = itr;
      tempItr++;
      delete itr->second;
      erase(itr);
      itr = tempItr;
    }
  }
}


XSContext::XSContext
(const Context* parent, const DimensionRef* parentDimension)
  : Context(parent, parentDimension), refCount(0)
{
}


XSContext::XSContext
()
  : Context(), refCount(1)
{
}


XSContext::~XSContext
()
{
}


void XSContext::refCountDecrement
()
{
  XSContext* marker;

  for (marker = this; marker != NULL;
    marker = (XSContext*)(marker->parent)) {
    marker->refCount--;
  }
}


void XSContext::refCountIncrement
()
{
  XSContext* marker;

  for (marker = this; marker != NULL;
    marker = (XSContext*)(marker->parent)) {
    marker->refCount++;
  }
}


XSContext& XSContext::root
()
{
  XSContext* marker = this;

  while (marker->parent != NULL) {
    marker = (XSContext*)(marker->parent);
  }
  return *marker;
}


void XSContextOp::checkedClear
(ContextMap::iterator& itr)
{
  if (((XSContextOp*)(itr->second))->refCount > 0) {
    itr++->second->clear();
  } else {
    delete itr->second;
    erase(itr);
  }
}


void XSContextOp::clear
()
{
  ContextMap::iterator itr = begin(), tempItr;

  if (basecount == 0) {
    return;
  }
  if (base) {
    delete base;
  }
  base = NULL;
  propagateBasecount(-basecount);
  basecount = 0;
  propagateBlankcount(-blankcount);
  blankcount = 0;
  while (itr != end()) {
    if (((XSContextOp*)(itr->second))->refCount > 0) {
      itr++->second->clear();
    } else {
      tempItr = itr;
      tempItr++;
      delete itr->second;
      erase(itr);
      itr = tempItr;
    }
  }
}


XSContextOp::XSContextOp
(const Context* parent, const DimensionRef* parentDimension)
  : ContextOp(parent, parentDimension), refCount(0)
{
}


XSContextOp::XSContextOp
()
  : ContextOp(), refCount(1)
{
}


XSContextOp::~XSContextOp
()
{
}


void XSContextOp::refCountDecrement
()
{
  XSContextOp* marker;

  for (marker = this; marker != NULL;
    marker = (XSContextOp*)(marker->parent)) {
    marker->refCount--;
  }
}


void XSContextOp::refCountIncrement
()
{
  XSContextOp* marker;

  for (marker = this; marker != NULL;
    marker = (XSContextOp*)(marker->parent)) {
    marker->refCount++;
  }
}


XSContextOp& XSContextOp::root
()
{
  XSContextOp* marker = this;

  while (marker->parent != NULL) {
    marker = (XSContextOp*)(marker->parent);
  }
  return *marker;
}


XSContextBinder::XSContextBinder
(SV* bound)
  : Context(), bound(bound), refCount(1)
{
  SvREFCNT_inc(bound);
}


XSContextBinder::~XSContextBinder
()
{
  SvREFCNT_dec(bound);
}


MODULE = Intense		PACKAGE = Context


XSContext*
_voidConstructor()
CODE:
  RETVAL = new XSContext();
  ST(0) = sv_newmortal();
  sv_setref_pv( ST(0), "Context", (void*)RETVAL );
  XSRETURN(1);


XSContext*
_copyConstructor(XSContext* src)
CODE:
  RETVAL = new XSContext();
  ST(0) = sv_newmortal();
  sv_setref_pv( ST(0), "Context", (void*)RETVAL );
  RETVAL->assign(*src);
  XSRETURN(1);


XSContext*
_parseConstructor(const char* canonical)
CODE:
  RETVAL = new XSContext();
  ST(0) = sv_newmortal();
  sv_setref_pv( ST(0), "Context", (void*)RETVAL );
  try {
    RETVAL->parse(canonical);
  } catch (IntenseException& e) {
    croak("Intense::Context parse failure: %s", e.what());
  }
  XSRETURN(1);


void
XSContext::DESTROY()
CODE:
  int refCount = THIS->root().refCount;

  if (refCount > 1) {
    THIS->refCountDecrement();
  } else if (refCount == 1) {
    delete &(THIS->root());
  } else {
    cerr << "Bad refCount " << refCount << " in XSContext::DESTROY!\n";
    exit(1);
  }


void
XSContext::parse(char* canonicalForm)
CODE:
  try {
    THIS->parse(canonicalForm);
  } catch (IntenseException& e) {
    croak("Intense::Context parse failure: %s", e.what());
  }


char*
XSContext::canonical()
CODE:
  char* retval = strdup((char*)(THIS->canonical().data()));
  RETVAL = retval;
OUTPUT:
  RETVAL
CLEANUP:
  free(retval);


char*
XSContext::shortCanonical()
CODE:
  char* retval = strdup((char*)(THIS->shortCanonical().data()));
  RETVAL = retval;
OUTPUT:
  RETVAL
CLEANUP:
  free(retval);


XSContext*
XSContext::value(char* dimension)
CODE:
  XSContext* context;

  context = (XSContext*)&((*THIS)[dimension]);
  context->refCountIncrement();
  ST(0) = sv_newmortal();
  sv_setref_pv(ST(0), "Context", context);
  XSRETURN(1);


bool
XSContext::hasBase()
CODE:
  RETVAL = THIS->Context::getBase() != NULL;
OUTPUT:
  RETVAL


int
XSContext::baseCount()
CODE:
  RETVAL = THIS->baseCount();
OUTPUT:
  RETVAL


char*
XSContext::getBase()
CODE:
  if (THIS->Context::getBase() == NULL) {
    ST(0) = &PL_sv_undef;
    XSRETURN(1);
  } else {
    RETVAL = strdup((char*)(THIS->Context::getBase()->canonical().data()));
  }
OUTPUT:
  RETVAL


char*
XSContext::setBase(char* value)
CODE:
  if (THIS->Context::getBase() == NULL) {
    ST(0) = &PL_sv_undef;
    RETVAL = NULL;
  } else {
    RETVAL = strdup((char*)(THIS->Context::getBase()->canonical().data()));
  }
  THIS->Context::operator=(value);
  if (RETVAL == NULL) {
    XSRETURN(1);
  }
OUTPUT:
  RETVAL


char*
XSContext::clearBase()
CODE:
  if (THIS->Context::getBase() == NULL) {
    ST(0) = &PL_sv_undef;
    RETVAL = NULL;
  } else {
    RETVAL = strdup((char*)(THIS->Context::getBase()->canonical().data()));
  }
  THIS->clearBase();
  if (RETVAL == NULL) {
    XSRETURN(1);
  }
OUTPUT:
  RETVAL


XSContext*
XSContext::_applyContextOp(XSContextOp* op)
CODE:
  THIS->apply(*op);
  THIS->refCountIncrement();
  ST(0) = sv_newmortal();
  sv_setref_pv(ST(0), "Context", THIS);
  XSRETURN(1);


XSContext*
XSContext::_assignContextOp(XSContext* c)
CODE:
  THIS->assign(*c);
  THIS->refCountIncrement();
  ST(0) = sv_newmortal();
  sv_setref_pv(ST(0), "Context", THIS);
  XSRETURN(1);


MODULE = Intense		PACKAGE = ContextOp


XSContextOp*
_voidConstructor()
CODE:
  RETVAL = new XSContextOp();
  ST(0) = sv_newmortal();
  sv_setref_pv( ST(0), "ContextOp", (void*)RETVAL );
  XSRETURN(1);


XSContextOp*
_copyConstructor(XSContextOp* src)
CODE:
  RETVAL = new XSContextOp();
  ST(0) = sv_newmortal();
  sv_setref_pv( ST(0), "ContextOp", (void*)RETVAL );
  RETVAL->assign(*src);
  XSRETURN(1);


XSContextOp*
_parseConstructor(const char* canonical)
CODE:
  RETVAL = new XSContextOp();
  ST(0) = sv_newmortal();
  sv_setref_pv( ST(0), "ContextOp", (void*)RETVAL );
  try {
    RETVAL->parse(canonical);
  } catch (IntenseException& e) {
    croak("Intense::ContextOp parse failure: %s", e.what());
  }
  XSRETURN(1);
 

void
XSContextOp::DESTROY()
CODE:
  int& refCount = THIS->root().refCount;

  if (refCount > 1) {
    THIS->refCountDecrement();
  } else if (refCount == 1) {
    delete &(THIS->root());
  } else {
    cerr << "Bad refCount " << refCount << " in XSContextOp::DESTROY!\n";
    exit(1);
  }


void
XSContextOp::parse(char* canonicalForm)
CODE:
  try {
    THIS->parse(canonicalForm);
  } catch (IntenseException& e) {
    croak("Intense::ContextOp parse failure: %s", e.what());
  }


char*
XSContextOp::canonical()
CODE:
  char* retval = strdup((char*)(THIS->canonical().data()));
  RETVAL = retval;
OUTPUT:
  RETVAL
CLEANUP:
  free(retval);


char*
XSContextOp::shortCanonical()
CODE:
  char* retval = strdup((char*)(THIS->shortCanonical().data()));
  RETVAL = retval;
OUTPUT:
  RETVAL
CLEANUP:
  free(retval);


SV*
XSContextOp::value(char* dimension)
CODE:
  XSContextOp* context;

  context = (XSContextOp*)&((*THIS)[dimension]);
  context->refCountIncrement();
  ST(0) = sv_newmortal();
  sv_setref_pv(ST(0), "ContextOp", context);
  XSRETURN(1);


bool
XSContextOp::hasBase()
CODE:
  RETVAL = THIS->Context::getBase() != NULL;
OUTPUT:
  RETVAL


int
XSContextOp::baseCount()
CODE:
  RETVAL = THIS->baseCount();
OUTPUT:
  RETVAL


int
XSContextOp::blankCount()
CODE:
  RETVAL = THIS->blankCount();
OUTPUT:
  RETVAL


char*
XSContextOp::getBase()
CODE:
  if (THIS->Context::Context::getBase() == NULL) {
    ST(0) = &PL_sv_undef;
    XSRETURN(1);
  } else {
    RETVAL = strdup((char*)(THIS->Context::getBase()->toString().data()));
  }
OUTPUT:
  RETVAL


char*
XSContextOp::setBase(char* value)
CODE:
  if (THIS->Context::getBase() == NULL) {
    ST(0) = &PL_sv_undef;
    RETVAL = NULL;
  } else {
    RETVAL = strdup((char*)(THIS->Context::getBase()->canonical().data()));
  }
  THIS->ContextOp::operator=(value);
  if (RETVAL == NULL) {
    XSRETURN(1);
  }
OUTPUT:
  RETVAL


char*
XSContextOp::clearBase()
CODE:
  if (THIS->Context::getBase() == NULL) {
    ST(0) = &PL_sv_undef;
    RETVAL = NULL;
  } else {
    RETVAL = strdup((char*)(THIS->Context::getBase()->canonical().data()));
  }
  THIS->clearBase();
  if (RETVAL == NULL) {
    XSRETURN(1);
  }
OUTPUT:
  RETVAL


int
XSContextOp::getClearBaseFlag()
CODE:
  RETVAL = THIS->getClearBaseFlag();
OUTPUT:
  RETVAL


void
XSContextOp::setClearBaseFlag()
CODE:
  THIS->setClearBaseFlag();


void
XSContextOp::resetClearBaseFlag()
CODE:
  THIS->resetClearBaseFlag();


int
XSContextOp::getClearDimsFlag()
CODE:
  RETVAL = THIS->getClearDimsFlag();
OUTPUT:
  RETVAL


void
XSContextOp::setClearDimsFlag()
CODE:
  THIS->setClearDimsFlag();


void
XSContextOp::resetClearDimsFlag()
CODE:
  THIS->resetClearDimsFlag();


XSContextOp*
XSContextOp::_applyContext(XSContext* c)
CODE:
  THIS->apply(*((XSContext*)c));
  THIS->refCountIncrement();
  ST(0) = sv_newmortal();
  sv_setref_pv(ST(0), "ContextOp", THIS);
  XSRETURN(1);


XSContextOp*
XSContextOp::_applyContextOp(XSContextOp* c)
CODE:
  THIS->apply(*((XSContextOp*)c));
  THIS->refCountIncrement();
  ST(0) = sv_newmortal();
  sv_setref_pv(ST(0), "ContextOp", THIS);
  XSRETURN(1);


XSContextOp*
XSContextOp::_assignContextOp(XSContextOp* op)
CODE:
  THIS->assign(*op);
  THIS->refCountIncrement();
  ST(0) = sv_newmortal();
  sv_setref_pv(ST(0), "ContextOp", THIS);
  XSRETURN(1);


MODULE = Intense		PACKAGE = ContextBinder


XSContextBinder*
_boundConstructor(SV* bound)
CODE:
  RETVAL = new XSContextBinder(bound);
  ST(0) = sv_newmortal();
  sv_setref_pv( ST(0), "ContextBinder", (void*)RETVAL );
  XSRETURN(1);


void
XSContextBinder::DESTROY()
CODE:
  int refCount = THIS->refCount;

  if (refCount > 1) {
    THIS->refCountDecrement();
  } else if (refCount == 1) {
    delete THIS;
  } else {
    cerr << "Bad refCount " << refCount
         << " in XSContextBinder::DESTROY!\n";
    exit(1);
  }


SV*
XSContextBinder::bound()
CODE:
  SvREFCNT_inc(THIS->bound);
  RETVAL = THIS->bound;
OUTPUT:
  RETVAL


void
XSContextBinder::parse(char* canonicalForm)
CODE:
  try {
    THIS->parse(canonicalForm);
  } catch (IntenseException& e) {
    croak("Intense::ContextBinder parse failure: %s", e.what());
  }


char*
XSContextBinder::canonical()
CODE:
  char* retval = strdup((char*)(THIS->canonical().data()));
  RETVAL = retval;
OUTPUT:
  RETVAL
CLEANUP:
  free(retval);


char*
XSContextBinder::shortCanonical()
CODE:
  char* retval = strdup((char*)(THIS->shortCanonical().data()));
  RETVAL = retval;
OUTPUT:
  RETVAL
CLEANUP:
  free(retval);


XSContextBinder*
XSContextBinder::assign(XSContext* src)
CODE:
  THIS->Context::operator=(*((Context*)src));
  THIS->refCountIncrement();
  ST(0) = sv_newmortal();
  sv_setref_pv(ST(0), "ContextBinder", THIS);
  XSRETURN(1);


XSContextBinder*
XSContextBinder::apply(XSContextOp* op)
CODE:
  THIS->apply(*op);
  THIS->refCountIncrement();
  ST(0) = sv_newmortal();
  sv_setref_pv(ST(0), "ContextBinder", THIS);
  XSRETURN(1);


MODULE = Intense		PACKAGE = ContextDomain


SetContextDomain*
SetContextDomain::new()


void
SetContextDomain::DESTROY()
CODE:
  ContextSet::iterator itr;

  for (itr = THIS->begin(); itr != THIS->end(); itr++) {
    if (((XSContextBinder*)(*itr))->refCount > 1) {
      ((XSContextBinder*)(*itr))->refCountDecrement();
    } else if (((XSContextBinder*)(*itr))->refCount == 1) {
      delete *itr;
    } else {
      cerr << "Bad binder refCount " << ((XSContextBinder*)(*itr))->refCount
           << " in SetContextDomain::DESTROY!\n";
      exit(1);
    }
  }
  THIS->clear();
  delete THIS;


bool
SetContextDomain::insertBinder(XSContextBinder* context)
CODE:
  bool returnValue = false;
  ContextSet::iterator itr;

  if ((itr = THIS->find(context)) != THIS->end()) {
    returnValue = true;
    if (((XSContextBinder*)(*itr))->refCount > 1) {
      ((XSContextBinder*)(*itr))->refCountDecrement();
    } else if (((XSContextBinder*)(*itr))->refCount == 1) {
      delete *itr;
    } else {
      cerr << "Bad binder refCount " << ((XSContextBinder*)(*itr))->refCount
           << " in SetContextDomain::insert!\n";
      exit(1);
    }
    THIS->erase(itr);
  }
  THIS->insert(context, true);
  context->refCountIncrement();
  RETVAL = returnValue;
OUTPUT:
  RETVAL


XSContext*
SetContextDomain::bestBinder(XSContext* context)
CODE:
  XSContextBinder* result = (XSContextBinder*)THIS->best(*context, false);
  if (result != NULL) {
    result->refCountIncrement();
    ST(0) = sv_newmortal();
    sv_setref_pv(ST(0), "ContextBinder", result);
    XSRETURN(1);
  } else {
    ST(0) = &PL_sv_undef;
    XSRETURN(1);
  }


void
SetContextDomain::nearBinders(XSContext* context)
PPCODE:
  list<Context*> resultList = THIS->near(*context);
  EXTEND(SP, resultList.size());
  for (list<Context*>::iterator itr = resultList.begin();
     itr != resultList.end(); itr++) {
    XSContextBinder* binderPtr = (XSContextBinder*)*itr;
    SV* binderSV;
    binderPtr->refCountIncrement();
    binderSV = sv_newmortal();
    sv_setref_pv(binderSV, "ContextBinder", binderPtr);
    PUSHs(binderSV);
  }

