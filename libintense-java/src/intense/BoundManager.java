// ****************************************************************************
//
// BoundManager.java - Serialisation and deserialisation of BoundBaseValues.
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


/**
 * Abstract base class for version base values.
 */
public interface BoundManager {

  /**
   * Serialise the bound object of a BoundBaseValue to a byte array.
   *
   * @return The unpacked Object, or null if no suitable object type could
   * be determined.
   */
  abstract public byte[] pack
  (Object bound)
    throws IOException;

  /**
   * Deserialise the bound data of a BoundBaseValue from a byte array.
   *
   * @return The unpacked Object, or null if no suitable object type could
   * be determined.
   */
  abstract public Object unpack
  (byte[] data)
    throws IOException;

}
