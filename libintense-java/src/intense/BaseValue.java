// ****************************************************************************
//
// BaseValue.java - Base values of intensional contexts.
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
 * Abstract base class for version base values.
 */
public abstract class BaseValue
 implements Serializable {

  // Various types for subclasses:
  public static final int VOID = 0;
  public static final int ALPHA = 1;
  public static final int OMEGA = 2;
  public static final int NUMBER = 3;
  public static final int STRING = 4;
  public static final int BINARY = 5;
  public static final int BOUND = 6;

  /**
   * An offset map of integer subclass types to strings.
   */
  public static final String[] typeMap = {
    "VOID", "ALPHA", "OMEGA", "NUMBER", "STRING", "BINARY", "BOUND"
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
   * @return A string description of the BaseValue type.
   */
  public String getTypeString
  ()
    throws IntenseException
  {
    int type = getType();

    if ((type >= typeMap.length)||(type < 0)) {
      throw new IntenseException("Invalid BaseValue type " + type);
    } else {
      return typeMap[type];
    }
  }

  /**
   * Parametrized factory for use in deserialisation.
   */
  static BaseValue factory
  (int subclassType)
    throws IOException
  {
    switch (subclassType) {
    case VOID:
      return null;
    case ALPHA:
      return new AlphaBaseValue();
    case OMEGA:
      return new OmegaBaseValue();
    case NUMBER:
      return new NumberBaseValue();
    case STRING:
      return new StringBaseValue();
    case BINARY:
      return new BinaryBaseValue();
    case BOUND:
      return new BoundBaseValue();
    default:
      throw new IOException("Bad BaseValue type " + subclassType);
    }
  }

  /**
   * Canonical form string accessor.
   *
   * @return The canonical form string for this BaseValue.
   */
  public abstract String canonical
  ();

  /**
   * Canonical form string accessor.
   *
   * @return The canonical form string for this BaseValue.
   */
  public String toString
  ()
  {
    return canonical();
  }

  /**
   * Object copy.
   *
   * @return A complete copy of this BaseValue.
   */
  public abstract Object clone
  ();

  /**
   * Test the equality of two BaseValues.
   *
   * @param bv An operand BaseValue.
   * @return A boolean comparison value.
   */
  public abstract boolean equals
  (BaseValue bv);

  /**
   * Test the intensional refinement (partial order relation) between two
   * BaseValues.
   *
   * @param bv An operand BaseValue.
   * @return A boolean comparison value.
   */
  public abstract boolean refinesTo
  (BaseValue bv);

  /**
   * Test the lexicographic comparison (total order relation) between two
   * BaseValues.
   *
   * @param bv An operand BaseValue.
   * @return A boolean comparison value.
   */
  public abstract boolean lessThan
  (BaseValue bv);

  /**
   * Assign the value of another BaseValue to this BaseValue.
   *
   * @param bv An operand BaseValue.
   * @return A reference to this BaseValue, after the assignment.
   */
  public abstract BaseValue assign
  (BaseValue bv)
    throws IntenseException;

  /**
   * Outbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  abstract void serialise
  (DataOutputStream out)
    throws IOException;

  /**
   * Inbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  abstract void deserialise
  (DataInputStream in)
    throws IOException;

  /**
   * Outbound factory-based serialisation, compatable with the C++
   * libintense-serial XDRBaseSerialiser.
   */
  static void factorySerialise
  (BaseValue baseValue, DataOutputStream out, BoundManager boundManager)
    throws IOException
  {
    if (baseValue != null) {
      out.writeInt(baseValue.getType());
    } else {
      out.writeInt(BaseValue.VOID);
    }
    if (baseValue != null) {
      if (baseValue.getType() == BaseValue.BOUND) {
        ((BoundBaseValue)baseValue).serialise(out, boundManager);
      } else {
        baseValue.serialise(out);
      }
    }
  }

  /**
   * Inbound factory-based deserialisation, compatable with the C++
   * libintense-serial XDRBaseSerialiser.
   */
  static BaseValue factoryDeserialise
  (DataInputStream in, BoundManager boundManager)
    throws IOException
  {
    int type;
    BaseValue base;

    type = in.readInt();
    base = factory(type);
    if (base != null) {
      if (base.getType() == BaseValue.BOUND) {
        ((BoundBaseValue)base).deserialise(in, boundManager);
      } else {
        base.deserialise(in);
      }
    }
    return base;
  }

}
