// ****************************************************************************
//
// ContextManager.java - A Singleton current-context manager, intended for use
// with ContextDomains.
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



package intense;


import java.util.*;
import java.io.*;


/**
 * Defines a global, singleton context manager, intended for use with
 * ContextDomains for purposes of runtime intensional configuration management.
 */
public class ContextManager {

  private static ContextManager instancePtr = new ContextManager();

  private Context context;

  private ContextManager
  ()
  {
    context = new Context();
  }

  /**
   * Singleton accessor.
   */
  public static ContextManager instance
  ()
  {
    return instancePtr;
  }

  // Current context accessor:
  public Context current
  ()
  {
    return context;
  }

}
