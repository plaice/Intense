// ****************************************************************************
//
// LogException.java - Exceptions specific to intense.log.
//
// Copyright 2004 by Paul Swoboda.  All Rights Reserved.
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


package intense.log;


import intense.*;


public class LogException
  extends IntenseException {

  static final long serialVersionUID = -4162604818056148303L;

  public LogException
  (String what)
  {
    super(what);
  }

  public LogException
  (String what, Exception e)
  {
    super(what, e);
  }

}
