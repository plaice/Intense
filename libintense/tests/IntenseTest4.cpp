// ****************************************************************************
//
// IntenseTest4.cpp : A preliminary (somewhat hokey string-based) test for
// AEthers.
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


class TestOrigin
  : public Origin {

public:

  std::string name;

  TestOrigin
  (std::string newName)
    : Origin(), name(newName)
  {}

};


inline ostream& operator<<
(ostream& os, const TestOrigin& origin)
{
  return os << origin.name;
}


TestOrigin* theOrigin = new TestOrigin("The Origin");


class IntenseTest4
  : public IntenseTest {

protected:

  class TestParticipant
    : public AEtherParticipant {

    string name;

    ostream* os;

  public:

    TestParticipant
    (std::string _name, ostream& _os)
      : AEtherParticipant(), name(_name), os(&_os)
    {
    }

    TestParticipant
    (AEther& a, std::string _name, ostream& _os)
      : AEtherParticipant(a), name(_name), os(&_os)
    {
    }

    void applyNotify
    (const ContextOp& o, const CompoundDimension* dim, Origin* origin)
    {
      if (dim != NULL) {
        *os << name << ": " << dim->canonical() << ":" << o;
      } else { 
        *os << name << ": " << o;
      }
      if (origin == NULL) {
        *os << " origin: void";
      } else {
        *os << " origin: " << *(TestOrigin*)origin;
      }
      *os << '\n';
    }

    void assignNotify
    (const Context& newValue, const CompoundDimension* dim, Origin* origin)
    {
      if (dim != NULL) {
        *os << name << ": " << dim->canonical() << ":" << newValue;
      } else { 
        *os << name << ": " << newValue;
      }
      if (origin == NULL) {
        *os << " origin: void";
      } else {
        *os << " origin: " << *(TestOrigin*)origin;
      }
      *os << '\n';
    }

    void clearNotify
    (const CompoundDimension* dim, Origin* origin)
    {
      if (dim != NULL) {
        *os << name << ": clear(" << dim->canonical() << ").";
      } else {
        *os << name << ": clear.";
      }
      if (origin == NULL) {
        *os << " origin: void";
      } else {
        *os << " origin: " << *(TestOrigin*)origin;
      }
      *os << '\n';
    }

    void kickNotify
    (Origin* origin)
    {
      *os << name << ": kicked.";
      if (origin == NULL) {
        *os << " origin: void";
      } else {
        *os << " origin: " << *(TestOrigin*)origin;
      }
      *os << '\n';
    }

    bool isPure
    ()
    {
      return false;
    }

  };

public:

  IntenseTest4
  ()
    : IntenseTest("IntenseTest4")
  {
  }

  void test();

};


IntenseTest* test = new IntenseTest4();


void IntenseTest4::test
()
{
  AEther* a1;
  ostringstream outputBuffer;
  TestParticipant* p1 = new TestParticipant("p1", outputBuffer);
  TestParticipant* p2 = new TestParticipant("p2", outputBuffer);
  TestParticipant* p3 = new TestParticipant("p3", outputBuffer);
  ContextOp op;
  Context c;
  CompoundDimension emptyDimension;

  a1 = new AEther();
  p1->join(*a1);
  p2->join((*a1)["d2:d21"]);
  p3->join((*a1)["some:bogus:dimension"]);
  *a1 = "base";
  // First, set base values here and there, to generate opNotifications:
  (*a1)["d1"] = "1";
  (*a1)["d2"] = "2";
  (*a1)["d3"] = "3";
  (*a1)["d2:d21"] = "21";
  (*a1)["d2:d21:d211"] = "211";
  (*a1)["d2:d21:d212"] = "212";
  (*a1)["d2:d21:d212:d2121"] = "2121";
  (*a1)["d2:d21:d212:d2122"] = "2122";
  (*a1)["d2:d21:d213"] = "213";
  (*a1)["d2:d21:d214"] = "214";
  (*a1)["d2:d21:d214"].setBase(new StringBaseValue("214"), theOrigin);
  // Now let's try an internal clear or two:
  (*a1)["d2"].clear(emptyDimension, theOrigin);
  (*a1)["d3"].clear();
  (*a1)["d2:d21:d212"].clear();
  (*a1)["d2:d21"].clear();
  // Now some actual operation applications:
  op.parse("[d21:d212:[a:\"1\"+b:\"2\"+c:\"3\"]]");
  (*a1)["d2"].apply(op);
  op.parse("[d21:d212:a:-]");
  (*a1)["d2"].apply(op);
  op.parse("<>");
  (*a1)["d2"].apply(op);
  (*a1)["d3"].apply(op);
  // Now some context assignments:
  c.parse("<d21:<d212:<a:<\"1\">+b:<\"2\">+c:<\"3\">>>>");
  (*a1)["d2"].assign(c);
  c.parse("<d21:<d212:<a:<>>>>");
  (*a1)["d2"].assign(c);
  c.parse("<>");
  (*a1)["d2"].assign(c);
  (*a1)["d3"].assign(c);
  // Now some equivalent direct parsing into the aether:
  (*a1)["d2"].parse("<d21:<d212:<a:<\"1\">+b:<\"2\">+c:<\"3\">>>>");
  (*a1)["d2"].parse("<d21:<d212:<a:<>>>>");
  (*a1)["d2"].parse("<>");
  (*a1)["d3"].parse("<>");
  a1->parse("<d2:d21:<newvalue+a:\"1\"+b:\"2\">+d3:<d3value>>");
  (*a1)["d2"].clear("d21");
  (*a1)["d2"].clear("d21");
  a1->parse("<d2:d21:<newvalue+a:\"1\"+b:\"2\">+d3:<d3value>>");
  (*a1)["d2:d21"].clearBase();
  (*a1)["d2:d21"].clearBase();
  a1->kick();
  delete a1;
  if (outputBuffer.str() !=
      "p1: [\"base\"] origin: void\n"
      "p1: d1:[\"1\"] origin: void\n"
      "p1: d2:[\"2\"] origin: void\n"
      "p1: d3:[\"3\"] origin: void\n"
      "p1: d2:d21:[\"21\"] origin: void\n"
      "p2: [\"21\"] origin: void\n"
      "p2: d211:[\"211\"] origin: void\n"
      "p1: d2:d21:d211:[\"211\"] origin: void\n"
      "p2: d212:[\"212\"] origin: void\n"
      "p1: d2:d21:d212:[\"212\"] origin: void\n"
      "p2: d212:d2121:[\"2121\"] origin: void\n"
      "p1: d2:d21:d212:d2121:[\"2121\"] origin: void\n"
      "p2: d212:d2122:[\"2122\"] origin: void\n"
      "p1: d2:d21:d212:d2122:[\"2122\"] origin: void\n"
      "p2: d213:[\"213\"] origin: void\n"
      "p1: d2:d21:d213:[\"213\"] origin: void\n"
      "p2: d214:[\"214\"] origin: void\n"
      "p1: d2:d21:d214:[\"214\"] origin: void\n"
      "p2: d214:[\"214\"] origin: The Origin\n"
      "p1: d2:d21:d214:[\"214\"] origin: The Origin\n"
      "p2: clear. origin: The Origin\n"
      "p1: clear(d2). origin: The Origin\n"
      "p1: clear(d3). origin: void\n"
      "p1: d2:[d21:[d212:[a:[\"1\"]+b:[\"2\"]+c:[\"3\"]]]] origin: void\n"
      "p2: [d212:[a:[\"1\"]+b:[\"2\"]+c:[\"3\"]]] origin: void\n"
      "p1: d2:[d21:[d212:[a:[-]]]] origin: void\n"
      "p2: [d212:[a:[-]]] origin: void\n"
      "p1: d2:[---] origin: void\n"
      "p2: clear. origin: void\n"
      "p1: d3:[---] origin: void\n"
      "p1: d2:<d21:<d212:<a:<\"1\">+b:<\"2\">+c:<\"3\">>>> origin: void\n"
      "p2: <d212:<a:<\"1\">+b:<\"2\">+c:<\"3\">>> origin: void\n"
      "p2: clear. origin: void\n"
      "p1: d2:<> origin: void\n"
      "p1: d2:<> origin: void\n"
      "p1: d3:<> origin: void\n"
      "p2: <d212:<a:<\"1\">+b:<\"2\">+c:<\"3\">>> origin: void\n"
      "p1: d2:<d21:<d212:<a:<\"1\">+b:<\"2\">+c:<\"3\">>>> origin: void\n"
      "p2: clear. origin: void\n"
      "p2: <> origin: void\n"
      "p1: d2:<> origin: void\n"
      "p1: d2:<> origin: void\n"
      "p1: d3:<> origin: void\n"
      "p1: clear. origin: void\n"
      "p2: <\"newvalue\"+a:<\"1\">+b:<\"2\">> origin: void\n"
      "p1: <d2:<d21:<\"newvalue\"+a:<\"1\">+b:<\"2\">>>+d3:<\"d3value\">> origin: void\n"
      "p2: clear. origin: void\n"
      "p1: clear(d2:d21). origin: void\n"
      "p1: clear. origin: void\n"
      "p2: <\"newvalue\"+a:<\"1\">+b:<\"2\">> origin: void\n"
      "p1: <d2:<d21:<\"newvalue\"+a:<\"1\">+b:<\"2\">>>+d3:<\"d3value\">> origin: void\n"
      "p1: d2:d21:[-] origin: void\n"
      "p2: [-] origin: void\n"
      "p1: kicked. origin: void\n"
      "p2: kicked. origin: void\n"
      "p3: kicked. origin: void\n"
      ) {
    ostringstream oss;

    oss << "output string inconsistency:\n\nreceived:\n\n\""
        << outputBuffer.str() << "\"";
    throw IntenseTestException(oss.str());
  }
  delete p1;
  delete p2;
  delete p3;
}
