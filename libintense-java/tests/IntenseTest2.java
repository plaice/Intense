// ****************************************************************************
//
// IntenseTest2.java : A basic sanity test for applying ContextOps to Contexts.
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


public class IntenseTest2
  extends IntenseTest {

  public IntenseTest2
  ()
  {
    super("IntenseTest2");
  }

  protected void test
  ()
    throws Exception
  {

  testOperation(
    10,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "<d2:[d2newval+d21:<d21newval>]>",
    "<d2:<\"d2newval\"+d21:<\"d21newval\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    4
  );

  testOperation(
    20,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[]",
    "<d1:<\"d1val\">+d2:<\"d2val\"+d21:<\"d21val\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    5
  );

  testOperation(
    30,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[d1:[d2:[d3:[]+d4:[d5:[]]+d7:[d8:[]+d9:[]]]+d10:[]]]",
    "<d1:<\"d1val\">+d2:<\"d2val\"+d21:<\"d21val\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    5
  );

  testOperation(
    40,
    "<>",
    "<abc+d:<e>>",
    "<\"abc\"+d:<\"e\">>",
    2
  );

  testOperation(
    50,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "<d2:[d2newval+d21:<d21newval>]>",
    "<d2:<\"d2newval\"+d21:<\"d21newval\">+d22:<\"d22val\"+d221:<\"d221val\">>>>",
    4
  );

  testOperation(
    60,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[d2:[d22:[-]]]",
    "<d1:<\"d1val\">+d2:<\"d2val\"+d21:<\"d21val\">+d22:<d221:<\"d221val\">>>>",
    4
  );

  testOperation(
    70,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[d2:[d22:[--]]]",
    "<d1:<\"d1val\">+d2:<\"d2val\"+d21:<\"d21val\">+d22:<\"d22val\">>>",
    4
  );

  testOperation(
    80,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[d2:<d22:[--]>]",
    "<d1:<\"d1val\">+d2:<d22:<\"d22val\">>>",
    2
  );

  testOperation(
    90,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[d2:<d22:<>>]",
    "<d1:<\"d1val\">>",
    1
  );

  testOperation(
    100,
    "<d1:<d1val>+d2:<d2val+d21:<d21val>+d22:<d22val+d221:<d221val>>>>",
    "[d2:<d22:[-+d221:<d221newval>]>]",
    "<d1:<\"d1val\">+d2:<d22:<d221:<\"d221newval\">>>>",
    2
  );

  testOperation(
    110,
    "<d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "<b0>",
    "<\"b0\">",
    1
  );

  testOperation(
    120,
    "<d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "[b0]",
    "<\"b0\"+d1:<\"b1\">+d2:<\"b2\">+d3:<\"b3\">+d4:<\"b4\">+d5:<\"b5\">>",
    6
  );

  testOperation(
    130,
    "<b0+d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "[--]",
    "<\"b0\">",
    1
  );

  testOperation(
    140,
    "<b0+d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "[--+d2:[]]",
    "<\"b0\"+d2:<\"b2\">>",
    2
  );

  testOperation(
    150,
    "<b0+d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "[--+d2:[]+d4:[]]",
    "<\"b0\"+d2:<\"b2\">+d4:<\"b4\">>",
    3
  );

  testOperation(
    160,
    "<b0+d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "<d2:[]>",
    "<d2:<\"b2\">>",
    1
  );

  testOperation(
    170,
    "<b0+d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "<d2:[]>",
    "<d2:<\"b2\">>",
    1
  );

  testOperation(
    180,
    "<b0+d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "<d2:[]+d4:[]>",
    "<d2:<\"b2\">+d4:<\"b4\">>",
    2
  );

  testOperation(
    190,
    "<b0+d1:<b1>+d2:<b2>+d3:<b3>+d4:<b4>+d5:<b5>>",
    "<d2:[]+d4:[]>",
    "<d2:<\"b2\">+d4:<\"b4\">>",
    2
  );

  // Some IntegerDimensions:
  testOperation(
    200,
    "<123456:654321+dim1:\"value1\">",
    "[--+1:[\"1val\"]+2:[\"2val\"+21:[211:[\"211val\"]+212:[\"212val\"]]]]",
    "<1:<\"1val\">+2:<\"2val\"+21:<211:<\"211val\">+212:<\"212val\">>>>",
    4
  );

  }

  public static void main(String[] argv)
  {
    IntenseTest2 test = new IntenseTest2();
    test.testMain(argv);
  }

}
