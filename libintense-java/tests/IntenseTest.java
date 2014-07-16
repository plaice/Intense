// ****************************************************************************
//
// IntenseTest.java : Base for tests of libintense.
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
import java.util.*;
import java.util.regex.*;


public abstract class IntenseTest
  extends Test {

  protected static void testParse
  (int i, Context c, String canonical, String expectedCanonical)
    throws IntenseException
  {
    try {
      c.parse(canonical);
    } catch (Exception e) {
      throw new IntenseTestException(
        i,
        "ERROR: Context or ContextOp parse failed!\n" + "input: " + canonical +
        "\nerror: " + e.getMessage(),
        e
      );
    }
    if (c.canonical().compareTo(expectedCanonical) != 0) {
      throw new IntenseTestException(
        i,
        "ERROR: Canonical form was incorrect for parse!" +
        "\nc.canonical(): " + c.canonical() + ' ' + StringUtil.byteArrayToHexString(c.canonical().getBytes(), " ") +
        "\nexpected:      " + expectedCanonical + ' ' + StringUtil.byteArrayToHexString(expectedCanonical.getBytes(), " ")
      );
    }
  }

  protected static void testParse
  (int i, Context c, String canonical)
    throws IntenseException
  {
    testParse(i, c, canonical, canonical);
  }

  protected static void testContext
  (int i, String input, String expectedCanonical, int expectedBasecount)
    throws IntenseException
  {
    Context c = new Context();

    testParse(i, c, input, expectedCanonical);
    if (c.baseCount() != expectedBasecount) {
      throw new IntenseTestException(
        i,
        "ERROR: Parsed Context basecount is wrong!\ninput: " + input +
        "\nc.canonical(): " + c.canonical() + "\nbasecount: " + c.baseCount() +
        "\nexpected:      " + expectedBasecount
      );
    }
  }

  protected static void testContextOp
  (int i, String input, String expectedCanonical,
   String expectedShortCanonical, int expectedBlankcount,
   int expectedBasecount)
    throws IntenseException
  {
    ContextOp c = new ContextOp(), c2 = new ContextOp();

    try {
      c.parse(input);
    } catch (Exception e) {
      throw new IntenseTestException(
        i,
        "ERROR: Context or ContextOp parse failed!\ninput: " + input
      );
    }
    c2.apply(c);
    if (c2.canonical().compareTo(expectedCanonical) != 0) {
      throw new IntenseTestException(
        i,
        "ERROR: Canonical form was incorrect for " +
        "parse after application to []!\ninput: " + input +
        "\nc.canonical():  " + c.canonical() +
        "\nc2.canonical(): " + c2.canonical() +
        "\nexpected:       " + expectedCanonical
      );
    } else if (c2.shortCanonical().compareTo(expectedShortCanonical) != 0) {
      throw new IntenseTestException(
        i,
        "ERROR: Short canonical form was incorrect for " +
        "parse after application to []!\ninput: " + input +
        "\nc.shortCanonical():  " + c.shortCanonical() +
        "\nc2.shortCanonical(): " + c2.shortCanonical() +
        "\nexpected:            " + expectedShortCanonical
      );
    }
    if (c2.blankCount() != expectedBlankcount) {
      throw new IntenseTestException(
        i,
        "ERROR: Applied ContextOp blankcount is wrong!\nintput: " +
        input + "\nc.canonical(): " + c.canonical() + "\nc2.canonical(): " +
        c2.canonical() + "\nblankcount: " + c2.blankCount() +
        "\nexpected: " + expectedBlankcount
      );
    }
    if (c2.baseCount() != expectedBasecount) {
      throw new IntenseTestException(
        i,
        "ERROR: Applied ContextOp basecount is wrong!\ninput: " +
        input + "\nbasecount: " + c2.baseCount() + "\nc.canonical(): " +
        c.canonical() + "\nc2.canonical(): " + c2.canonical() +
        "\nexpected: " + expectedBasecount
      );
    }
  }

  void testOperation
  (int i, String c, String o, String expectedCanonical, int expectedBasecount)
    throws IntenseException
  {
    Context context = new Context(), context2 = new Context();
    ContextOp contextOp = new ContextOp();

    try {
      context.parse(c);
    } catch (Exception e) {
      throw new IntenseTestException(
        i,
        "ERROR: Context parse failed!\ninput: " + c
      );
    }
    context2.assign(context);
    if (!context.equals(context2)) {
      System.err.println("Comparison is " + context.compare(context2));
      throw new IntenseTestException(
        i,
        "ERROR: Context assignment was incorrect!\ncontext input: " + c +
        "\ncontext:  " + context.canonical() +
        "\nresult:   " + context2.canonical() +
        "\nexpected: " + c
      );
    }
    try {
      contextOp.parse(o);
    } catch (Exception e) {
      throw new IntenseTestException(
        i,
        "ERROR: ContextOp parse failed!\ninput: " + o
      );
    }
    context2.apply(contextOp);
    if (context2.canonical().compareTo(expectedCanonical) != 0) {
      throw new IntenseTestException(
        i,
        "ERROR: Operation was incorrect!\ncontext: " + c +
        "\noperator input:     " + o +
        "\noperator canonical: " + contextOp.canonical() +
        "\nresult:   " + context2.canonical() +
        "\nexpected: " + expectedCanonical
      );
    }
    if (context2.baseCount() != expectedBasecount) {
      throw new IntenseTestException(
        i,
        "ERROR: Operation result basecount was incorrect!\n" +
        "context: " + c + "\noperator input: " + o + "\noperator canonical: " +
        contextOp.canonical() + "\nresult basecount: " +
        context2.baseCount() + "\nexpected: " + expectedBasecount
      );
    }
  }

  protected void testOperation
  (int i, String o1, String o2, String expectedCanonical,
   String expectedShortCanonical, int expectedBlankcount,
   int expectedBasecount)
    throws IntenseException
  {
    ContextOp op1 = new ContextOp(), op2 = new ContextOp();

    //    cerr + "Parsing op: " + o1 + "...\n";
    try {
      op1.parse(o1);
    } catch (Exception e) {
      throw new IntenseTestException(
        i,
        "ERROR: ContextOp parse failed!\ninput: " + o1
      );
    }
    try {
      op2.parse(o2);
    } catch (Exception e) {
      throw new IntenseTestException(
        i,
        "ERROR: ContextOp parse failed!\ninput: " + o2
      );
    }
    op1.apply(op2);
    if (op1.canonical().compareTo(expectedCanonical) != 0) {
      throw new IntenseTestException(
        i,
        "ERROR: Operation canonical form was incorrect!\nop1: " +
        o1 + "\nop2: " + o2 +
        "\nresult:   " + op1.canonical() +
        "\nexpected: " + expectedCanonical
      );
    }
    if (op1.shortCanonical().compareTo(expectedShortCanonical) != 0) {
      throw new IntenseTestException(
        i,
        "ERROR: Operation short canonical form was incorrect!\n" +
        "op1: " + o1 + "\nop2: " + o2 +
        "\nresult:   " + op1.shortCanonical() +
        "\nexpected: " + expectedShortCanonical
      );
    }
    if (op1.blankCount() != expectedBlankcount) {
      throw new IntenseTestException(
        i,
        "ERROR: Operation result blankcount was incorrect!\nop1: " +
        o1 + "\nop2: " + o2 + "\nresult blankcount: " + op1.blankCount() +
        "\nexpected: " + expectedBlankcount
      );
    }
    if (op1.baseCount() != expectedBasecount) {
      throw new IntenseTestException(
        i,
        "ERROR: Operation result basecount was incorrect!\nop1: " +
        o1 + "\nop2: " + o2 + "\nresult basecount: " + op1.baseCount() + "\n" +
        "expected: " + expectedBasecount
      );
    }
  }

  protected void testRefines
  (int i, Context c1, Context c2, boolean expected)
    throws IntenseException
  {
    if ((c1.refinesTo(c2)) != expected) {
      throw new IntenseTestException(
        i,
        "ERROR: Refinement test was incorrect!\nc1.canonical(): " +
        c1.canonical() + "\nc2.canonical(): " + c2.canonical() +
        "\nexpected: " + expected
      );
    }
  }

  protected void testFit
  (int i, ContextDomain domain, Context requested,
   String expectedCanonical /* null if no best fit expected */)
    throws IntenseException
  {
    Context fit;

    try {
      fit = domain.best(requested);
    } catch (IntenseException e) {
      throw new IntenseTestException(
        i,
        "ERROR: IntenseException in domain.best(): " + e.getMessage(),
        e
      );
    }
    if (fit != null) {
      if (expectedCanonical == null) {
        throw new IntenseTestException(
          i,
          "ERROR: Best fit for " + requested.canonical() + ":\n\t" +
          fit.canonical() + ".\n\tExected none!"
        );
      } else if (fit.canonical().compareTo(expectedCanonical) != 0) {
        throw new IntenseTestException(
          i,
          "ERROR: Best fit for " + requested.canonical() + ":\n\t" +
          fit.canonical() + ".\nExected:\n\t " + expectedCanonical
        );
      }
    } else if (expectedCanonical != null) {
      throw new IntenseTestException(
        i,
        "ERROR: No best fit for " + requested.canonical() +
        ".\nExpected:\n\t" + expectedCanonical
      );
    }
  }

  protected void testNearFit
  (int i, ContextDomain domain, Context requested, String expectedCanonicals)
    throws IntenseException
  {
    StringBuffer canonicals = new StringBuffer();
    List nearFit;

    try {
      nearFit = domain.near(requested);
    } catch (IntenseException e) {
      throw new IntenseTestException(
        i,
        "ERROR: IntenseException in domain.near(): " + e.getMessage(),
        e
      );
    }
    if (nearFit.size() > 0) {
      Iterator itr = nearFit.iterator();

      while (itr.hasNext()) {
        // Append canonical form of next expected near-fit context:
        canonicals.append(((Context)itr.next()).canonical());
      }
    }
    if (expectedCanonicals.compareTo(canonicals.toString()) != 0) {
      throw new IntenseTestException(
        i,
        "ERROR: Near fit for " + requested.canonical() + ":\n\t" +
        canonicals.toString() + ".\nExected:\n\t" + expectedCanonicals
      );
    }
  }

  public IntenseTest
  (String execName)
  {
    super(execName);
  }

  protected void usage
  ()
  {
    System.err.println(
      "\nusage:\n\n" + execName + " [-t<n threads>] " +
      "[-m (infinite loop mem leak test)]\n" +
      "\t[-h (help - you're looking at it...)]\n"
    );
    System.exit(1);
  }

}
