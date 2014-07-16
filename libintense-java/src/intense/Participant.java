// ****************************************************************************
//
// Participant.java - Interface for participants in aethers.
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


/**
 * Participants in AEthers.
 */
public interface Participant
  extends Origin {

  /**
   * Notify this Participant of a context assignment to the AEther under which
   * the Participant is registered.
   *
   * @param context The Context assigned to the AEther.
   * @param dim The dimension under which the assignment occurred (null if the
   * desired target node in the AEther is the same as the Participant's node in
   * the AEther).
   * @param origin An implementation-specific origin object, denoting the
   * source of the operation (null by default).
   */
  public abstract void assignNotify
  (Context context, CompoundDimension dim, Origin origin)
    throws IntenseException;

  /**
   * Notify this Participant of a context operation.
   *
   * @param dim The dimension under which the operation occurred (null if the
   * AEther is the same as the Participant's current AEther).
   * @param op The context operation.
   * @param origin An implementation-specific origin object, denoting the
   * source of the operation (null by default).
   */
  public abstract void applyNotify
  (ContextOp op, CompoundDimension dim, Origin origin)
    throws IntenseException;

  /**
   * Notify this Participant that the AEther in which it resides has been
   * cleared.
   *
   * @param dim The dimension under which the operation occurred (null if the
   * AEther is the same as the Participant's current AEther).
   * @param origin An implementation-specific origin object, denoting the
   * source of the operation (null by default).
   */
  public abstract void clearNotify
  (CompoundDimension dim, Origin origin)
    throws IntenseException;

  /**
   * This is to notify the Participant that it has been removed from the
   * calling AEther's list of Participants.  It should be assumed that the
   * caller will delete this Participant shortly after kickNotify() returns.
   *
   * @param origin An implementation-specific origin object, denoting the
   * source of the operation (null by default).
   */
  public abstract void kickNotify
  (Origin origin)
    throws IntenseException;

  /**
   * This is called prior to notification of an assign or a clear - if it
   * returns true, the participant is considered a "pure" participant, i.e.,
   * one which receives only context operations and kick notifications,
   * requiring that assignments and clears be converted to context operations
   * prior to notification via applyNotify().
   *
   * @return true if the participant only accepts apply notifications, false
   * otherwise.
   */
  public abstract boolean isPure
  ();

}
