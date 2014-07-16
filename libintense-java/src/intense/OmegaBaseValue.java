// ****************************************************************************
//
// OmegaBaseValue.java - Maximally defined (in the intensional sense)
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
 * Maximally defined base values.
 */
public class OmegaBaseValue
  extends BaseValue {

  static final long serialVersionUID = -4779882725295919720L;

  /**
   * Void constructor.
   */
  public OmegaBaseValue
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
    return OMEGA;
  }

  /**
   * Get the canonical form string.
   *
   * @return The canonical form string.
   */
  public String canonical
  ()
  {
    return "^";
  }

  public Object clone
  ()
  {
    return new OmegaBaseValue();
  }

  public boolean equals
  (BaseValue bv)
  {
    if (bv.getType() == OMEGA) {
      return true;
    }
    return false;
  }

  public boolean refinesTo
  (BaseValue bv)
  {
    if (bv.getType() == OMEGA) {
      return true;
    }
    return false;
  }

  public boolean lessThan
  (BaseValue bv)
  {
    if (bv.getType() == OMEGA) {
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
