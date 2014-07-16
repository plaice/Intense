// ****************************************************************************
//
// Dimension.java - Generalised Context dimensions.
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
 * Abstract base class for Context dimensions.
 */
public abstract class Dimension
  implements Serializable {

  // Various types for subclasses:
  public static final int VOID = 0;
  public static final int STRING = 1;
  public static final int INTEGER = 2;

  /**
   * An offset map of integer subclass types to strings.
   */
  public static final String[] typeMap = {
    "VOID", "STRING", "INTEGER"
  };

  /**
   * Get the subclass type.
   *
   * @return An integer representing the subclass type.
   */
  public abstract int getType
  ();

  /**
   * Get the subclass type as a string.
   *
   * @return A string description of the Dimension type.
   */
  public String getTypeString
  ()
    throws IntenseException
  {
    int type = getType();

    if ((type >= typeMap.length)||(type < 0)) {
      throw new IntenseException("Invalid Dimension type " + type);
    } else {
      return typeMap[type];
    }
  }

  /**
   * Parametrized factory for use in deserialisation.
   */
  static Dimension factory
  (int subclassType)
    throws IOException
  {
    switch (subclassType) {
    case VOID:
      return null;
    case STRING:
      return new StringDimension();
    case INTEGER:
      return new IntegerDimension();
    default:
      throw new IOException("Bad Dimension type " + subclassType);
    }
  }

  /**
   * Canonical form string accessor.
   *
   * @return The canonical form string for this Dimension.
   */
  public abstract String canonical
  ();

  /**
   * Canonical form string accessor.
   *
   * @return The canonical form string for this Dimension.
   */
  public String toString
  ()
  {
    return canonical();
  }

  /**
   * Object copy.
   *
   * @return A complete copy of this Dimension.
   */
  public abstract Object clone
  ();

  /**
   * Compare two dimensions.
   *
   * @param key1 left key object in comparison.
   * @param key2 right key object in comparison.
   * @return -1 for <, 0 for ==, and 1 for >.
   */
  public abstract int compareTo
  (Dimension dim);

  /**
   * Test the equality of two dimensions.
   *
   * @param key1 left key object in comparison.
   * @param key2 right key object in comparison.
   * @return true if the dimension are equal, false otherwise.
   */
  public boolean equals
  (Dimension dim)
  {
    return compareTo(dim) == 0;
  }

  /**
   * Outbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  public abstract void serialise
  (DataOutputStream out)
    throws IOException;

  /**
   * Inbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  public abstract void deserialise
  (DataInputStream in)
    throws IOException;

  /**
   * Outbound factory-based serialisation, compatable with the C++
   * libintense-serial XDRBaseSerialiser.
   */
  public static void factorySerialise
  (Dimension dimension, DataOutputStream out)
    throws IOException
  {
    int type;

    if (dimension != null) {
      type = dimension.getType();
    } else {
      type = Dimension.VOID;
    }
    out.writeInt(type);
    if (dimension != null) {
      dimension.serialise(out);
    }
  }

  /**
   * Inbound factory-based deserialisation, compatable with the C++
   * libintense-serial XDRBaseSerialiser.
   */
  public static Dimension factoryDeserialise
  (DataInputStream in)
    throws IOException
  {
    int type = in.readInt();
    Dimension dimension = Dimension.factory(type);
    if (dimension != null) {
      dimension.deserialise(in);
    }
    return dimension;
  }

}
