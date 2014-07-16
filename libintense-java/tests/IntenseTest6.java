// ****************************************************************************
//
// IntenseTest6.java : A preliminary test for ContextDomains.
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


public class IntenseTest6
  extends IntenseTest {

  public IntenseTest6
  ()
  {
    super("IntenseTest6");
  }

  protected void test
  ()
    throws IntenseException
  {

  SetContextDomain d = new SetContextDomain();
  Context c = new Context();
  Context vanilla = new Context();

  testParse(10, c, "<a:<\"one\">>");
  d.insert((Context)c.clone());
  testParse(20, c, "<b:<\"two\">>");
  d.insert((Context)c.clone());
  testParse(30, c, "<a:<\"one\">+aa:<\"one\">+b:<\"two\">>");
  d.insert((Context)c.clone());
  testParse(40, c, "<a:<\"one\">+b:<\"two\">>");
  d.insert((Context)c.clone());
  testParse(50, c, "<a:<\"one\">+b:<\"three\">>");
  d.insert((Context)c.clone());
  testParse(60, c, "<a:<\"one\">+b:<\"two\">+c:<\"four\">>");
  d.insert((Context)c.clone());

  testFit(70, d, c, "<a:<\"one\">+b:<\"two\">+c:<\"four\">>");
  testNearFit(80, d, c, "<a:<\"one\">+b:<\"two\">+c:<\"four\">>");
  testParse(90, c, "<a:<\"one\">+b:<\"two\">+c:<\"blah\">>");
  testFit(100, d, c, "<a:<\"one\">+b:<\"two\">>");
  testParse(110, c, "<a:<\"one\">+b:<\"two\">+d:<\"blah\">>");
  testFit(120, d, c, "<a:<\"one\">+b:<\"two\">>");

  testParse(130, c, "<d:<\"dvalue\">>");
  d.insert((Context)c.clone());
  testParse(140, c, "<e:<\"evalue\">>");
  d.insert((Context)c.clone());

  testParse(150, c, "<d:<\"dvalue\">+e:<\"evalue\">+f:<\"fvalue\">>");
  testNearFit(160, d, c, "<d:<\"dvalue\">><e:<\"evalue\">>");
  testNearFit(170, d, vanilla, "");

  }

  public static void main(String[] argv)
  {
    IntenseTest6 test = new IntenseTest6();
    test.testMain(argv);
  }

}
