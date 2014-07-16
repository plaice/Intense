// ****************************************************************************
//
// StringDimension.java - String-valued Context dimensions.
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
 * String-valued Context dimensions.
 */
public class StringDimension
  extends Dimension {

  static final long serialVersionUID = 3862214807950171580L;

  // Personally, I think this is generous:
  private static final int maxStringDimensionLength = 1024;

  private String value;

  private boolean requiresQuotes;

  /**
   * Void constructor.
   */
  protected StringDimension
  ()
  {
    this.value = null;
  }

  /**
   * Value constructor.
   *
   * @param value The value of this StringDimension.
   */
  public StringDimension
  (String value, boolean requiresQuotes)
  {
    this.value = value;
    this.requiresQuotes = requiresQuotes;
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
   * Canonical form string accessor.
   *
   * @return The canonical form string for this Dimension.
   */
  public String canonical
  ()
  {
    if (requiresQuotes) {
      StringBuffer buffer = new StringBuffer();
      int length = value.length();

      buffer.append("\"");
      for (int i = 0; i < length; i++) {
        switch (value.charAt(i)) {
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
        case '"':
          buffer.append("\\\"");
          break;
        default:
          buffer.append(value.charAt(i));
          break;
        }
      }
      buffer.append("\"");
      return buffer.toString();
    } else {
      return value;
    }
  }

  /**
   * Object copy.
   *
   * @return A complete copy of this Dimension.
   */
  public Object clone
  ()
  {
    return new StringDimension(value, requiresQuotes);
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
      return value.compareTo(((StringDimension)dim).value);
    case INTEGER:
      return 1;
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
    byte[] bytes = value.getBytes("UTF-8");

    if (requiresQuotes) {
      out.writeByte(1);
    } else {
      out.writeByte(0);
    }
    out.writeInt(bytes.length);
    if (bytes.length > 0) {
      out.write(bytes, 0, bytes.length);
    }
  }

  /**
   * Inbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  public void deserialise
  (DataInputStream in)
    throws IOException
  {
    int length;
    byte[] buffer;

    requiresQuotes = (in.readByte() == 1);
    length = in.readInt();
    if (length > maxStringDimensionLength) {
      throw new IOException(
        "Attempt to deserialise string dimension of length " + length +
        " > max allowed length " + maxStringDimensionLength
      );
    } else {
      buffer = new byte[length];
    }
    if (length > 0) {
      try {
        in.readFully(buffer);
      } catch (EOFException e) {
        throw new IOException("EOF reached deserialising StringDimension");
      }
    }
    value = new String(buffer, 0, length, "UTF-8");
  }

}
