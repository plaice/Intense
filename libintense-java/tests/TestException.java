// ****************************************************************************
//
// TestException.java : Test.java-related exceptions.
//
// Copyright 2004 Paul Swoboda.
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


public class TestException
  extends Exception {

  int testId;

  public TestException
  (int testId, String what)
  {
    super(what);
    this.testId = testId;
  }

  public TestException
  (int testId, String what, Exception e)
  {
    super(what, e);
    this.testId = testId;
  }

  public TestException
  (String what)
  {
    super(what);
    testId = -1;
  }

  public TestException
  (String what, Exception e)
  {
    super(what, e);
    testId = -1;
  }

  public String getMessage
  ()
  {
    String whatString;

    if (testId >= 0) {
      whatString = "test " + testId + ": " + super.getMessage();
    } else {
      whatString = "test: " + super.getMessage();
    }
    return whatString;
  }

}
