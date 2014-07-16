// ****************************************************************************
//
// IntegerDimension.java - Integer-valued Context dimensions.
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


import java.io.*;
import java.util.*;


/**
 * Integer-valued Context dimensions.
 */
public class IntegerDimension
  extends Dimension {

  static final long serialVersionUID = -9163506516833445960L;

  private int value;

  /**
   * Void constructor.
   */
  protected IntegerDimension
  ()
  {
    this.value = 0;
  }

  /**
   * Value constructor.
   *
   * @param value The value of this IntegerDimension.
   */
  public IntegerDimension
  (int value)
  {
    this.value = value;
  }

  /**
   * Get the subclass type.
   *
   * @return An integer representing the subclass type.
   */
  public int getType
  ()
  {
    return INTEGER;
  }

  /**
   * Canonical form string accessor.
   *
   * @return The canonical form string for this Dimension.
   */
  public String canonical
  ()
  {
    return "" + value;
  }

  /**
   * Object copy.
   *
   * @return A complete copy of this Dimension.
   */
  public Object clone
  ()
  {
    return new IntegerDimension(value);
  }

  /**
   * Compare two dimensions.
   *
   * @param key1 left key object in comparison.
   * @param key2 right key object in comparison.
   * @return -1 for <, 0 for ==, and 1 for >.
   */
  public int compareTo
  (Dimension dim)
  {
    switch (dim.getType()) {
    case STRING:
      return -1;
    case INTEGER:
      if (value < ((IntegerDimension)dim).value) {
        return -1;
      } else if (value > ((IntegerDimension)dim).value) {
        return 1;
      } else {
        return 0;
      }
    default:
      // Really, we should be throwing an exception, here...
      return 0;
    }
  }

  /**
   * Outbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  public void serialise
  (DataOutputStream out)
    throws IOException
  {
    out.writeInt(value);
  }

  /**
   * Inbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  public void deserialise
  (DataInputStream in)
    throws IOException
  {
    value = in.readInt();
  }

}
