// ****************************************************************************
//
// IntenseTest5.java :  A test for refinement tests.
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


public class IntenseTest5
  extends IntenseTest {

  public IntenseTest5
  ()
  {
    super("IntenseTest5");
  }

  protected void test
  ()
    throws Exception
  {

  Context[] c = new Context[20];
  int i;
  int j;
  boolean[][] refinementTable1 = {
    // 0      1      2      3      4      5      6      7      8      9      10     11     12     13     14     15
    {   true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true }, // 0
    {  false,  true,  true,  true, false,  true,  true,  true,  true, false,  true, false, false, false, false,  true }, // 1
    {  false, false,  true,  true, false,  true, false,  true,  true, false,  true, false, false, false, false, false }, // 2
    {  false, false, false,  true, false, false, false, false,  true, false, false, false, false, false, false, false }, // 3
    {  false, false, false, false,  true, false, false, false, false,  true, false, false, false, false, false, false }, // 4
    {  false, false, false, false, false,  true, false, false, false, false,  true, false, false, false, false, false }, // 5
    {  false, false, false, false, false, false,  true, false, false, false, false, false, false, false, false, false }, // 6
    {  false, false, false, false, false, false, false,  true, false, false, false, false, false, false, false, false }, // 7
    {  false, false, false, false, false, false, false, false,  true, false, false, false, false, false, false, false }, // 8
    {  false, false, false, false, false, false, false, false, false,  true, false, false, false, false, false, false }, // 9
    {  false, false, false, false, false, false, false, false, false, false,  true, false, false, false, false, false }, // 10
    {  false, false, false, false, false, false,  true, false, false, false, false,  true, false,  true, false, false }, // 11
    {  false, false, false, false, false, false,  true,  true,  true,  true,  true,  true,  true,  true, false, false }, // 12
    {  false, false, false, false, false, false, false, false, false, false, false, false, false,  true, false, false }, // 13
    {  false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false,  true,  true }, // 14
    {  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true }  // 15
  };
  boolean[][] refinementTable2 = {
    // 0      1      2      3      4      5      6      7      8      9      10     11     12     13     14     15
    {   true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true }, // 0
    {  false,  true, false, false, false, false, false, false, false, false, false, false, false,  true, false, false }, // 1
    {  false, false,  true,  true,  true,  true, false,  true,  true, false,  true,  true, false,  true,  true,  true }, // 2
    {  false, false, false,  true,  true,  true, false,  true,  true, false,  true,  true, false,  true,  true,  true }, // 3
    {  false, false, false, false,  true,  true, false,  true,  true, false,  true,  true, false,  true,  true,  true }, // 4
    {  false, false, false, false, false,  true, false, false,  true, false, false, false, false,  true, false, false }, // 5
    {  false, false, false, false, false, false,  true,  true, false,  true,  true,  true, false, false, false,  true }, // 6
    {  false, false, false, false, false, false, false,  true, false, false, false, false, false, false, false,  true }, // 7
    {  false, false, false, false, false, false, false, false,  true, false, false, false, false,  true, false, false }, // 8
    {  false, false, false, false, false, false, false, false, false,  true, false, false, false, false, false,  true }, // 9
    {  false, false, false, false, false, false, false, false, false, false,  true, false, false, false, false, false }, // 10
    {  false, false, false, false, false, false, false, false, false, false, false,  true, false, false, false, false }, // 11
    {  false,  true,  true,  true,  true,  true, false,  true,  true, false,  true,  true,  true,  true,  true,  true }, // 12
    {  false, false, false, false, false, false, false, false, false, false, false, false, false,  true, false, false }, // 13
    {  false, false, false, false, false, false, false,  true, false, false,  true,  true, false, false,  true,  true }, // 14
    {  false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true }  // 15
  };

  for (i = 0; i < 20; i++) {
    c[i] = new Context();
  }

  // c[0] left vanilla
  testParse(10, c[1], "<a:<\"one\">>");
  testParse(20, c[2], "<a:<\"one\">+b:<\"two\">>");
  testParse(30, c[3], "<a:<\"one\">+b:<\"two\">+c:<\"three\">>");
  testParse(40, c[4], "<a:<\"two\">+b:<\"two\">+c:<\"three\">>");
  testParse(50, c[5], "<a:<\"one\">+b:<\"two\">+d:<\"four\">>");
  testParse(60, c[6], "<\"one\"+a:<\"one\">>");
  testParse(70, c[7], "<\"two\"+a:<\"one\">+b:<\"two\">>");
  testParse(80, c[8], "<\"three\"+a:<\"one\">+b:<\"two\">+c:<\"three\">>");
  testParse(90, c[9], "<\"four\"+a:<\"two\">+b:<\"two\">+c:<\"three\">>");
  testParse(100, c[10], "<\"five\"+a:<\"one\">+b:<\"two\">+d:<\"four\">>");
  testParse(110, c[11], "<\"one\">");
  testParse(120, c[12], "<~>");
  testParse(130, c[13], "<^>");
  testParse(140, c[14], "<a:<~>>");
  testParse(150, c[15], "<a:<^>>");

  // Just test the whole cross product; that's what the table is filled in for:
  for (i = 0; i < 15; i++) {
    for (j = 0; j < 15; j++) {
      testRefines(
        1000 + (100*i) + (15*j), c[i], c[j], refinementTable1[i][j]
      );
    }
  }

  // Now stuff all of the the above versions into a subdimension and repeat
  // with the same refinement table:
  // c[0] left vanilla
  testParse(3000, c[1], "<\"blah\"+dim1:<a:<\"one\">>>");
  testParse(3010, c[2], "<\"blah\"+dim1:<a:<\"one\">+b:<\"two\">>>");
  testParse(3020, c[3], "<\"blah\"+dim1:<a:<\"one\">+b:<\"two\">+c:<\"three\">>>");
  testParse(3030, c[4], "<\"blah\"+dim1:<a:<\"two\">+b:<\"two\">+c:<\"three\">>>");
  testParse(3040, c[5], "<\"blah\"+dim1:<a:<\"one\">+b:<\"two\">+d:<\"four\">>>");
  testParse(3050, c[6], "<\"blah\"+dim1:<\"one\"+a:<\"one\">>>");
  testParse(3060, c[7], "<\"blah\"+dim1:<\"two\"+a:<\"one\">+b:<\"two\">>>");
  testParse(3070, c[8], "<\"blah\"+dim1:<\"three\"+a:<\"one\">+b:<\"two\">+c:<\"three\">>>");
  testParse(3080, c[9], "<\"blah\"+dim1:<\"four\"+a:<\"two\">+b:<\"two\">+c:<\"three\">>>");
  testParse(3090, c[10], "<\"blah\"+dim1:<\"five\"+a:<\"one\">+b:<\"two\">+d:<\"four\">>>");
  testParse(3100, c[11], "<\"blah\"+dim1:<\"one\">>");
  testParse(3110, c[12], "<\"blah\"+dim1:<~>>");
  testParse(3120, c[13], "<\"blah\"+dim1:<^>>");
  testParse(3130, c[14], "<\"blah\"+dim1:<a:<~>>>");
  testParse(3140, c[15], "<\"blah\"+dim1:<a:<^>>>");

  for (i = 0; i < 15; i++) {
    for (j = 0; j < 15; j++) {
      testRefines(
        4000 + (100*i) + (15*j), c[i], c[j], refinementTable1[i][j]
      );
    }
  }

  // Just for kicks, stuff all of the the above into into two subdimensions
  // (duplicated) and repeat with the same refinement table:
  // c[0] left vanilla
  testParse(6000, c[1], "<\"blah\"+dim1:<a:<\"one\">>+dim2:<a:<\"one\">>>");
  testParse(6010, c[2], "<\"blah\"+dim1:<a:<\"one\">+b:<\"two\">>+dim2:<a:<\"one\">+b:<\"two\">>>");
  testParse(6020, c[3], "<\"blah\"+dim1:<a:<\"one\">+b:<\"two\">+c:<\"three\">>+dim2:<a:<\"one\">+b:<\"two\">+c:<\"three\">>>");
  testParse(6030, c[4], "<\"blah\"+dim1:<a:<\"two\">+b:<\"two\">+c:<\"three\">>+dim2:<a:<\"two\">+b:<\"two\">+c:<\"three\">>>");
  testParse(6040, c[5], "<\"blah\"+dim1:<a:<\"one\">+b:<\"two\">+d:<\"four\">>+dim2:<a:<\"one\">+b:<\"two\">+d:<\"four\">>>");
  testParse(6050, c[6], "<\"blah\"+dim1:<\"one\"+a:<\"one\">>+dim2:<\"one\"+a:<\"one\">>>");
  testParse(6060, c[7], "<\"blah\"+dim1:<\"two\"+a:<\"one\">+b:<\"two\">>+dim2:<\"two\"+a:<\"one\">+b:<\"two\">>>");
  testParse(6070, c[8], "<\"blah\"+dim1:<\"three\"+a:<\"one\">+b:<\"two\">+c:<\"three\">>+dim2:<\"three\"+a:<\"one\">+b:<\"two\">+c:<\"three\">>>");
  testParse(6080, c[9], "<\"blah\"+dim1:<\"four\"+a:<\"two\">+b:<\"two\">+c:<\"three\">>+dim2:<\"four\"+a:<\"two\">+b:<\"two\">+c:<\"three\">>>");
  testParse(6090, c[10], "<\"blah\"+dim1:<\"five\"+a:<\"one\">+b:<\"two\">+d:<\"four\">>+dim2:<\"five\"+a:<\"one\">+b:<\"two\">+d:<\"four\">>>");
  testParse(6100, c[11], "<\"blah\"+dim1:<\"one\">+dim2:<\"one\">>");
  testParse(6110, c[12], "<\"blah\"+dim1:<~>+dim2:<~>>");
  testParse(6120, c[13], "<\"blah\"+dim1:<^>+dim2:<^>>");
  testParse(6130, c[14], "<\"blah\"+dim1:<a:<~>>+dim2:<a:<~>>>");
  testParse(6140, c[15], "<\"blah\"+dim1:<a:<^>>+dim2:<a:<^>>>");

  for (i = 0; i < 15; i++) {
    for (j = 0; j < 15; j++) {
      testRefines(
        7000 + (100*i) + (15*j), c[i], c[j], refinementTable1[i][j]
      );
    }
  }

  testParse(9000, c[1], "<a:<\"one\">>");
  testParse(9010, c[2], "<a:<-1.0>>");
  testParse(9020, c[3], "<a:<0.0>>");
  testParse(9030, c[4], "<a:<1.0>>");
  testParse(9040, c[5], "<a:<2.0>>");
  testParse(9050, c[6], "<b:<0.0>>");
  testParse(9060, c[7], "<a:<1.0>+b:<2.0>>");
  testParse(9070, c[8], "<a:<3.0>>");
  testParse(9080, c[9], "<b:<100000.0>>");
  testParse(9090, c[10], "<a:<1.0>+b:<1.0>+c:<\"one thousand\">>");
  testParse(9100, c[11], "<a:<1.0>+b:<1.0>+c:<1000.0>>");
  testParse(9110, c[12], "<a:<~>>");
  testParse(9120, c[13], "<a:<^>>");
  testParse(9130, c[14], "<a:<1.0>+b:<~>>");
  testParse(9140, c[15], "<a:<1.0>+b:<^>>");

  for (i = 0; i < 15; i++) {
    for (j = 0; j < 15; j++) {
      testRefines(
        11000 + (100*i) + (15*j), c[i], c[j], refinementTable2[i][j]
      );
    }
  }

  testParse(12000, c[1], "<\"blah\"+dim1:<a:<\"one\">>>");
  testParse(12010, c[2], "<\"blah\"+dim1:<a:<-1.0>>>");
  testParse(12020, c[3], "<\"blah\"+dim1:<a:<0.0>>>");
  testParse(12030, c[4], "<\"blah\"+dim1:<a:<1.0>>>");
  testParse(12040, c[5], "<\"blah\"+dim1:<a:<2.0>>>");
  testParse(12050, c[6], "<\"blah\"+dim1:<b:<0.0>>>");
  testParse(12060, c[7], "<\"blah\"+dim1:<a:<1.0>+b:<2.0>>>");
  testParse(12070, c[8], "<\"blah\"+dim1:<a:<3.0>>>");
  testParse(12080, c[9], "<\"blah\"+dim1:<b:<100000.0>>>");
  testParse(12090, c[10], "<\"blah\"+dim1:<a:<1.0>+b:<1.0>+c:<\"one thousand\">>>");
  testParse(12100, c[11], "<\"blah\"+dim1:<a:<1.0>+b:<1.0>+c:<1000.0>>>");
  testParse(12110, c[12], "<\"blah\"+dim1:<a:<~>>>");
  testParse(12120, c[13], "<\"blah\"+dim1:<a:<^>>>");
  testParse(12130, c[14], "<\"blah\"+dim1:<a:<1.0>+b:<~>>>");
  testParse(12140, c[15], "<\"blah\"+dim1:<a:<1.0>+b:<^>>>");

  for (i = 0; i < 15; i++) {
    for (j = 0; j < 15; j++) {
      testRefines(
        14000 + (100*i) + (15*j), c[i], c[j], refinementTable2[i][j]
      );
    }
  }

  testParse(15000, c[1], "<\"blah\"+dim1:<a:<\"one\">>+dim2:<a:<\"one\">>>");
  testParse(15010, c[2], "<\"blah\"+dim1:<a:<-1.0>>+dim2:<a:<-1.0>>>");
  testParse(15020, c[3], "<\"blah\"+dim1:<a:<0.0>>+dim2:<a:<0.0>>>");
  testParse(15030, c[4], "<\"blah\"+dim1:<a:<1.0>>+dim2:<a:<1.0>>>");
  testParse(15040, c[5], "<\"blah\"+dim1:<a:<2.0>>+dim2:<a:<2.0>>>");
  testParse(15050, c[6], "<\"blah\"+dim1:<b:<0.0>>+dim2:<b:<0.0>>>");
  testParse(15060, c[7], "<\"blah\"+dim1:<a:<1.0>+b:<2.0>>+dim2:<a:<1.0>+b:<2.0>>>");
  testParse(15070, c[8], "<\"blah\"+dim1:<a:<3.0>>+dim2:<a:<3.0>>>");
  testParse(15080, c[9], "<\"blah\"+dim1:<b:<100000.0>>+dim2:<b:<100000.0>>>");
  testParse(15090, c[10], "<\"blah\"+dim1:<a:<1.0>+b:<1.0>+c:<\"one thousand\">>+dim2:<a:<1.0>+b:<1.0>+c:<\"one thousand\">>>");
  testParse(15100, c[11], "<\"blah\"+dim1:<a:<1.0>+b:<1.0>+c:<1000.0>>+dim2:<a:<1.0>+b:<1.0>+c:<1000.0>>>");
  testParse(15110, c[12], "<\"blah\"+dim1:<a:<~>>+dim2:<a:<~>>>");
  testParse(15120, c[13], "<\"blah\"+dim1:<a:<^>>+dim2:<a:<^>>>");
  testParse(15130, c[14], "<\"blah\"+dim1:<a:<1.0>+b:<~>>+dim2:<a:<1.0>+b:<~>>>");
  testParse(15140, c[15], "<\"blah\"+dim1:<a:<1.0>+b:<^>>+dim2:<a:<1.0>+b:<^>>>");

  for (i = 0; i < 15; i++) {
    for (j = 0; j < 15; j++) {
      testRefines(
        17000 + (100*i) + (15*j), c[i], c[j], refinementTable2[i][j]
      );
    }
  }

  }

  public static void main(String[] argv)
  {
    IntenseTest5 test = new IntenseTest5();
    test.testMain(argv);
  }

}
