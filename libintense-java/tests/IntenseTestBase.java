// ****************************************************************************
//
// IntenseTestBase.java
//
// Copyright 2001, 2002 Paul Swoboda.
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

import java.lang.*;
import java.io.*;
import java.util.*;
import intense.*;


abstract class IntenseTestBase {

    protected static final boolean MEMLEAKTEST = false;

    protected static final boolean REENTRANTTEST = false;

    static java.io.PrintStream stdout;

    static {
	stdout = System.out;
    }

    protected static void stdout(String outputString)
    {
	System.out.println(outputString);
    }

    protected static void testOperation(Context c, ContextOp o,
					String expectedCanonical)
    {
	Context tempC = c;

	tempC.apply(o);
	if (!(tempC.canonical().equals(expectedCanonical))) {
	    stdout(
	        "ERROR: operation was incorrect!\n" +
		"c.canonical(): " + c.canonical() + "\n" +
		"o.canonical(): " + o.canonical() + "\n" +
		"expected: " + expectedCanonical
	    );
	    System.exit(1);
	}
    }

    protected static void testOperation(String c, String o,
					String expectedCanonical)
    {
	Context context = new Context();
	Context originalContext = new Context();
	ContextOp contextOp = new ContextOp();

	testParse(context, c);
	originalContext.assign(context);
	testParse(contextOp, o);
	context.apply(contextOp);
	if (context.canonical() != expectedCanonical) {
	    stdout(
	        "ERROR: operation was incorrect!\n" +
		"original: " + originalContext.canonical() + "\n" +
		"operator: " + contextOp.canonical() + "\n" +
		"result: " + context.canonical() + "\n" +
		"expected: " + expectedCanonical
	    );
	    System.exit(1);
	}
    }

    protected static void testOperation(String c, String expectedCCanonical,
					String o, String expectedOCanonical,
					String expectedCanonical,
					int expectedBasecount)
    {
	Context context = new Context();
	Context originalContext = new Context();
	ContextOp contextOp = new ContextOp();

	testParse(context, c, expectedCCanonical);
	originalContext.assign(context);
	testParse(contextOp, o, expectedOCanonical);
	context.apply(contextOp);
	if (!(context.canonical().equals(expectedCanonical))) {
	    stdout(
	        "ERROR: operation was incorrect!\n" +
		"original: " + originalContext.canonical() + "\n" +
		"operator: " + contextOp.canonical() + "\n" +
		"result: " + context.canonical() + "\n" +
		"expected: " + expectedCanonical
	    );
	    System.exit(1);
	}
	if (context.baseCount() != expectedBasecount) {
	    stdout(
	        "ERROR: Operation result basecount is wrong!\n" +
		"original: " + originalContext.canonical() + "\n" +
		"operator: " + contextOp.canonical() + "\n" +
		"result.canonical(): " + context.canonical() + "\n" +
		"basecount: " + context.baseCount() + "\n" +
		"expected: " + expectedBasecount
	    );
	    System.exit(1);
	}
    }

    protected abstract void test();

    protected void testMain()
    {
	// #if REENTRANTTEST
	// pthread_t thread1, thread2, thread3, thread4;

	// if (pthread_create(&thread1, 0, test, 0)) {
	// cerr << "Couldn't create thread!\n";
	// exit(1);
	// }
	// if (pthread_create(&thread2, 0, test, 0)) {
	// cerr << "Couldn't create thread!\n";
	// exit(1);
	// }
	// if (pthread_create(&thread3, 0, test, 0)) {
	// cerr << "Couldn't create thread!\n";
	// exit(1);
	// }
	// if (pthread_create(&thread4, 0, test, 0)) {
	// cerr << "Couldn't create thread!\n";
	// exit(1);
	// }
	// #endif

	if (MEMLEAKTEST) {
	    while (true) test();
	} else {
	    test();
	}
    }

}
