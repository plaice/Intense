// ****************************************************************************
//
// IntenseTest6.cpp : A preliminary test for ContextDomains.
//
// Copyright 2003 Paul Swoboda.
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


#include <iostream>
#include <sstream>
#include <stdexcept>
#include <map>
#include <set>
#include <list>
#include <string>
#include <pthread.h>
#include <intense.hpp>
#include "IntenseTest.hpp"


using namespace intense;


class IntenseTest6
  : public IntenseTest {

public:

  IntenseTest6
  ()
    : IntenseTest("IntenseTest6")
  {
  }

  void test
  ();

};


IntenseTest* test = new IntenseTest6();


void IntenseTest6::test
()
{
  SetContextDomain* d;
  Context* c;
  Context vanilla;

  d = new SetContextDomain();
  c = new Context();
  testParse(10, *c, "<a:<\"one\">>");
  d->insert(c->clone());
  testParse(20, *c, "<b:<\"two\">>");
  d->insert(c->clone());
  testParse(30, *c, "<a:<\"one\">+aa:<\"one\">+b:<\"two\">>");
  d->insert(c->clone());
  testParse(40, *c, "<a:<\"one\">+b:<\"two\">>");
  d->insert(c->clone());
  testParse(50, *c, "<a:<\"one\">+b:<\"three\">>");
  d->insert(c->clone());
  testParse(60, *c, "<a:<\"one\">+b:<\"two\">+c:<\"four\">>");
  d->insert(c->clone());

  testFit(70, *d, *c, "<a:<\"one\">+b:<\"two\">+c:<\"four\">>");
  testNearFit(80, *d, *c, "<a:<\"one\">+b:<\"two\">+c:<\"four\">>");
  testParse(90, *c, "<a:<\"one\">+b:<\"two\">+c:<\"blah\">>");
  testFit(100, *d, *c, "<a:<\"one\">+b:<\"two\">>");
  testParse(110, *c, "<a:<\"one\">+b:<\"two\">+d:<\"blah\">>");
  testFit(120, *d, *c, "<a:<\"one\">+b:<\"two\">>");

  testParse(130, *c, "<d:<\"dvalue\">>");
  d->insert(c->clone());
  testParse(140, *c, "<e:<\"evalue\">>");
  d->insert(c->clone());

  testParse(150, *c, "<d:<\"dvalue\">+e:<\"evalue\">+f:<\"fvalue\">>");
  testNearFit(160, *d, *c, "<d:<\"dvalue\">><e:<\"evalue\">>");
  testNearFit(170, *d, vanilla, "");

  delete d;
  delete c;
}
