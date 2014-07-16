// ****************************************************************************
//
// IntenseTest4.java : A preliminary (somewhat hokey string-based) test for
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


import intense.*;


public class IntenseTest4
  extends IntenseTest {


  static class TestOrigin
    implements Origin {

    String name;

    public TestOrigin
    (String name)
    {
      this.name = name;
    }

    public String toString
    ()
    {
      return name;
    }

  }


  public static final TestOrigin theOrigin = new TestOrigin("The Origin");


  class TestParticipant
    extends AEtherParticipant {

    String name;

    StringBuffer output;

    TestParticipant
    (String name, StringBuffer output)
    {
      super();
      this.name = name;
      this.output = output;
    }

    TestParticipant
    (AEther a, String name, StringBuffer output)
      throws IntenseException
    {
      super(a);
      this.name = name;
      this.output = output;
    }

    public void applyNotify
    (ContextOp o, CompoundDimension dim, Origin origin)
    {
      if (dim != null) {
        output.append(
          name + ": " + dim + ":" + o.canonical()
        );
      } else { 
        output.append(
          name + ": " + o.canonical()
        );
      }
      if (origin == null) {
        output.append(" origin: void");
      } else {
        output.append(" origin: " + origin);
      }
      output.append('\n');
    }

    public void assignNotify
    (Context context, CompoundDimension dim, Origin origin)
    {
      if (dim != null) {
        output.append(
          name + ": " + dim + ":" + context.canonical()
        );
      } else { 
        output.append(
          name + ": " + context.canonical()
        );
      }
      if (origin == null) {
        output.append(" origin: void");
      } else {
        output.append(" origin: " + origin);
      }
      output.append('\n');
    }

    public void clearNotify
    (CompoundDimension dim, Origin origin)
    {
      if (dim != null) {
        output.append(
          name + ": clear(" + dim + ")."
        );
      } else {
        output.append(
          name + ": clear."
        );
      }
      if (origin == null) {
        output.append(" origin: void");
      } else {
        output.append(" origin: " + origin);
      }
      output.append('\n');
    }

    public void kickNotify
    (Origin origin)
    {
      output.append(
        name + ": kicked."
      );
      if (origin == null) {
        output.append(" origin: void");
      } else {
        output.append(" origin: " + origin);
      }
      output.append('\n');
    }

    public boolean isPure
    ()
    {
      return false;
    }

  }

  public IntenseTest4
  ()
  {
    super("IntenseTest4");
  }

  protected void test
  ()
    throws Exception
  {

  AEther a1;
  StringBuffer outputBuffer = new StringBuffer();
  TestParticipant p1 = new TestParticipant("p1", outputBuffer);
  TestParticipant p2 = new TestParticipant("p2", outputBuffer);
  TestParticipant p3 = new TestParticipant("p3", outputBuffer);
  ContextOp op = new ContextOp();
  Context c = new Context();
  CompoundDimension emptyDimension = new CompoundDimension();

  a1 = new AEther();
  p1.join((AEther)a1);
  p2.join((AEther)(a1).value("d2:d21"));
  p3.join((AEther)(a1).value("some:bogus:dimension"));
  a1.setBase("base");
  a1.value("d1").setBase("1");
  a1.value("d2").setBase("2");
  a1.value("d3").setBase("3");
  a1.value("d2:d21").setBase("21");
  a1.value("d2:d21:d211").setBase("211");
  a1.value("d2:d21:d212").setBase("212");
  a1.value("d2:d21:d212:d2121").setBase("2121");
  a1.value("d2:d21:d212:d2122").setBase("2122");
  a1.value("d2:d21:d213").setBase("213");
  a1.value("d2:d21:d214").setBase("214");
  ((AEther)a1.value("d2:d21:d214")).setBase(new StringBaseValue("214"), theOrigin);
  // Now let's try an internal clear or two:
  ((AEther)a1.value("d2")).clear(emptyDimension, theOrigin);
  a1.value("d3").clear();
  a1.value("d2:d21:d212").clear();
  a1.value("d2:d21").clear();
  // Now some actual operation applications:
  op.parse("[d21:d212:[a:\"1\"+b:\"2\"+c:\"3\"]]");
  a1.value("d2").apply(op);
  op.parse("[d21:d212:a:-]");
  a1.value("d2").apply(op);
  op.parse("<>");
  a1.value("d2").apply(op);
  a1.value("d3").apply(op);
  // Now some context assignments:
  c.parse("<d21:<d212:<a:<\"1\">+b:<\"2\">+c:<\"3\">>>>");
  a1.value("d2").assign(c);
  c.parse("<d21:<d212:<a:<>>>>");
  a1.value("d2").assign(c);
  c.parse("<>");
  a1.value("d2").assign(c);
  a1.value("d3").assign(c);
  // Now some equivalent direct parsing into the aether:
  a1.value("d2").parse("<d21:<d212:<a:<\"1\">+b:<\"2\">+c:<\"3\">>>>");
  a1.value("d2").parse("<d21:<d212:<a:<>>>>");
  a1.value("d2").parse("<>");
  a1.value("d3").parse("<>");
  a1.parse("<d2:d21:<newvalue+a:\"1\"+b:\"2\">+d3:<d3value>>");
  a1.value("d2").clear("d21");
  a1.value("d2").clear("d21");
  a1.parse("<d2:d21:<newvalue+a:\"1\"+b:\"2\">+d3:<d3value>>");
  a1.value("d2:d21").clearBase();
  a1.value("d2:d21").clearBase();
  a1.kick();
  if (!(outputBuffer.toString().equals(
      "p1: [\"base\"] origin: void\n" +
      "p1: d1:[\"1\"] origin: void\n" +
      "p1: d2:[\"2\"] origin: void\n" +
      "p1: d3:[\"3\"] origin: void\n" +
      "p1: d2:d21:[\"21\"] origin: void\n" +
      "p2: [\"21\"] origin: void\n" +
      "p2: d211:[\"211\"] origin: void\n" +
      "p1: d2:d21:d211:[\"211\"] origin: void\n" +
      "p2: d212:[\"212\"] origin: void\n" +
      "p1: d2:d21:d212:[\"212\"] origin: void\n" +
      "p2: d212:d2121:[\"2121\"] origin: void\n" +
      "p1: d2:d21:d212:d2121:[\"2121\"] origin: void\n" +
      "p2: d212:d2122:[\"2122\"] origin: void\n" +
      "p1: d2:d21:d212:d2122:[\"2122\"] origin: void\n" +
      "p2: d213:[\"213\"] origin: void\n" +
      "p1: d2:d21:d213:[\"213\"] origin: void\n" +
      "p2: d214:[\"214\"] origin: void\n" +
      "p1: d2:d21:d214:[\"214\"] origin: void\n" +
      "p2: d214:[\"214\"] origin: The Origin\n" +
      "p1: d2:d21:d214:[\"214\"] origin: The Origin\n" +
      "p2: clear. origin: The Origin\n" +
      "p1: clear(d2). origin: The Origin\n" +
      "p1: clear(d3). origin: void\n" +
      "p1: d2:[d21:[d212:[a:[\"1\"]+b:[\"2\"]+c:[\"3\"]]]] origin: void\n" +
      "p2: [d212:[a:[\"1\"]+b:[\"2\"]+c:[\"3\"]]] origin: void\n" +
      "p1: d2:[d21:[d212:[a:[-]]]] origin: void\n" +
      "p2: [d212:[a:[-]]] origin: void\n" +
      "p1: d2:[---] origin: void\n" +
      "p2: clear. origin: void\n" +
      "p1: d3:[---] origin: void\n" +
      "p1: d2:<d21:<d212:<a:<\"1\">+b:<\"2\">+c:<\"3\">>>> origin: void\n" +
      "p2: <d212:<a:<\"1\">+b:<\"2\">+c:<\"3\">>> origin: void\n" +
      "p2: clear. origin: void\n" +
      "p1: d2:<> origin: void\n" +
      "p1: d2:<> origin: void\n" +
      "p1: d3:<> origin: void\n" +
      "p2: <d212:<a:<\"1\">+b:<\"2\">+c:<\"3\">>> origin: void\n" +
      "p1: d2:<d21:<d212:<a:<\"1\">+b:<\"2\">+c:<\"3\">>>> origin: void\n" +
      "p2: clear. origin: void\n" +
      "p2: <> origin: void\n" +
      "p1: d2:<> origin: void\n" +
      "p1: d2:<> origin: void\n" +
      "p1: d3:<> origin: void\n" +
      "p1: clear. origin: void\n" +
      "p2: <\"newvalue\"+a:<\"1\">+b:<\"2\">> origin: void\n" +
      "p1: <d2:<d21:<\"newvalue\"+a:<\"1\">+b:<\"2\">>>+d3:<\"d3value\">> origin: void\n" +
      "p2: clear. origin: void\n" +
      "p1: clear(d2:d21). origin: void\n" +
      "p1: clear. origin: void\n" +
      "p2: <\"newvalue\"+a:<\"1\">+b:<\"2\">> origin: void\n" +
      "p1: <d2:<d21:<\"newvalue\"+a:<\"1\">+b:<\"2\">>>+d3:<\"d3value\">> origin: void\n" +
      "p1: d2:d21:[-] origin: void\n" +
      "p2: [-] origin: void\n" +
      "p1: kicked. origin: void\n" +
      "p2: kicked. origin: void\n" +
      "p3: kicked. origin: void\n"
      ))) {
    throw new IntenseTestException(
        "output string inconsistency:\n\nreceived:\n\n\"" +
        outputBuffer.toString() + "\""
    );
  }

  }

  public static void main(String[] argv)
  {
    IntenseTest4 test = new IntenseTest4();
    test.testMain(argv);
  }

}
