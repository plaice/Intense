// ****************************************************************************
//
// NumberBaseValue.java - BaseValues with numeric values.
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


// A basic string-valued BaseValue subclass:
public class NumberBaseValue
  extends BaseValue {

  static final long serialVersionUID = 5495015024095678789L;

  private double number;

  /**
   * Copy constructor.
   *
   * @param bv The BaseValue from which to take the numberic value of this
   * NumberBaseValue.
   */
  public NumberBaseValue
  (BaseValue bv)
    throws IntenseException
  {
    if (bv.getType() == NUMBER) {
      number = ((NumberBaseValue)bv).number;
    } else {
      throw new IntenseException(
        "Attempt to construct NumberBaseValue from BaseValue of type " +
        getTypeString()
      );
    }
  }

  /**
   * Initialization constructor.
   *
   * @param number The initial value of this NumberBaseValue.
   */
  public NumberBaseValue
  (double number)
  {
    this.number = number;
  }

  /**
   * Void constructor.
   */
  public NumberBaseValue
  ()
  {
    number = 0;
  }

  /**
   * Get the subclass type.
   *
   * @return An integer representing the subclass type.
   */
  public int getType
  ()
  {
    return NUMBER;
  }

  /**
   * Get the canonical form string.
   *
   * @return The canonical form string.
   */
  public String canonical
  ()
  {
    return Double.toString(number);
  }

  public Object clone
  ()
  {
    return new NumberBaseValue(number);
  }

  public boolean equals
  (BaseValue bv)
  {
    return (bv.getType() == NUMBER)&&(number == ((NumberBaseValue)bv).number);
  }

  public boolean refinesTo
  (BaseValue bv)
  {
    return (bv.getType() == OMEGA)||
      ((bv.getType() == NUMBER)&&(number <= ((NumberBaseValue)bv).number));
  }

  public boolean lessThan
  (BaseValue bv)
  {
    return (bv.getType() == NUMBER)&&(number < ((NumberBaseValue)bv).number);
  }

  public BaseValue assign
  (BaseValue bv)
    throws IntenseException
  {
    if (bv.getType() != NUMBER) {
      throw new IntenseException(
        "Incompatible types in BaseValue assignment"
      );
    }
    number = ((NumberBaseValue)bv).number;
    return this;
  }

  public BaseValue assign
  (String s)
    throws IntenseException
  {
    try {
    } catch (NumberFormatException e) {
      throw new IntenseException(
        "Attempt to assign non-double-convertable string \"" +
        s + "\" as value to NumberBaseValue"
      );
    }
    return this;
  }

  /**
   * Outbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  void serialise
  (DataOutputStream out)
    throws IOException
  {
    out.writeDouble(number);
  }

  /**
   * Inbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  void deserialise
  (DataInputStream in)
    throws IOException
  {
    number = in.readDouble();
  }

}
