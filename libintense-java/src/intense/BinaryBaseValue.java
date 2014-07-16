// ****************************************************************************
//
// BinaryBaseValue.java - BaseValues with binary strings.
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
 * A basic binary-string-valued BaseValue subclass.
 */
public class BinaryBaseValue
  extends BaseValue {

  static final long serialVersionUID = -3431992282128068667L;

  public static final int MAX_LENGTH = 1048576;

  /**
   * The Binary value of this BaseValue.
   */
  protected byte[] content;

  /**
   * Canonical form constructor.
   *
   * @param bv The BaseValue from which to take the canonical form for the
   * Binary value of this BinaryBaseValue.
   */
  public BinaryBaseValue
  (BaseValue bv)
  {
    if (bv.getType() == BINARY) {
      byte[] bvContent = ((BinaryBaseValue)bv).content;
      content = new byte[bvContent.length];
      for (int i = 0; i < bvContent.length; i++) {
        content[i] = bvContent[i];
      }
    } else {
      content = bv.canonical().getBytes();
    }
  }

  /**
   * Initialization constructor.
   *
   * @param s The initial Binary value of this BinaryBaseValue.
   */
  public BinaryBaseValue
  (String s)
  {
    content = s.getBytes();
  }

  /**
   * Construction with empty but pre-allocated content.
   *
   * @param initialSize Initial size to allocate for a content buffer in
   * bytes.
   */
  public BinaryBaseValue
  (int initialSize)
  {
    content = new byte[initialSize];
  }

  /**
   * Void constructor.
   */
  public BinaryBaseValue
  ()
  {
    content = new byte[0];
  }

  /**
   * Get the subclass type.
   *
   * @return An integer representing the subclass type.
   */
  public int getType
  ()
  {
    return BINARY;
  }

  /**
   * Get the canonical form string.
   *
   * @return The canonical form string.
   */
  public String canonical
  ()
  {
    StringBuffer buffer = new StringBuffer();

    buffer.append("#");
    for (int i = 0; i < content.length; i++) {
      buffer.append(nybbleToChar((content[i]&0xf0) >> 4));
      buffer.append(nybbleToChar(content[i]&0x0f));
    }
    return buffer.toString();
  }

  char nybbleToChar
  (int nybble)
  {
    if ((nybble >= 0)&&(nybble <= 9)) {
      return (char)('0' + (char)nybble);
    } else {
      return (char)('a' + (char)nybble - 10);
    }
  }

  public Object clone
  ()
  {
    return new BinaryBaseValue(this);
  }

  public boolean equals
  (BaseValue bv)
  {
    if (bv.getType() != BINARY) {
      return false;
    } else if (((BinaryBaseValue)bv).content.length != content.length) {
      return false;
    } else {
      for (int i = 0; i < content.length; i++) {
        if (content[i] != ((BinaryBaseValue)bv).content[i]) {
          return false;
        }
      }
      return true;
    }
  }

  public boolean refinesTo
  (BaseValue bv)
  {
    if (bv.getType() == OMEGA) {
      return true;
    }
    return equals(bv);
  }

  public boolean lessThan
  (BaseValue bv)
  {
    if (bv.getType() != BINARY) {
      // No comparing incompatible types:
      return false;
    } else {
      int maxLength;
      BinaryBaseValue bvPtr = (BinaryBaseValue)(bv);
      if (bvPtr.content.length > content.length) {
        maxLength = content.length;
      } else {
        maxLength = bvPtr.content.length;
      }
      for (int i = 0; i < maxLength; i++) {
        if (content[i] < ((BinaryBaseValue)bv).content[i]) {
          return true;
        } else if (content[i] > ((BinaryBaseValue)bv).content[i]) {
          return false;
        }
      }
      if (content.length < bvPtr.content.length) {
        return true;
      } else {
        return false;
      }
    }
  }

  public BaseValue assign
  (BaseValue bv)
    throws IntenseException
  {
    if (bv.getType() == BINARY) {
      BinaryBaseValue bvPtr = (BinaryBaseValue)bv;
      content = new byte[bvPtr.content.length];
      for (int i = 0; i < content.length; i++) {
        content[i] = bvPtr.content[i];
      }
    } else {
      content = bv.canonical().getBytes();
    }
    return this;
  }

  public BaseValue assign
  (String s)
    throws IntenseException
  {
    content = s.getBytes();
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
    out.writeInt(content.length);
    if (content.length > 0) {
      out.write(content, 0, content.length);
    }
  }

  /**
   * Inbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  void deserialise
  (DataInputStream in)
    throws IOException
  {
    int length = in.readInt();

    if (length > MAX_LENGTH) {
      throw new IOException(
        "Attempt to deserialise binary string of length " + length +
        " > max allowed length " + MAX_LENGTH
      );
    } else {
      content = new byte[length];
    }
    if (length > 0) {
      try {
        in.readFully(content);
      } catch (EOFException e) {
        throw new IOException("EOF reached deserialising BinaryBaseValue");
      }
    }
  }

  /**
   * Convert a hex string of the form /([a-fA-F0-9][a-fA-F0-9])+/ to a
   * BinaryBaseValue.
   *
   * @param hexString The ASCII hex string to convert.
   * @return The converted BinaryBaseValue.
   * @throws IntenseException if the argument hex string is invalid.
   */
  public static BinaryBaseValue hexStringToBinaryBaseValue
  (String hexString)
    throws IntenseException
  {
    BinaryBaseValue value = new BinaryBaseValue(hexString.length() / 2);
    byte[] byteArray = hexString.getBytes();

    if ((hexString.length()%2) != 0) {
      throw new IntenseException(
        "Binary hex string length " + hexString.length() + " is not even"
      );
    }
    for (int i = 0; i < byteArray.length / 2; i++) {
      byte character = 0;
      if ((byteArray[2*i] >= '0')&&(byteArray[2*i] <= '9')) {
        character = (byte)((byteArray[2*i] - '0') << 4);
      } else if ((byteArray[2*i] >= 'a')&&(byteArray[2*i] <= 'f')) {
        character = (byte)((byteArray[2*i] - 'a' + 10) << 4);
      } else if ((byteArray[2*i] >= 'A')&&(byteArray[2*i] <= 'F')) {
        character = (byte)((byteArray[2*i] - 'A' + 10) << 4);
      } else {
        throw new IntenseException(
          "Invalid char " + ((int)byteArray[2*i]) + " int binary hex string"
        );
      }
      if ((byteArray[2*i + 1] >= '0')&&(byteArray[2*i + 1] <= '9')) {
        character |= (byte)(byteArray[2*i + 1] - '0');
      } else if ((byteArray[2*i + 1] >= 'a')&&(byteArray[2*i + 1] <= 'f')) {
        character |= (byte)(byteArray[2*i + 1] - (byte)'a' + (byte)10);
      } else if ((byteArray[2*i + 1] >= 'A')&&(byteArray[2*i + 1] <= 'F')) {
        character |= (byte)(byteArray[2*i + 1] - (byte)'A' + (byte)10);
      } else {
        throw new IntenseException(
          "Invalid char " + ((int)byteArray[2*i + 1]) + " in binary hex string"
        );
      }
      value.content[i] = character;
    }
    return value;
  }

}
