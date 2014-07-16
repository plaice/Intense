// ****************************************************************************
//
// CompoundDimension.java - Multi-part Context dimensions.
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
 * Multi-part Context dimensions.
 */
public class CompoundDimension
  implements Serializable {

  static final long serialVersionUID = -1198581100483469303L;

  private LinkedList<Dimension> list;

  private static final int maxCompoundDimensionSize = 1024;

  /**
   * Void constructor.
   */
  public CompoundDimension
  ()
  {
    list = new LinkedList<Dimension>();
  }

  /**
   * Copy constructor.
   */
  public CompoundDimension
  (CompoundDimension src)
  {
    Iterator itr;

    list = new LinkedList<Dimension>();
    itr = src.list.iterator();
    while (itr.hasNext()) {
      Dimension dimension = (Dimension)itr.next();

      list.addLast(dimension);
    }
  }

  /**
   * Parse constructor.
   *
   * @param value A String to parse as the CompoundDimension value.  Must not
   * contain a trailing colon (e.g. - "dim1:dim2:dim3").
   */
  public CompoundDimension
  (String value)
    throws IntenseException
  {
    list = new LinkedList<Dimension>();

    parse(value);
  }

  /**
   * Clear this compound dimension of all dimensions.
   */
  public void clear
  ()
  {
    list.clear();
  }

  /**
   * Add a Dimension to the end of this CompoundDimension.
   *
   * @param dimension The Dimension to add.
   */
  public void append
  (Dimension dimension)
  {
    list.add(dimension);
  }

  /**
   * Add a Dimension to the beginning of this CompoundDimension.
   *
   * @param dimension The Dimension to add.
   */
  public void prepend
  (Dimension dimension)
  {
    list.addFirst(dimension);
  }

  /**
   * Canonical form string accessor.
   *
   * @return The canonical form string for this Dimension.
   */
  public String canonical
  ()
  {
    StringBuffer buffer = new StringBuffer();
    Iterator itr = iterator();

    if (itr.hasNext()) {
      buffer.append(((Dimension)itr.next()).canonical());
    }
    while (itr.hasNext()) {
      buffer.append(':' + ((Dimension)itr.next()).canonical());
    }
    return buffer.toString();
  }

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
   * Obtain an Iterator for the LinkedList of Dimensions underlying this
   * CompoundDimension.
   *
   * @return the Iterator.
   */
  public java.util.Iterator iterator
  ()
  {
    return list.iterator();
  }

  /**
   * Parse a CompoundDimension from a stream.
   *
   * @param in A stream to parse from.
   */
  public void parse
  (Reader r)
    throws IntenseException
  {
    ContextOpLexer lexer = new ContextOpLexer(r);
    Context.Token token = new Context.Token();
    boolean done = false;

    list.clear();
    while (!done) {
      int tokenType;

      try {
        tokenType = lexer.getToken(token);
      } catch (IOException e) {
        throw new IntenseException(
          "Caught IOException in CompoundDimension parse: " + e.getMessage(), e
        );
      }
      switch (tokenType) {
      case Context.Token.DIMENSION:
        list.add((Dimension)token.value);
        break;
      case Context.Token.ERROR:
        throw new IntenseException(
          "Bad token \"" + token.value + "\" in compound dimension"
        );
      case Context.Token.EOF:
        done = true;
        break;
      default:
        throw new IntenseException(
          "Bad token type " + token.getType() + " in compound dimension"
        );
      }
    }
  }

  public void parse
  (String src)
    throws IntenseException
  {
    parse(new StringReader(src + ':'));
  }

  /**
   * Outbound serialisation, compatible with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  public void serialise
  (DataOutputStream out)
    throws IOException
  {
    out.writeInt(list.size());
    Iterator itr = list.iterator();

    while (itr.hasNext()) {
      Dimension.factorySerialise((Dimension)itr.next(), out);
    }
  }

  /**
   * Inbound serialisation, compatible with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  public void deserialise
  (DataInputStream in)
    throws IOException, IntenseException
  {
    int sizeInt;

    list.clear();
    sizeInt = in.readInt();
    if (sizeInt < 0) {
      throw new IntenseException(
        "Attempt to deserialise CompoundDimension with negative size"
      );
    }
    if (sizeInt > maxCompoundDimensionSize) {
      throw new IntenseException(
        "Attempt to deserialise CompoundDimension with size " + sizeInt +
        " > max allowed size " + maxCompoundDimensionSize
      );
    }
    for (int i = 0; i < sizeInt; i++) {
      list.add(Dimension.factoryDeserialise(in));
    }
  }

}
