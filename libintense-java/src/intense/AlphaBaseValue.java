// ****************************************************************************
//
// AlphaBaseValue.java - Minimally defined (in the intensional sense)
// BaseValues.
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
 * Minimally defined base values.
 */
public class AlphaBaseValue
  extends BaseValue {

  static final long serialVersionUID = 8417931511236799373L;

  /**
   * Void constructor.
   */
  public AlphaBaseValue
  ()
  {
    super();
  }

  /**
   * Get the subclass type.
   *
   * @return An integer representing the subclass type.
   */
  public int getType
  ()
  {
    return ALPHA;
  }

  /**
   * Get the canonical form string.
   *
   * @return The canonical form string.
   */
  public String canonical
  ()
  {
    return "~";
  }

  public Object clone
  ()
  {
    return new AlphaBaseValue();
  }

  public boolean equals
  (BaseValue bv)
  {
    if (bv.getType() != ALPHA) {
      return false;
    }
    return true;
  }

  public boolean refinesTo
  (BaseValue bv)
  {
    return true;
  }

  public boolean lessThan
  (BaseValue bv)
  {
    if (bv.getType() == ALPHA) {
      return false;
    }
    return true;
  }

  public BaseValue assign
  (BaseValue bv)
    throws IntenseException
  {
    return null;
  }

  public BaseValue assign
  (String s)
    throws IntenseException
  {
    return null;
  }

  /**
   * Outbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  void serialise
  (DataOutputStream out)
    throws IOException
  {
  }

  /**
   * Inbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  void deserialise
  (DataInputStream in)
    throws IOException
  {
  }

}
