// ****************************************************************************
//
// IntenseTest0.cpp : A basic sanity test for parsing and copying of Contexts.
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
#include <stdexcept>
#include <sstream>
#include <map>
#include <set>
#include <list>
#include <string>
#include <pthread.h>
#include <intense.hpp>
#include "IntenseTest.hpp"


using namespace intense;


class IntenseTest0
  : public IntenseTest {

public:

  IntenseTest0
  ()
    : IntenseTest("IntenseTest0")
  {
  }

  void test
  ();

};


IntenseTest* test = new IntenseTest0();


void IntenseTest0::test
()
{

  testContext(
    10,
    "<>",
    "<>",
    0
  );

  testContext(
    20,
    "<dim1:<dim2:<dim3:<>+dim4:<dim5:<>>+dim7:<dim8:<>+dim9:<>>>+dim10:<>>>",
    "<>",
    0
  );

  testContext(
    30,
    "<base>",
    "<\"base\">",
    1
  );

  testContext(
    40,
    "<base+newbase>",
    "<\"newbase\">",
    1
  );

  testContext(
    50,
    "<base+blah+blah+blah+blah+blah+blah+blah+blah+blah+blah+blah+newbase>",
    "<\"newbase\">",
    1
  );

  testContext(
    60,
    "<base+dim1:<base2>>",
    "<\"base\"+dim1:<\"base2\">>",
    2
  );

  testContext(
    70,
    "<dim1:<base2>+base>",
    "<\"base\"+dim1:<\"base2\">>",
    2
  );

  testContext(
    80,
    "<base+dim1:<base2+blah+blah+blah+blah+blah+blah+newbase2>>",
    "<\"base\"+dim1:<\"newbase2\">>",
    2
  );

  testContext(
    90,
    "<dim1:<base2+blah+blah+blah+blah+blah+blah+newbase2>+base>",
    "<\"base\"+dim1:<\"newbase2\">>",
    2
  );

  testContext(
    100,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "<d1:<\"d1val\">+d2:<\"d2val\"+d21:<\"d21val\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    5
  );

  testContext(
    110,
    "<d1:<d1val>+d1:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "<d1:<\"d2val\"+d21:<\"d21val\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    4
  );

  // I really don't like this vmod-like merging during parsing.  I suppose
  // it's not that bad, but it makes for headaches...
  testContext(
    120,
    "<d1:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>+d1:<d1val>>",
    "<d1:<\"d1val\"+d21:<\"d21val\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    4
  );

  testContext(
    130,
    "<d1:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>+d1:<d1val>+d1:<d22:<d22newval>>>",
    "<d1:<\"d1val\"+d21:<\"d21val\">+d22:<\"d22newval\"+d221:<\"d221val\">>>>",
    4
  );

  testContext(
    140,
    "<d1:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>+d1:<d1val>+d1:d22:d22newval>",
    "<d1:<\"d1val\"+d21:<\"d21val\">+d22:<\"d22newval\"+d221:<\"d221val\">>>>",
    4
  );

  // Some AlphaBaseValue and OmegaBaseValue uses.
  testContext(
    150,
    "<d1:~+d2:d21:d211:~+d2:d21:d212:^+d2:^>",
    "<d1:<~>+d2:<^+d21:<d211:<~>+d212:<^>>>>",
    4
  );

  // Some NumberBaseValue uses - dependent on ostream output precision for
  // long doubles; might fail on some systems.
  testContext(
    160,
    "<d1:123456789+d2:d21:d211:-123.456e-08+d2:d21:d212:\"123\"+d2:123.456E+10>",
    "<d1:<1.23457e+08>+d2:<1.23456e+12+d21:<d211:<-1.23456e-06>+d212:<\"123\">>>>",
    4
  );

  // Some IntegerDimensions:
  testContext(
    170,
    "<1:\"1val\"+2:21:211:\"211val\"+2:21:212:\"212val\"+2:\"2val\">",
    "<1:<\"1val\">+2:<\"2val\"+21:<211:<\"211val\">+212:<\"212val\">>>>",
    4
  );

  testContext(
    180,
    "<1:\"1val\"+2:-21:211:\"211val\"+2:21:212:\"212val\"+2:\"2val\">",
    "<1:<\"1val\">+2:<\"2val\"+-21:<211:<\"211val\">>+21:<212:<\"212val\">>>>",
    4
  );

  // Some quoted dimensions and UTF-8 dimensions and base values:
  testContext(
    190,
    "<\"1\":\"1val\"+2:\"2val\">",
    "<2:<\"2val\">+\"1\":<\"1val\">>",
    2
  );

  // Try to create an integer dimension with UTF-8 (hopefully unsuccessfully):
  testContext(
    200,
    "<\"\\U31\\U0032\\U000033\\U00000030\":\"1val\"+2:\"2val\">",
    "<2:<\"2val\">+\"1230\":<\"1val\">>",
    2
  );

  testContext(
    210,
    "<\"  blah  \":\"1val\"+2:\"2val\">",
    "<2:<\"2val\">+\"  blah  \":<\"1val\">>",
    2
  );

  testContext(
    220,
    "<1:\"  blah  \":\"1val\"+2:\"2val\">",
    "<1:<\"  blah  \":<\"1val\">>+2:<\"2val\">>",
    2
  );

  testContext(
    230,
    "<1:\"  blah  \":11:\"1val\"+2:\"2val\">",
    "<1:<\"  blah  \":<11:<\"1val\">>>+2:<\"2val\">>",
    2
  );

  testContext(
    250,
    // UTF-8 encoded dimension:
    "<\"\\U91\\U92\\U0091\\U0092\\U000091\\U000092\\U00000091\\U00000092\":\"1val\"+2:\"2val\">",
    "<2:<\"2val\">+\302\221\302\222\302\221\302\222\302\221\302\222\302\221\302\222:<\"1val\">>",
    2
  );

  testContext(
    260,
    // UTF-8 encoded dimension with non-UTF-8 padding:
    "<\"abc123_\\U91\\U92\\U0091\\U0092\\U000091\\U000092\\U00000091\\U00000092_def456\":\"1val\"+2:\"2val\">",
    "<2:<\"2val\">+abc123_\302\221\302\222\302\221\302\222\302\221\302\222\302\221\302\222_def456:<\"1val\">>",
    2
  );

  testContext(
    270,
    // UTF-8 encoded dimension with non-UTF-8 padding with whitespace:
    "<\"abc 123 \\U91\\U92\\U0091\\U0092\\U000091\\U000092\\U00000091\\U00000092 def 456\":\"1val\"+2:\"2val\">",
    "<2:<\"2val\">+\"abc 123 \302\221\302\222\302\221\302\222\302\221\302\222\302\221\302\222 def 456\":<\"1val\">>",
    2
  );

  testContext(
    280,
    // UTF-8 encoded basevalue:
    "<dim1:\"\\U91\\U92\\U0091\\U0092\\U000091\\U000092\\U00000091\\U00000092\">>",
    "<dim1:<\"\302\221\302\222\302\221\302\222\302\221\302\222\302\221\302\222\">>",
    1
  );

  testContext(
    290,
    // UTF-8 encoded basevalue with non-UTF-8 padding:
    "<dim1:\"abc123_\\U91\\U92\\U0091\\U0092\\U000091\\U000092\\U00000091\\U00000092_def456\">>",
    "<dim1:<\"abc123_\302\221\302\222\302\221\302\222\302\221\302\222\302\221\302\222_def456\">>",
    1
  );

  testContext(
    300,
    // UTF-8 encoded basevalue with non-UTF-8 padding with whitespace:
    "<dim1:\"abc 123 \\U91\\U92\\U0091\\U0092\\U000091\\U000092\\U00000091\\U00000092 def 456\">>",
    "<dim1:<\"abc 123 \302\221\302\222\302\221\302\222\302\221\302\222\302\221\302\222 def 456\">>",
    1
  );

  testContext(
    310,
    // UTF-8 encoded reserved characters, in base values:
    "<d1:<\"_\\U07\">+d2:<\"_\\U08\">+d3:<\"_\\U09\">+d4:<\"_\\U0a\">+d5:<\"_\\U0b\">+d6:<\"_\\U0c\">+d7:<\"_\\U0d\">>",
    "<d1:<\"_\\a\">+d2:<\"_\\b\">+d3:<\"_\\t\">+d4:<\"_\\n\">+d5:<\"_\\v\">+d6:<\"_\\f\">+d7:<\"_\\r\">>",
    7
  );

  testContext(
    320,
    // More UTF-8 encoded reserved characters, in base values:
    "<da:<\"_\\U5c\">+db:<\"_\\U22\">+dc:<\"_\\U23\">+dd:<\"_\\U20\">+de:<\"_\\U7e\">+df:<\"_\\U5e\">+dg:<\"_\\U2d\">+dh:<\"_\\U3c\">+di:<\"_\\U3e\">+dj:<\"_\\U5b\">+dk:<\"_\\U5d\">+dl:<\"_\\U2b\">+dm:<\"_\\U3a\">>",
    "<da:<\"_\\\\\">+db:<\"_\\\"\">+dc:<\"_#\">+dd:<\"_ \">+de:<\"_~\">+df:<\"_^\">+dg:<\"_-\">+dh:<\"_<\">+di:<\"_>\">+dj:<\"_[\">+dk:<\"_]\">+dl:<\"_+\">+dm:<\"_:\">>",
    13
  );

  testContext(
    330,
    // UTF-8 encoded reserved characters, in dimensions:
    "<\"d1\\U07\":<1>+\"d2\\U08\":<2>+\"d3\\U09\":<3>+\"d4\\U0a\":<4>+\"d5\\U0b\":<5>+\"d6\\U0c\":<6>+\"d7\\U0d\":<7>>>",
    "<\"d1\\a\":<1>+\"d2\\b\":<2>+\"d3\\t\":<3>+\"d4\\n\":<4>+\"d5\\v\":<5>+\"d6\\f\":<6>+\"d7\\r\":<7>>",
    7
  );

  testContext(
    340,
    // More UTF-8 encoded reserved characters, in dimensions:
    "<\"da\\U5c\":<1>+\"db\\U22\":<2>+\"dc\\U23\":<3>+\"dd\\U20\":<4>+\"de\\U7e\":<5>+\"df\\U5e\":<6>+\"dg\\U2d\":<7>+\"dh\\U3c\":<8>+\"di\\U3e\":<9>+\"dj\\U5b\":<10>+\"dk\\U5d\":<11>+\"dl\\U2b\":<12>+\"dm\\U3a\":<13>>",
    "<\"da\\\\\":<1>+\"db\\\"\":<2>+\"dc#\":<3>+\"dd \":<4>+\"de~\":<5>+\"df^\":<6>+\"dg-\":<7>+\"dh<\":<8>+\"di>\":<9>+\"dj[\":<10>+\"dk]\":<11>+\"dl+\":<12>+\"dm:\":<13>>",
    13
  );

}
