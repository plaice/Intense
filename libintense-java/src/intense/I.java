// ****************************************************************************
//
// I : Template  for libintense-java.
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


import java.util.Iterator;


/**
 * Defines a set of Contexts, which can be used to perform intensional best
 * fits.
 */
public class I<Versioned extends Object> {

  static final long serialVersionUID = -1L;

  private SetContextDomain<Version<Versioned>> domain;

  public I()
  {
    domain = new SetContextDomain<Version<Versioned>>();
  }

  /**
   * Insert a version of an object with replacement.
   */
  public boolean insert
  (Version<Versioned> version)
  {
    return domain.insert(version, true);
  }

  /**
   * Insert a version of an object with replacement.
   */
  public boolean insert
  (String context, Versioned versioned)
    throws IntenseException
  {
    return domain.insert(new Version<Versioned>(context, versioned), true);
  }

  /**
   * Insert a version of an object with optional replacement.
   */
  boolean insert
  (Version<Versioned> version, boolean canReplace)
  {
    return domain.insert(version, canReplace);
  }

  /**
   * Remove a version of an object by its version context.
   */
  public void remove
  (Context context)
  {
    domain.remove(context);
  }

  /**
   * Remove a version of an object by its version context.
   */
  public void remove
  (String context)
    throws IntenseException
  {
    domain.remove(new Context(context));
  }

  public Versioned best
  (Context desired)
    throws IntenseException
  {
    return domain.best(desired, true).bound;
  }

  public Versioned best
  (String desired)
    throws IntenseException
  {
    return domain.best(new Context(desired), true).bound;
  }

  public Versioned best
  (Context desired, boolean canThrow)
    throws IntenseException
  {
    return domain.best(desired, canThrow).bound;
  }

  public Versioned best
  (String desired, boolean canThrow)
    throws IntenseException
  {
    return domain.best(new Context(desired), canThrow).bound;
  }

  public Versioned best
  (boolean canThrow)
    throws IntenseException
  {
    return domain.best(ContextManager.instance().current(), canThrow).bound;
  }

  public Versioned best
  ()
    throws IntenseException
  {
    return domain.best(ContextManager.instance().current(), true).bound;
  }

  public void clear
  ()
  {
    domain.clear();
  }

}
