// ****************************************************************************
//
// AEPException.java : Exceptions common to AEP 2.0.
//
// Copyright 2001, 2002, 2004 Paul Swoboda.
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


package intense.aep;


import java.lang.*;
import java.util.*;
import java.io.*;
import intense.*;
import intense.log.*;


public class AEPException
  extends IntenseException {

  static final long serialVersionUID = -6107473202125293650L;

  int priority;

  public AEPException
  (String what)
  {
    super(what);
    this.priority = Log.ERROR;
  }

  public AEPException
  (String what, Exception e)
  {
    super(what, e);
    this.priority = Log.ERROR;
  }

  public AEPException
  (String what, int priority)
  {
    super(what);
    this.priority = priority;
  }

  public AEPException
  (Exception e)
  {
    super(null, e);
    this.priority = Log.ERROR;
  }

  public AEPException
  (Exception e, int priority)
  {
    super(null, e);
    this.priority = priority;
  }

  int getPriority
  ()
  {
    return priority;
  }

}
