// ****************************************************************************
//
// BoundBaseValue.java - BaseValues with bound data.
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


public class BoundBaseValue
  extends BaseValue {

  static final long serialVersionUID = -7727701211142196087L;

  public static final int MAX_LENGTH = 1048576;

  protected Object bound;

  protected byte[] data;

  /**
   * Bound data constructor.
   *
   * @param bound The object to use as the bound data for this BoundBaseValue.
   */
  public BoundBaseValue(Object bound)
  {
    this.bound = bound;
  }

  /**
   * Void constructor.
   */
  public BoundBaseValue
  ()
  {
    bound = null;
  }

  /**
   * Outbound java.io serialization.
   *
   * @param out A stream to serialize the BoundBaseValue to.
   */
  private void writeObject
  (java.io.ObjectOutputStream out)
    throws IOException
  {
    out.writeObject(bound);
  }

  /**
   * Inbound java.io deserialization.
   *
   * @param in A stream to deserialize the BoundBaseValue from.
   */
  private void readObject
  (java.io.ObjectInputStream in)
    throws IOException, ClassNotFoundException
  {
    bound = in.readObject();
  }

  /**
   * Outbound raw serialization.
   *
   * @param out ObjectOutputStream to serialize to.
   */
  public void serialize
  (ObjectOutputStream out)
    throws IOException
  {
    writeObject(out);
  }

  /**
   * Inbound raw serialization.
   *
   * @param in ObjectInputStream to de-serialize from.
   */
  public void deserialize
  (ObjectInputStream in)
    throws IOException, ClassNotFoundException
  {
    readObject(in);
  }

  /**
   * Get the subclass type.
   *
   * @return An integer representing the subclass type.
   */
  public int getType
  ()
  {
    return BOUND;
  }

  /**
   * Get the canonical form string.
   *
   * @return The canonical form string.
   */
  public String canonical
  ()
  {
    return "";
  }

  public Object clone
  ()
  {
    return new BoundBaseValue(bound);
  }

  public boolean equals
  (BaseValue bv)
  {
    return (bv.getType() == BOUND)&&(bound == ((BoundBaseValue)bv).bound);
  }

  public boolean refinesTo
  (BaseValue bv)
  {
    return (bv.getType() == OMEGA)||
      ((bv.getType() == BOUND)&&(bound == ((BoundBaseValue)bv).bound));
  }

  public boolean lessThan
  (BaseValue bv)
  {
    return false;
  }

  public BaseValue assign
  (BaseValue bv)
    throws IntenseException
  {
    if (bv.getType() != BOUND) {
      throw new IntenseException(
        "Incompatible types in BaseValue assignment"
      );
    }
    bound = ((BoundBaseValue)bv).bound;
    return this;
  }

  public BaseValue assign
  (String s)
    throws IntenseException
  {
    bound = s;
    return this;
  }

  public byte[] data
  ()
  {
    byte[] returnValue = new byte[0];
    return returnValue;
  }

  /**
   * Outbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser - NOT valid without BoundManager for BoundBaseValue.
   */
  void serialise
  (DataOutputStream out)
    throws IOException
  {
    serialise(out, null);
  }

  /**
   * Inbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser - NOT valid without BoundManager for BoundBaseValue.
   */
  void deserialise
  (DataInputStream in)
    throws IOException
  {
    deserialise(in, null);
  }

  /**
   * Outbound serialisation, via an external BoundManager.
   */
  void serialise
  (DataOutputStream out, BoundManager boundSerialiser)
    throws IOException
  {
    if ((boundSerialiser != null)&&(bound != null)) {
      byte[] packed = boundSerialiser.pack(bound);

      if (packed != null) {
        out.writeInt(packed.length);
        out.write(packed, 0, packed.length);
      } else {
        out.writeInt(0);
      }
    } else {
      if (data != null) {
        out.writeInt(data.length);
        out.write(data, 0, data.length);
      } else {
        out.writeInt(0);
      }
    }
  }

  /**
   * Inbound serialisation, via an external BoundManager.
   */
  void deserialise
  (DataInputStream in, BoundManager boundSerialiser)
    throws IOException
  {
    int length = in.readInt();
    byte[] packed;

    if (length > MAX_LENGTH) {
      throw new IOException(
        "Attempt to deserialise BoundBaseValue content of length " + length +
        " > max allowed length " + MAX_LENGTH
      );
    }
    packed = new byte[length];
    try {
      in.readFully(packed);
    } catch (EOFException e) {
      throw new IOException("EOF reached deserialising BoundBaseValue");
    }
    bound = null;
    if (boundSerialiser != null) {
      bound = boundSerialiser.unpack(packed);
    }
    if (bound == null) {
      // We just store the data (good for middleman-type Participants, etc.):
      data = packed;
    }
  }

}
