// ****************************************************************************
//
// IntenseTest3.cpp : A basic sanity test of associative application of
// ContextOps to ContextOps.
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


class IntenseTest3
  : public IntenseTest {

public:

  IntenseTest3
  ()
    : IntenseTest("IntenseTest3")
  {
  }

  void test
  ();

};


IntenseTest* test = new IntenseTest3();


void IntenseTest3::test
()
{

  testOperation(
    10,
    // Target:
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    // Operator:
    "<d2:[d2newval+d21:<d21newval>]>",
    // Expected result, long canonical form:
    "[---+d2:[--+\"d2newval\"+d21:[--+\"d21newval\"]+d22:[--+\"d22val\"+d221:[--+\"d221val\"]]]]",
    // Expected result, short canonical form:
    "<d2:<\"d2newval\"+d21:<\"d21newval\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    // Expected result's blankcount and basecount:
    6, 4
  );

  testOperation(
    20,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[]",
    "[---+d1:[--+\"d1val\"]+d2:[--+\"d2val\"+d21:[--+\"d21val\"]+d22:[--+\"d22val\"+d221:[--+\"d221val\"]]]]",
    "<d1:<\"d1val\">+d2:<\"d2val\"+d21:<\"d21val\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    7, 5
  );

  testOperation(
    30,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[d1:[d2:[d3:[]+d4:[d5:[]]+d7:[d8:[]+d9:[]]]+d10:[]]]",
    "[---+d1:[--+\"d1val\"]+d2:[--+\"d2val\"+d21:[--+\"d21val\"]+d22:[--+\"d22val\"+d221:[--+\"d221val\"]]]]",
    "<d1:<\"d1val\">+d2:<\"d2val\"+d21:<\"d21val\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    7, 5
  );

  testOperation(
    40,
    "<>",
    "<abc+d:<e>>",
    "[--+\"abc\"+d:[--+\"e\"]]",
    "<\"abc\"+d:<\"e\">>",
    2, 2
  );

  testOperation(
    50,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "<d2:[d2newval+d21:<d21newval>]>",
    "[---+d2:[--+\"d2newval\"+d21:[--+\"d21newval\"]+d22:[--+\"d22val\"+d221:[--+\"d221val\"]]]]",
    "<d2:<\"d2newval\"+d21:<\"d21newval\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    6, 4
  );

  testOperation(
    60,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[d2:[d22:[-]]]",
    "[---+d1:[--+\"d1val\"]+d2:[--+\"d2val\"+d21:[--+\"d21val\"]+d22:[---+d221:[--+\"d221val\"]]]]",
    "<d1:<\"d1val\">+d2:<\"d2val\"+d21:<\"d21val\">+d22:<d221:<\"d221val\">>>>",
    8, 4
  );

  testOperation(
    70,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[d2:[d22:[--]]]",
    "[---+d1:[--+\"d1val\"]+d2:[--+\"d2val\"+d21:[--+\"d21val\"]+d22:[--+\"d22val\"]]]",
    "<d1:<\"d1val\">+d2:<\"d2val\"+d21:<\"d21val\">+d22:<\"d22val\">>>",
    6, 4
  );

  testOperation(
    80,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[d2:<d22:[--]>]",
    "[---+d1:[--+\"d1val\"]+d2:[---+d22:[--+\"d22val\"]]]",
    "<d1:<\"d1val\">+d2:<d22:<\"d22val\">>>",
    6, 2
  );

  testOperation(
    90,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[d2:<d22:<>>]",
    "[---+d1:[--+\"d1val\"]]",
    "<d1:<\"d1val\">>",
    3, 1
  );

  testOperation(
    100,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[d2:<d22:[-+d221:<d221newval>]>]",
    "[---+d1:[--+\"d1val\"]+d2:[---+d22:[---+d221:[--+\"d221newval\"]]]]",
    "<d1:<\"d1val\">+d2:<d22:<d221:<\"d221newval\">>>>",
    8, 2
  );

  testOperation(
    110,
    "<d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "<b0>",
    "[--+\"b0\"]",
    "<\"b0\">",
    1, 1
  );

  testOperation(
    120,
    "<d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "[b0]",
    "[--+\"b0\"+d1:[--+\"b1\"]+d2:[--+\"b2\"]+d3:[--+\"b3\"]+d4:[--+\"b4\"]+d5:[--+\"b5\"]]",
    "<\"b0\"+d1:<\"b1\">+d2:<\"b2\">+d3:<\"b3\">+d4:<\"b4\">+d5:<\"b5\">>",
    6, 6
  );

  testOperation(
    130,
    "<b0+d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "[--]",
    "[--+\"b0\"]",
    "<\"b0\">",
    1, 1
  );

  testOperation(
    140,
    "<b0+d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "[--+d2:[]]",
    "[--+\"b0\"+d2:[--+\"b2\"]]",
    "<\"b0\"+d2:<\"b2\">>",
    2, 2
  );

  testOperation(
    150,
    "<b0+d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "[--+d2:[]+d4:[]]",
    "[--+\"b0\"+d2:[--+\"b2\"]+d4:[--+\"b4\"]]",
    "<\"b0\"+d2:<\"b2\">+d4:<\"b4\">>",
    3, 3
  );

  testOperation(
    160,
    "<b0+d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "<d2:[]>",
    "[---+d2:[--+\"b2\"]]",
    "<d2:<\"b2\">>",
    3, 1
  );

  testOperation(
    170,
    "<b0+d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "<d2:[]>",
    "[---+d2:[--+\"b2\"]]",
    "<d2:<\"b2\">>",
    3, 1
  );

  testOperation(
    180,
    "<b0+d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "<d2:[]+d4:[]>",
    "[---+d2:[--+\"b2\"]+d4:[--+\"b4\"]]",
    "<d2:<\"b2\">+d4:<\"b4\">>",
    4, 2
  );

  testOperation(
    190,
    "<b0+d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "<d2:[]+d4:[]>",
    "[---+d2:[--+\"b2\"]+d4:[--+\"b4\"]]",
    "<d2:<\"b2\">+d4:<\"b4\">>",
    4, 2
  );

  // And now, some tests involving application of ContextOps to arbitrary
  // (ie vmod-like) ContextOps...
  testOperation(
    200,
    "[--+d1:<d1val+d12:[d12val+d121:<d121val>]>]",
    "<d1:[]>",
    "[---+d1:[--+\"d1val\"+d12:[\"d12val\"+d121:[--+\"d121val\"]]]]",
    "<d1:<\"d1val\"+d12:[\"d12val\"+d121:<\"d121val\">]>>",
    4, 3
  );

  // Some IntegerDimensions:
  testOperation(
    210,
    "<123456:654321+dim1:\"value1\">",
    "[--+1:[\"1val\"]+2:[\"2val\"+21:[211:[\"211val\"]+212:[\"212val\"]]]]",
    "[---+1:[--+\"1val\"]+2:[--+\"2val\"+21:[---+211:[--+\"211val\"]+212:[--+\"212val\"]]]]",
    "<1:<\"1val\">+2:<\"2val\"+21:<211:<\"211val\">+212:<\"212val\">>>>",
    8, 4
  );

  testOperation(
    220,
    "[123456:654321+dim1:\"value1\"]",
    "[--+1:[\"1val\"]+2:[\"2val\"+21:[211:[\"211val\"]+212:[\"212val\"]]]]",
    "[--+1:[\"1val\"]+2:[\"2val\"+21:[211:[\"211val\"]+212:[\"212val\"]]]]",
    "[--+1:[\"1val\"]+2:[\"2val\"+21:[211:[\"211val\"]+212:[\"212val\"]]]]",
    1, 4
  );

}
