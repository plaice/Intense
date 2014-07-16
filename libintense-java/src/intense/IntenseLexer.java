// ****************************************************************************
//
// IntenseLexer.java - Interface for ContextOp/AETP lexical analysers.
//
// Copyright 2004 Paul Swoboda.
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
 * Interface for ContextOp/AETP lexical analysers.
 */
public interface IntenseLexer {

  /**
   * Get a Context.Token.
   *
   * @return The type of the token.
   */
  public abstract int getToken
  (Context.Token token)
    throws IOException, IntenseException;

}
