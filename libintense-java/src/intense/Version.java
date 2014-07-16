// ****************************************************************************
//
// Version.java - A simple binder subclass of Context to attach arbitrary
// objects to a Context.
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


/**
 * A simple binder subclass of Context to attach arbitrary objects to a
 * Context.  Crude and effective.
 */
public class Version<Versioned extends Object>
  extends Context {

  static final long serialVersionUID = 7214674325405071973L;

  /**
   * The bound object.
   */
  public Versioned bound;

  /**
   * Void constructor.
   */
  public Version
  ()
  {
    super();
    bound = null;
  }

  /**
   * Bound constructor.
   */
  public Version
  (Versioned bound)
  {
    super();
    this.bound = bound;
  }

  /**
   * Constructor to initialize the bound element with a context.
   */
  public Version
  (Context context, Versioned bound)
    throws IntenseException
  {
    super(context);
    this.bound = bound;
  }

  /**
   * Constructor to initialize the bound element with a context.
   */
  public Version
  (String context, Versioned bound)
    throws IntenseException
  {
    super(context);
    this.bound = bound;
  }

}
