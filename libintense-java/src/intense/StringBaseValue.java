// ****************************************************************************
//
// StringBaseValue.java - BaseValues with string values.
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
public class StringBaseValue
  extends BaseValue {

  static final long serialVersionUID = 4850787497324813468L;

  public static final int MAX_LENGTH = 1048576;

  /**
   * The String value of this BaseValue.
   */
  private String string;

  /**
   * Copy constructor.
   *
   * @param bv The BaseValue from which to take the canonical form for the
   * String value of this StringBaseValue.
   */
  public StringBaseValue
  (BaseValue bv)
  {
    string = bv.canonical();
  }

  /**
   * Initialization constructor.
   *
   * @param s The initial String value of this StringBaseValue.
   */
  public StringBaseValue
  (String s)
  {
    string = s;
  }

  /**
   * Void constructor.
   */
  public StringBaseValue
  ()
  {
    string = "";
  }

  /**
   * Get the subclass type.
   *
   * @return An integer representing the subclass type.
   */
  public int getType
  ()
  {
    return STRING;
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
    int length = string.length();

    buffer.append("\"");
    for (int i = 0; i < length; i++) {
      switch (string.charAt(i)) {
      case '\\':
        buffer.append("\\\\");
        break;
      case '\n':
        buffer.append("\\n");
        break;
      case '\t':
        buffer.append("\\t");
        break;
      case '\013':
        buffer.append("\\v");
        break;
      case '\b':
        buffer.append("\\b");
        break;
      case '\r':
        buffer.append("\\r");
        break;
      case '\f':
        buffer.append("\\f");
        break;
      case '\007':
        buffer.append("\\a");
        break;
      case '\"':
        buffer.append("\\\"");
        break;
      default:
        buffer.append(string.charAt(i));
        break;
      }
    }
    buffer.append("\"");
    return buffer.toString();
  }

  public Object clone
  ()
  {
    return new StringBaseValue(string);
  }

  public boolean equals
  (BaseValue bv)
  {
    if (bv.getType() != STRING) {
      return false;
    } else {
      return string.equals(((StringBaseValue)bv).string);
    }
  }

  public boolean refinesTo
  (BaseValue bv)
  {
    if (bv.getType() == OMEGA) {
      return true;
    } if (bv.getType() != STRING) {
      return false;
    } else {
      return canonical().equals(bv.canonical());
    }
  }

  public boolean lessThan
  (BaseValue bv)
  {
    if (bv.getType() != STRING) {
      // No comparing incompatible types:
      return false;
    } else {
      return canonical().compareTo(bv.canonical()) < 0;
    }
  }

  public BaseValue assign
  (BaseValue bv)
    throws IntenseException
  {
    string = bv.canonical();
    return this;
  }

  public BaseValue assign
  (String s)
    throws IntenseException
  {
    string = s;
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
    byte[] bytes = string.getBytes("UTF-8");

    out.writeInt(bytes.length);
    if (bytes.length > 0) {
      out.write(bytes, 0, bytes.length);
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
    byte[] buffer;

    if (length > MAX_LENGTH) {
      throw new IOException(
        "Attempt to deserialise string base value of length " + length +
        " > max allowed length " + MAX_LENGTH
      );
    } else {
      buffer = new byte[length];
    }
    if (length > 0) {
      try {
        in.readFully(buffer);
      } catch (EOFException e) {
        throw new IOException("EOF reached deserialising StringBaseValue");
      }
    }
    string = new String(buffer, 0, length, "UTF-8");
  }

}
