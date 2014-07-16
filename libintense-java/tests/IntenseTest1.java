// ****************************************************************************
//
// IntenseTest1.java : A basic sanity test for parsing and copying of
// ContextOps, similar to test0, which is for Contexts.
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


import intense.*;


public class IntenseTest1
  extends IntenseTest {

  public IntenseTest1
  ()
  {
    super("IntenseTest1");
  }

  protected void test
  ()
    throws Exception
  {

  testContextOp(
    10,
    // Input string:
    "<>",
    // Expected canonical string:
    "[---]",
    // Expected short-form canonical string:
    "<>",
    // Expected blank count and base count:
    2, 0
  );

  testContextOp(
    20,
    "[--+dim:[]]",
    "[--+dim:[]]",
    "[--+dim:[]]",
    1, 0
  );

  testContextOp(
    30,
    "[dim:[]]",
    "[]",
    "[]",
    0, 0
  );

  testContextOp(
    40,
    "[---+dim:[]]",
    "[---+dim:[]]",
    "<dim:[]>",
    2, 0
  );

  testContextOp(
    50,
    "<dim:[]>",
    "[---+dim:[]]",
    "<dim:[]>",
    2, 0
  );

  testContextOp(
    60,
    "[dim1:[dim2:[dim3:[]+dim4:[dim5:[]]+dim7:[dim8:[]+dim9:[]]]+dim10:[]]]",
    "[]",
    "[]",
    0, 0
  );

  testContextOp(
    70,
    "<base>",
    "[--+\"base\"]",
    "<\"base\">",
    1, 1
  );

  // Some tests of string basevalues:
  testContextOp(
    80,
    "<\"base value\\t\\t\\n\\r\\n\\t\\tthis is a test\">",
    "[--+\"base value\\t\\t\\n\\r\\n\\t\\tthis is a test\"]",
    "<\"base value\\t\\t\\n\\r\\n\\t\\tthis is a test\">",
    1, 1
  );

  testContextOp(
    90,
    "<\"base\">",
    "[--+\"base\"]",
    "<\"base\">",
    1, 1
  );

  // A binary basevalue:
  testContextOp(
    100,
    "<#494E54454E53494F4E414C2052554C45532100>",
    "[--+#494e54454e53494f4e414c2052554c45532100]",
    "<#494e54454e53494f4e414c2052554c45532100>",
    1, 1
  );

  testContextOp(
    110,
    "<base+newbase>",
    "[--+\"newbase\"]",
    "<\"newbase\">",
    1, 1
  );

  testContextOp(
    120,
    "<base+blah+blah+blah+blah+blah+blah+blah+blah+blah+blah+blah+newbase>",
    "[--+\"newbase\"]",
    "<\"newbase\">",
    1, 1
  );

  testContextOp(
    130,
    "<base+dim1:<base2>>",
    "[--+\"base\"+dim1:[--+\"base2\"]]",
    "<\"base\"+dim1:<\"base2\">>",
    2, 2
  );

  testContextOp(
    140,
    "<dim1:<base2>+base>",
    "[--+\"base\"+dim1:[--+\"base2\"]]",
    "<\"base\"+dim1:<\"base2\">>",
    2, 2
  );

  testContextOp(
    150,
    "<base+dim1:<base2+blah+blah+blah+blah+blah+blah+newbase2>>",
    "[--+\"base\"+dim1:[--+\"newbase2\"]]",
    "<\"base\"+dim1:<\"newbase2\">>",
    2, 2
  );

  testContextOp(
    160,
    "<dim1:<base2+blah+blah+blah+blah+blah+blah+newbase2>+base>",
    "[--+\"base\"+dim1:[--+\"newbase2\"]]",
    "<\"base\"+dim1:<\"newbase2\">>",
    2, 2
  );

  testContextOp(
    170,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[---+d1:[--+\"d1val\"]+d2:[--+\"d2val\"+d21:[--+\"d21val\"]+d22:[--+\"d22val\"+d221:[--+\"d221val\"]]]]",
    "<d1:<\"d1val\">+d2:<\"d2val\"+d21:<\"d21val\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    7, 5
  );

  testContextOp(
    180,
    "<d1:<d1val>+d1:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[---+d1:[--+\"d2val\"+d21:[--+\"d21val\"]+d22:[--+\"d22val\"+d221:[--+\"d221val\"]]]]",
    "<d1:<\"d2val\"+d21:<\"d21val\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    6, 4
  );

  testContextOp(
    190,
    "<d1:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>+d1:<d1val>>",
    "[---+d1:[--+\"d1val\"+d21:[--+\"d21val\"]+d22:[--+\"d22val\"+d221:[--+\"d221val\"]]]]",
    "<d1:<\"d1val\"+d21:<\"d21val\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    6, 4
  );

  testContextOp(
    200,
    "<d1:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>+d1:<d1val>+d1:[d22:[d22newval]]>",
    "[---+d1:[--+\"d1val\"+d21:[--+\"d21val\"]+d22:[--+\"d22newval\"+d221:[--+\"d221val\"]]]]",
    "<d1:<\"d1val\"+d21:<\"d21val\">+d22:<\"d22newval\"+d221:<\"d221val\">>>>",
    6, 4
  );

  testContextOp(
    210,
    "<d1:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>+d1:<d1val>+d1:d22:d22newval>",
    "[---+d1:[--+\"d1val\"+d21:[--+\"d21val\"]+d22:[--+\"d22newval\"+d221:[--+\"d221val\"]]]]",
    "<d1:<\"d1val\"+d21:<\"d21val\">+d22:<\"d22newval\"+d221:<\"d221val\">>>>",
    6, 4
  );

  testContextOp(
    220,
    "<d1:[d1val]>",
    "[---+d1:[\"d1val\"]]",
    "<d1:[\"d1val\"]>",
    2, 1
  );

  testContextOp(
    230,
    "<d1:<d2val+d21:<d21val>+d22:[d22val+d221:<d221val>]>+d1:[d1newval]+d1:[d22:[d22newval]]>",
    "[---+d1:[--+\"d1newval\"+d21:[--+\"d21val\"]+d22:[\"d22newval\"+d221:[--+\"d221val\"]]]]",
    "<d1:<\"d1newval\"+d21:<\"d21val\">+d22:[\"d22newval\"+d221:<\"d221val\">]>>",
    5, 4
  );

  testContextOp(
    240,
    "<d1:<d2val+d21:<d21val>+d22:[d22val+d221:<d221val>]>+d1:[d1newval]+d1:d22:d22newval>",
    "[---+d1:[--+\"d1newval\"+d21:[--+\"d21val\"]+d22:[\"d22newval\"+d221:[--+\"d221val\"]]]]",
    "<d1:<\"d1newval\"+d21:<\"d21val\">+d22:[\"d22newval\"+d221:<\"d221val\">]>>",
    5, 4
  );

  // Now the same as all the above, with the non-short canonical form as the
  // input string:
  testContextOp(
    250,
    "[---]",
    "[---]",
    "<>",
    2, 0
  );

  testContextOp(
    260,
    "[--+base]",
    "[--+\"base\"]",
    "<\"base\">",
    1, 1
  );

  testContextOp(
    270,
    "[--+newbase]",
    "[--+\"newbase\"]",
    "<\"newbase\">",
    1, 1
  );

  testContextOp(
    280,
    "[--+base+dim1:[--+base2]]",
    "[--+\"base\"+dim1:[--+\"base2\"]]",
    "<\"base\"+dim1:<\"base2\">>",
    2, 2
  );

  testContextOp(
    290,
    "[---+d1:[--+d1val]+d2:[--+d2val+d21:[--+d21val]+d22:[--+d22val+d221:[--+d221val]]]]",
    "[---+d1:[--+\"d1val\"]+d2:[--+\"d2val\"+d21:[--+\"d21val\"]+d22:[--+\"d22val\"+d221:[--+\"d221val\"]]]]",
    "<d1:<\"d1val\">+d2:<\"d2val\"+d21:<\"d21val\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    7, 5
  );

  testContextOp(
    300,
    "[---+d1:[--+d2val+d21:[--+d21val]+d22:[--+d22val+d221:[--+d221val]]]]",
    "[---+d1:[--+\"d2val\"+d21:[--+\"d21val\"]+d22:[--+\"d22val\"+d221:[--+\"d221val\"]]]]",
    "<d1:<\"d2val\"+d21:<\"d21val\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    6, 4
  );

  testContextOp(
    310,
    "[---+d1:[--+d1val]]",
    "[---+d1:[--+\"d1val\"]]",
    "<d1:<\"d1val\">>",
    3, 1
  );

  testContextOp(
    320,
    "[---+d1:[--+d1val+d22:[--+d22newval]]]",
    "[---+d1:[--+\"d1val\"+d22:[--+\"d22newval\"]]]",
    "<d1:<\"d1val\"+d22:<\"d22newval\">>>",
    4, 2
  );

  // Now test some operations with strange cases involving internal
  // application of operations to operations:
  testContextOp(
    330,
    "<d1:<d1val+d12:<d12val>>+d1:[-+d12:[-]]>",
    "[---]",
    "<>",
    2, 0
  );

  testContextOp(
    340,
    "<d1:<d1val+d12:<d12val>>+d1:<-+d12:[-]>>",
    "[---]",
    "<>",
    2, 0
  );

  testContextOp(
    350,
    "<d1:<d1val+d12:<d12val>>+d1:[-+d12:[d12newval]]>",
    "[---+d1:[---+d12:[--+\"d12newval\"]]]",
    "<d1:<d12:<\"d12newval\">>>",
    5, 1
  );

  testContextOp(
    360,
    "[--+d1:<d1val+d12:[d12val+d121:<d121val>]>]",
    "[--+d1:[--+\"d1val\"+d12:[\"d12val\"+d121:[--+\"d121val\"]]]]",
    "[--+d1:<\"d1val\"+d12:[\"d12val\"+d121:<\"d121val\">]>]",
    3, 3
  );

  // Some IntegerDimensions:
  testContextOp(
    370,
    "[--+1:\"1val\"+2:21:211:\"211val\"+2:21:212:\"212val\"+2:\"2val\"]",
    "[--+1:[\"1val\"]+2:[\"2val\"+21:[211:[\"211val\"]+212:[\"212val\"]]]]",
    "[--+1:[\"1val\"]+2:[\"2val\"+21:[211:[\"211val\"]+212:[\"212val\"]]]]",
    1, 4
  );

  }

  public static void main(String[] argv)
  {
    IntenseTest1 test = new IntenseTest1();
    test.testMain(argv);
  }

}
