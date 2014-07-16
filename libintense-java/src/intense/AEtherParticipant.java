// ****************************************************************************
//
// AEtherParticipant.java - Participants (directly) in AEthers.
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
 * Participants in (direct) AEthers.
 */
public abstract class AEtherParticipant
  implements Participant {

  /**
   * The AEther to which this Participant is joined.  (null if no AEther is
   * joined).
   */
  protected AEther aether;

  /**
   * Void constructor.
   */
  public AEtherParticipant
  ()
  {
    aether = null;
  }

  /**
   * Initialization constructor.
   */
  public AEtherParticipant
  (AEther aether)
    throws IntenseException
  {
    join(aether);
  }

  /**
   * Leave the currently-joined AEther.
   */
  public void leave
  ()
    throws IntenseException
  {
    if (aether != null) {
      aether.detach(this);
      aether = null;
    }
  }

  /**
   * Join an AEther.
   *
   * @param aether The AEther to join.
   */
  public void join
  (AEther aether)
    throws IntenseException
  {
    leave();
    this.aether = aether;
    aether.attach(this);
  }

  /**
   * Assign a Context to the AEther in which this Participant is registered.
   *
   * @param context The Context to assign to the AEther.
   * @param dim The dimension to which the Context is assigned (null if the
   * desired target node in the AEther is the same as the Participant's node in
   * the AEther).
   * @return An implementation-specific sequence number (-1 by default).
   */
  public long assign
  (Context context, CompoundDimension dim)
    throws IntenseException
  {
    if (aether != null) {
      if (dim == null) {
        aether.assign(context, this);
      } else {
        ((AEther)aether.value(dim)).assign(context, this);
      }
    } else {
      throw new IntenseException(
        "Attempt to assign in an unjoined Participant"
      );
    }
    return 0;
  }

  /**
   * Apply a ContextOp to the AEther in which this Participant is registered.
   *
   * @param context The operator to apply to the AEther.
   * @param dim The dimension to which the ContextOp is applied (null if the
   * desired target node in the AEther is the same as the Participant's node in
   * the AEther).
   * @return An implementation-specific sequence number (-1 by default).
   */
  public long apply
  (ContextOp op, CompoundDimension dim)
    throws IntenseException
  {
    if (aether != null) {
      if (dim == null) {
        aether.apply(op, this);
      } else {
        ((AEther)aether.value(dim)).apply(op, this);
      }
    } else {
      throw new IntenseException(
        "Attempt to apply in an unjoined Participant"
      );
    }
    return 0;
  }

  /**
   * Clear the AEther in which this Participant resides.
   *
   * @param dim The dimension under which to clear (null if the desired target
   * node in the AEther is the same as the Participant's current node in the
   * AEther).
   * @return An implementation-specific sequence number (-1 by default).
   */
  public long clear
  (CompoundDimension dim)
    throws IntenseException
  {
    if (aether != null) {
      if (dim == null) {
        aether.clear(this);
      } else {
        ((AEther)aether.value(dim)).clear(this);
      }
    } else {
      throw new IntenseException(
        "Attempt to clear in an unjoined Participant"
      );
    }
    return 0;
  }

  /**
   * Get a reference to the node at which this AEtherParticipant is currently
   * joined.
   *
   * @return The node at which this AEtherParticipant is joined.
   * @throws IntenseException if this AEtherParticpiant is not currently joined
   * to any AEther node.
   */
  public AEther getNode
  ()
    throws IntenseException
  {
    if (aether == null) {
      throw new IntenseException(
        "AEtherParticipant is not joined in AEtherParticipant.getNode"
      );
    }
    return aether;
  }

}
