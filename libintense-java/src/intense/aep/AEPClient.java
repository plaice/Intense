// ****************************************************************************
//
// AEPClient.java : AEP 2.0 client, for the abstract/core AEP implementation.
//
// Copyright 2001, 2002, 2004 Paul Swoboda.
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


package intense.aep;


import java.lang.*;
import java.util.*;
import java.util.concurrent.locks.*;
import java.io.*;
import intense.*;
import intense.log.*;


public abstract class AEPClient
  extends AEPCommon {


  public abstract static class Participant
    implements AEPCommon.Participant, intense.Participant {

    protected long id;

    protected Log log;

    protected int logLevel;

    AEPClient client;

    protected Participant
    ()
    {
      construct(null, Log.NOTICE);
      client = null;
    }

    protected Participant
    (Log log)
    {
      construct(log, Log.NOTICE);
      client = null;
    }

    protected Participant
    (Log log, int logLevel)
    {
      construct(log, logLevel);
      client = null;
    }

    protected Participant
    (AEPClient client)
    {
      construct(null, Log.NOTICE);
      this.client = client;
    }

    protected Participant
    (AEPClient client, Log log)
    {
      construct(log, Log.NOTICE);
      this.client = client;
    }

    protected Participant
    (AEPClient client, Log log, int logLevel)
    {
      construct(log, logLevel);
      this.client = client;
    }

    protected void construct
    (Log log, int logLevel)
    {
      id = -1;
      this.log = log;
      this.logLevel = logLevel;
    }

    protected void construct
    (long id, Log log, int logLevel)
    {
      this.id = id;
      this.log = log;
      this.logLevel = logLevel;
    }

    /**
     * Get the numeric id of this participant.
     */
    public long getId
    ()
    {
      return id;
    }    

    /**
     * Set the numeric id of this participant.
     */
    public void setId
    (long id)
    {
      this.id = id;
    }    

    /**
     * Set the Log handle used by this participant.
     */
    public void setLog
    (Log log)
    {
      this.log = log;
    }

    /**
     * Get the Log handle used by this participant.
     */
    public Log getLog
    ()
    {
      return log;
    }

    /**
     * Set the intense.Log level used by this participant.
     */
    public void setLogLevel
    (int newLogLevel)
      throws LogException
    {
      Log.checkPriority(logLevel);
      this.logLevel = logLevel;
    }

    /**
     * Get the intense.Log level used by this participant.
     */
    public int getLogLevel
    ()
    {
      return logLevel;
    }

    /**
     * Join this participant a the root dimension of its client.
     *
     * @param notify If true, the participant's assignNotify method will be
     * called with the initial context, prior to the call returning.
     * @return The server sequence of the AEP join.
     */
    public long join
    (boolean notify)
      throws IOException, IntenseException, AEPException
    {
      return join(null, notify, (CompoundDimension)null, null);
    }

    /**
     * Join this participant a the root dimension of a new client.
     *
     * @param newClient A new AEPClient for this participant.
     * @param notify If true, the participant's assignNotify method will be
     * called with the initial context, prior to the call returning.
     * @return The server sequence of the AEP join.
     */
    public long join
    (AEPClient newClient, boolean notify)
      throws IOException, IntenseException, AEPException
    {
      return join(newClient, notify, (CompoundDimension)null, null);
    }

    /**
     * Join this participant under a specific dimension of a new client.
     *
     * @param newClient A new AEPClient for this participant.
     * @return The server sequence of the AEP join.
     * @param dimension A CompoundDimension under which to join.
     * @param notify If true, the participant's assignNotify method will be
     * called with the initial context, prior to the call returning.
     */
    public long join
    (AEPClient newClient, boolean notify, CompoundDimension dimension)
      throws IOException, IntenseException, AEPException
    {
      return join(newClient, notify, dimension, null);
    }

    /**
     * Join this participant under a specific dimension of a new client.
     *
     * @param newClient A new AEPClient for this participant.
     * @param notify If true, the participant's assignNotify method will be
     * called with the initial context, prior to the call returning.
     * @param dimension A CompoundDimension under which to join.
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP join.
     */
    public long join
    (AEPClient newClient, boolean notify, CompoundDimension dimension,
     Condition optionalBlockCondition)
      throws IOException, IntenseException, AEPException
    {
      if ((client != null)&&(getId() >= 0)) {
        leave();
      }
      if (newClient != null) {
        client = newClient;
      } else {
        if (client == null) {
          throw new AEPException(
            "null AEPClient in AEPClient.Participant.join()", Log.ERROR
          );
        }
      }
      client.lock();
      client.addParticipant(this);
      return client.transaction(
        new AEPServer.JoinToken(
          client.nextSequence(), getId(), notify, dimension
        ),
        true, false, optionalBlockCondition
      );
    }

    /**
     * Join this participant under a specific dimension of a new client.
     *
     * @param newClient A new AEPClient for this participant.
     * @param notify If true, the participant's assignNotify method will be
     * called with the initial context, prior to the call returning.
     * @param dimension A CompoundDimension (to be parsed) under which to join.
     * @return The server sequence of the AEP join.
     */
    public long join
    (AEPClient newClient, boolean notify, String dimension)
      throws IOException, IntenseException, AEPException
    {
      CompoundDimension compoundDimension = new CompoundDimension(dimension);

      return join(newClient, notify, compoundDimension, null);
    }

    /**
     * Join this participant under a specific dimension of a new client.
     *
     * @param newClient A new AEPClient for this participant.
     * @param notify If true, the participant's assignNotify method will be
     * called with the initial context, prior to the call returning.
     * @param dimension A CompoundDimension (to be parsed) under which to join.
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP join.
     */
    public long join
    (AEPClient newClient, boolean notify, String dimension,
     Condition optionalBlockCondition)
      throws IOException, IntenseException, AEPException
    {
      CompoundDimension compound = new CompoundDimension(dimension);

      return join(newClient, notify, compound, optionalBlockCondition);
    }

    /**
     * Join this participant under a specific dimension of the current client.
     *
     * @param notify If true, the participant's assignNotify method will be
     * called with the initial context, prior to the call returning.
     * @param dimension A CompoundDimension under which to join.
     * @return The server sequence of the AEP join.
     */
    public long join
    (boolean notify, CompoundDimension dimension)
      throws IOException, IntenseException, AEPException
    {
      return join(null, notify, dimension, null);
    }

    /**
     * Join this participant under a specific dimension of the current client.
     *
     * @param notify If true, the participant's assignNotify method will be
     * called with the initial context, prior to the call returning.
     * @param dimension A CompoundDimension under which to join.
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP join.
     */
    public long join
    (boolean notify, CompoundDimension dimension,
     Condition optionalBlockCondition)
      throws IOException, IntenseException, AEPException
    {
      return join(null, notify, dimension, optionalBlockCondition);
    }

    /**
     * Join this participant under a specific dimension of the current client.
     *
     * @param notify If true, the participant's assignNotify method will be
     * called with the initial context, prior to the call returning.
     * @param dimension A CompoundDimension (to be parsed) under which to join.
     * @return The server sequence of the AEP join.
     */
    public long join
    (boolean notify, String dimension)
      throws IOException, IntenseException, AEPException
    {
      CompoundDimension compoundDimension = new CompoundDimension(dimension);

      return join(null, notify, compoundDimension, null);
    }

    /**
     * Join this participant under a specific dimension of the current client.
     *
     * @param notify If true, the participant's assignNotify method will be
     * called with the initial context, prior to the call returning.
     * @param dimension A CompoundDimension (to be parsed) under which to join.
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP join.
     */
    public long join
    (boolean notify, String dimension, Condition optionalBlockCondition)
      throws IOException, IntenseException, AEPException
    {
      CompoundDimension compoundDimension = new CompoundDimension(dimension);

      return join(null, notify, compoundDimension, optionalBlockCondition);
    }

    /**
     * This is called prior to notification of an assign or a clear - if it
     * returns true, the participant is considered a "pure" participant, i.e.,
     * one which receives only context operations and kick notifications,
     * requiring that assignments and clears be converted to context operations
     * prior to notification via applyNotify().
     *
     * @return always false.
     */
    public boolean isPure
    ()
    {
      return false;
    }

    /**
     * Leave this participant from any dimension currently joined.
     *
     * @return The server sequence of the AEP join.
     */
    public long leave
    ()
      throws IOException, IntenseException, AEPException
    {
      return leave(null);
    }

    /**
     * Leave this participant from any dimension currently joined.
     *
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP leave.
     */
    public long leave
    (Condition optionalBlockCondition)
      throws IOException, IntenseException, AEPException
    {
      String location = "AEPClient.Participant.leave";
      long returnValue;

      checkValidClientAndJoined(location);
      client.lock();
      returnValue = client.transaction(
        new AEPServer.LeaveToken(client.nextSequence(), getId()),
        true, false, optionalBlockCondition
      );
      client.removeParticipant(this, location);
      return returnValue;
    }

    /**
     * Assign a Context to the AEther in which this Participant is registered.
     *
     * @param context The Context to assign to the AEther.
     * @return The server sequence of the AEP assign.
     */
    public long assign
    (Context context)
      throws IOException, IntenseException, AEPException
    {
      return assign(context, (CompoundDimension)null, 0, null);
    }

    /**
     * Assign a Context to the AEther in which this Participant is registered.
     *
     * @param context The Context to assign to the AEther.
     * @param dim The relative dimension to which the Context is assigned (null
     * if the desired target node in the AEther is the same as the
     * Participant's node in the AEther).
     * @return The server sequence of the AEP assign.
     */
    public long assign
    (Context context, CompoundDimension dim)
      throws IOException, IntenseException, AEPException
    {
      return assign(context, dim, 0, null);
    }

    /**
     * Assign a Context to the AEther in which this Participant is registered.
     *
     * @param context The Context to assign to the AEther.
     * @param dim The relative dimension to which the Context is assigned (null
     * if the desired target node in the AEther is the same as the
     * Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @return The server sequence of the AEP assign.
     */
    public long assign
    (Context context, CompoundDimension dim, int flags)
      throws IOException, IntenseException, AEPException
    {
      return assign(context, dim, flags, null);
    }

    /**
     * Assign a Context to the AEther in which this Participant is registered.
     *
     * @param context The Context to assign to the AEther.
     * @param dim The relative dimension (to be parsed) to which the Context is
     * assigned (null if the desired target node in the AEther is the same as
     * the Participant's node in the AEther).
     * @return The server sequence of the AEP assign.
     */
    public long assign
    (Context context, String dim)
      throws IOException, IntenseException, AEPException
    {
      return assign(context, dim, 0, null);
    }

    /**
     * Assign a Context to the AEther in which this Participant is registered.
     *
     * @param context The Context to assign to the AEther.
     * @param dim The relative dimension (to be parsed) to which the Context is
     * assigned (null if the desired target node in the AEther is the same as
     * the Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @return The server sequence of the AEP assign.
     */
    public long assign
    (Context context, String dim, int flags)
      throws IOException, IntenseException, AEPException
    {
      return assign(context, dim, flags, null);
    }

    /**
     * Assign a Context to the AEther in which this Participant is registered.
     *
     * @param context The Context to assign to the AEther.
     * @param dim The relative dimension (to be parsed) to which the Context is
     * assigned (null if the desired target node in the AEther is the same as
     * the Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP assign.
     */
    public long assign
    (Context context, String dim, int flags, Condition optionalBlockCondition)
      throws IOException, IntenseException, AEPException
    {
      CompoundDimension compoundDimension = new CompoundDimension(dim);

      return assign(context, compoundDimension, flags, optionalBlockCondition);
    }

    /**
     * Assign a Context to the AEther in which this Participant is registered.
     *
     * @param context The Context (to be parsed) to assign to the AEther.
     * @return The server sequence of the AEP assign.
     */
    public long assign
    (String context)
      throws IOException, IntenseException, AEPException
    {
      return assign(new Context(context), (CompoundDimension)null, 0, null);
    }

    /**
     * Assign a Context to the AEther in which this Participant is registered.
     *
     * @param context The Context (to be parsed) to assign to the AEther.
     * @param dim The relative dimension to which the Context is assigned (null
     * if the desired target node in the AEther is the same as the
     * Participant's node in the AEther).
     * @return The server sequence of the AEP assign.
     */
    public long assign
    (String context, CompoundDimension dim)
      throws IOException, IntenseException, AEPException
    {
      return assign(new Context(context), dim, 0, null);
    }

    /**
     * Assign a Context to the AEther in which this Participant is registered.
     *
     * @param context The Context (to be parsed) to assign to the AEther.
     * @param dim The relative dimension to which the Context is assigned (null
     * if the desired target node in the AEther is the same as the
     * Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @return The server sequence of the AEP assign.
     */
    public long assign
    (String context, CompoundDimension dim, int flags)
      throws IOException, IntenseException, AEPException
    {
      return assign(new Context(context), dim, flags, null);
    }

    /**
     * Assign a Context to the AEther in which this Participant is registered.
     *
     * @param context The Context (to be parsed) to assign to the AEther.
     * @param dim The relative dimension (to be parsed) to which the Context is
     * assigned (null if the desired target node in the AEther is the same as
     * the Participant's node in the AEther).
     * @return The server sequence of the AEP assign.
     */
    public long assign
    (String context, String dim)
      throws IOException, IntenseException, AEPException
    {
      return assign(new Context(context), dim, 0, null);
    }

    /**
     * Assign a Context to the AEther in which this Participant is registered.
     *
     * @param context The Context (to be parsed) to assign to the AEther.
     * @param dim The relative dimension (to be parsed) to which the Context is
     * assigned (null if the desired target node in the AEther is the same as
     * the Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @return The server sequence of the AEP assign.
     */
    public long assign
    (String context, String dim, int flags)
      throws IOException, IntenseException, AEPException
    {
      return assign(new Context(context), dim, flags, null);
    }

    /**
     * Assign a Context to the AEther in which this Participant is registered.
     *
     * @param context The Context (to be parsed) to assign to the AEther.
     * @param dim The relative dimension (to be parsed) to which the Context is
     * assigned (null if the desired target node in the AEther is the same as
     * the Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP assign.
     */
    public long assign
    (String context, String dim, int flags, Condition optionalBlockCondition)
      throws IOException, IntenseException, AEPException
    {
      CompoundDimension compoundDimension = new CompoundDimension(dim);

      return assign(
        new Context(context), compoundDimension, flags, optionalBlockCondition
      );
    }

    /**
     * Assign a Context to the AEther in which this Participant is registered.
     *
     * @param context The Context to assign to the AEther.
     * @param dim The relative dimension to which the Context is assigned (null
     * if the desired target node in the AEther is the same as the
     * Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP assign.
     */
    public long assign
    (Context context, CompoundDimension dim, int flags,
     Condition optionalBlockCondition)
      throws IOException, IntenseException, AEPException
    {
      checkValidClientAndJoined("AEPClient.Participant.assign");
      client.lock();
      return client.transaction(
        new AEPServer.AssignToken(
          client.nextSequence(), getId(), context, dim, flags
        ),
        true, false, optionalBlockCondition
      );
    }

    /**
     * Apply a ContextOp to the AEther in which this Participant is registered.
     *
     * @param op The ContextOp to apply to the AEther.
     * @return The server sequence of the AEP apply.
     */
    public long apply
    (ContextOp op)
      throws IOException, IntenseException, AEPException
    {
      return apply(op, (CompoundDimension)null, 0, null);
    }

    /**
     * Apply a ContextOp to the AEther in which this Participant is registered.
     *
     * @param op The ContextOp to apply to the AEther.
     * @param dim The relative dimension to which op is applied (null if the
     * desired target node in the AEther is the same as the Participant's node
     * in the AEther).
     * @return The server sequence of the AEP apply.
     */
    public long apply
    (ContextOp op, CompoundDimension dim)
      throws IOException, IntenseException, AEPException
    {
      return apply(op, dim, 0, null);
    }

    /**
     * Apply a ContextOp to the AEther in which this Participant is registered.
     *
     * @param op The ContextOp to apply to the AEther.
     * @param dim The relative dimension to which op is applied (null if the
     * desired target node in the AEther is the same as the Participant's node
     * in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @return The server sequence of the AEP apply.
     */
    public long apply
    (ContextOp op, CompoundDimension dim, int flags)
      throws IOException, IntenseException, AEPException
    {
      return apply(op, dim, flags, null);
    }

    /**
     * Apply a ContextOp to the AEther in which this Participant is registered.
     *
     * @param op The ContextOp to apply to the AEther.
     * @param dim The relative dimension (to be parsed) to which op is applied
     * (null if the desired target node in the AEther is the same as the
     * Participant's node in the AEther).
     * @return The server sequence of the AEP apply.
     */
    public long apply
    (ContextOp op, String dim)
      throws IOException, IntenseException, AEPException
    {
      return apply(op, dim, 0, null);
    }

    /**
     * Apply a ContextOp to the AEther in which this Participant is registered.
     *
     * @param op The ContextOp to apply to the AEther.
     * @param dim The relative dimension (to be parsed) to which op is applied
     * (null if the desired target node in the AEther is the same as the
     * Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @return The server sequence of the AEP apply.
     */
    public long apply
    (ContextOp op, String dim, int flags)
      throws IOException, IntenseException, AEPException
    {
      return apply(op, dim, flags, null);
    }

    /**
     * Apply a ContextOp to the AEther in which this Participant is registered.
     *
     * @param op The ContextOp to apply to the AEther.
     * @param dim The relative dimension to which op is applied (null if the
     * desired target node in the AEther is the same as the Participant's node
     * in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP apply.
     */
    public long apply
    (ContextOp op, String dim, int flags, Condition optionalBlockCondition)
      throws IOException, IntenseException, AEPException
    {
      CompoundDimension compoundDimension = new CompoundDimension(dim);

      return apply(op, compoundDimension, flags, optionalBlockCondition);
    }

    /**
     * Apply a ContextOp to the AEther in which this Participant is registered.
     *
     * @param op The ContextOp (to be parsed) to apply to the AEther.
     * @return The server sequence of the AEP apply.
     */
    public long apply
    (String op)
      throws IOException, IntenseException, AEPException
    {
      return apply(new ContextOp(op), (CompoundDimension)null, 0, null);
    }

    /**
     * Apply a ContextOp to the AEther in which this Participant is registered.
     *
     * @param op The ContextOp (to be parsed) to apply to the AEther.
     * @param dim The relative dimension to which op is applied (null if the
     * desired target node in the AEther is the same as the Participant's node
     * in the AEther).
     * @return The server sequence of the AEP apply.
     */
    public long apply
    (String op, CompoundDimension dim)
      throws IOException, IntenseException, AEPException
    {
      return apply(new ContextOp(op), dim, 0, null);
    }

    /**
     * Apply a ContextOp to the AEther in which this Participant is registered.
     *
     * @param op The ContextOp (to be parsed) to apply to the AEther.
     * @param dim The relative dimension to which op is applied (null if the
     * desired target node in the AEther is the same as the Participant's node
     * in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @return The server sequence of the AEP apply.
     */
    public long apply
    (String op, CompoundDimension dim, int flags)
      throws IOException, IntenseException, AEPException
    {
      return apply(new ContextOp(op), dim, flags, null);
    }

    /**
     * Apply a ContextOp to the AEther in which this Participant is registered.
     *
     * @param op The ContextOp (to be parsed) to apply to the AEther.
     * @param dim The relative dimension (to be parsed) to which op is applied
     * (null if the desired target node in the AEther is the same as the
     * Participant's node in the AEther).
     * @return The server sequence of the AEP apply.
     */
    public long apply
    (String op, String dim)
      throws IOException, IntenseException, AEPException
    {
      return apply(new ContextOp(op), dim, 0, null);
    }

    /**
     * Apply a ContextOp to the AEther in which this Participant is registered.
     *
     * @param op The ContextOp (to be parsed) to apply to the AEther.
     * @param dim The relative dimension (to be parsed) to which op is applied
     * (null if the desired target node in the AEther is the same as the
     * Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @return The server sequence of the AEP apply.
     */
    public long apply
    (String op, String dim, int flags)
      throws IOException, IntenseException, AEPException
    {
      return apply(new ContextOp(op), dim, flags, null);
    }

    /**
     * Apply a ContextOp to the AEther in which this Participant is registered.
     *
     * @param op The ContextOp (to be parsed) to apply to the AEther.
     * @param dim The relative dimension (to be parsed) to which op is applied
     * (null if the desired target node in the AEther is the same as the
     * Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP apply.
     */
    public long apply
    (String op, String dim, int flags, Condition optionalBlockCondition)
      throws IOException, IntenseException, AEPException
    {
      CompoundDimension compoundDimension = new CompoundDimension(dim);

      return apply(
        new ContextOp(op), compoundDimension, flags, optionalBlockCondition
      );
    }

    /**
     * Apply a ContextOp to the AEther in which this Participant is registered.
     *
     * @param op The ContextOp to apply to the AEther.
     * @param dim The relative dimension to which op is applied (null if the
     * desired target node in the AEther is the same as the Participant's node
     * in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP apply.
     */
    public long apply
    (ContextOp op, CompoundDimension dim, int flags,
     Condition blockCondition)
      throws IOException, IntenseException, AEPException
    {
      checkValidClientAndJoined("AEPClient.Participant.apply");
      client.lock();
      return client.transaction(
        new AEPServer.ApplyToken(
          client.nextSequence(), getId(), op, dim, flags
        ),
        true, false, blockCondition
      );
    }

    /**
     * Clear the AEther in which this Participant is registered.
     *
     * @param dim The relative dimension under which the clear is to be
     * performed (null if the desired target node in the AEther is the same as
     * the Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP apply.
     */
    public long clear
    ()
      throws IOException, AEPException
    {
      return clear((CompoundDimension)null, 0, null);
    }

    /**
     * Clear the AEther in which this Participant is registered.
     *
     * @param dim The relative dimension under which the clear is to be
     * performed (null if the desired target node in the AEther is the same as
     * the Participant's node in the AEther).
     * @return The server sequence of the AEP clear.
     */
    public long clear
    (CompoundDimension dim)
      throws IOException, AEPException
    {
      return clear(dim, 0, null);
    }

    /**
     * Clear the AEther in which this Participant is registered.
     *
     * @param dim The relative dimension under which the clear is to be
     * performed (null if the desired target node in the AEther is the same as
     * the Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @return The server sequence of the AEP clear.
     */
    public long clear
    (CompoundDimension dim, int flags)
      throws IOException, AEPException
    {
      return clear(dim, flags, null);
    }

    /**
     * Clear the AEther in which this Participant is registered.
     *
     * @param dim The relative dimension (to be parsed) under which the clear
     * is to be performed (null if the desired target node in the AEther is the
     * same as the Participant's node in the AEther).
     * @return The server sequence of the AEP clear.
     */
    public long clear
    (String dim)
      throws IOException, IntenseException, AEPException
    {
      return clear(dim, 0, null);
    }

    /**
     * Clear the AEther in which this Participant is registered.
     *
     * @param dim The relative dimension (to be parsed) under which the clear
     * is to be performed (null if the desired target node in the AEther is the
     * same as the Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @return The server sequence of the AEP clear.
     */
    public long clear
    (String dim, int flags)
      throws IOException, IntenseException, AEPException
    {
      return clear(dim, flags, null);
    }

    /**
     * Clear the AEther in which this Participant is registered.
     *
     * @param dim The relative dimension (to be parsed) under which the clear
     * is to be performed (null if the desired target node in the AEther is the
     * same as the Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP clear.
     */
    public long clear
    (String dim, int flags, Condition optionalBlockCondition)
      throws IOException, IntenseException, AEPException
    {
      CompoundDimension compoundDimension = new CompoundDimension(dim);

      return clear(compoundDimension, flags, optionalBlockCondition);
    }

    /**
     * Clear the AEther in which this Participant is registered.
     *
     * @param dim The relative dimension under which the clear is to be
     * performed (null if the desired target node in the AEther is the same as
     * the Participant's node in the AEther).
     * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
     * AEPServer.postFenceBit, AEPServer.notifySelfBit,
     * AEPServer.notifyClientBit.
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP clear.
     */
    public long clear
    (CompoundDimension dim, int flags, Condition optionalBlockCondition)
      throws IOException, AEPException
    {
      checkValidClientAndJoined("AEPClient.Participant.clear");
      client.lock();
      return client.transaction(
        new AEPServer.ClearToken(client.nextSequence(), getId(), dim, flags),
        true, false, optionalBlockCondition
      );
    }

    /**
     * Send an AEP synch token through the server to which this participant's
     * client is connected.  A synch token (as with join, leave, and
     * disconnect) is synchronous, i.e., is guranteed to have been processed by
     * the remote AEPServer.AEther (after tokens with smaller server
     * sequences), prior to an ack being sent to the client.
     *
     * @return The server sequence of the AEP synch.  When this method returns,
     * all tokens with smaller server sequences will have been processed by the
     * remote AEPServer.AEther.
     */
    public long synch
    ()
      throws IOException, AEPException
    {
      return synch((Condition)null);
    }

    /**
     * Send an AEP synch token through the server to which this participant's
     * client is connected.  A synch token (as with join, leave, and
     * disconnect) is synchronous, i.e., is guranteed to have been processed by
     * the remote AEPServer.AEther (after tokens with smaller server
     * sequences), prior to an ack being sent to the client.
     *
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     * @return The server sequence of the AEP synch.  When this method returns,
     * all tokens with smaller server sequences will have been processed by the
     * remote AEPServer.AEther.
     */
    public long synch
    (Condition optionalBlockCondition)
      throws IOException, AEPException
    {
      client.lock();
      return client.transaction(
        new AEPServer.SynchToken(client.nextSequence()), true,
        false, optionalBlockCondition
      );
    }

    /**
     * Wait for a given server sequence to be seen by this participant's
     * client.  If the same sequence, or a greater sequence, has already been
     * seen, the method returns immediately.
     *
     * @param The server sequence to wait for.  When this method returns,
     * all tokens with smaller server sequences will have been processed by the
     * remote AEPServer.AEther.
     */
    public void synch
    (long serverSequence)
      throws AEPException
    {
      synch(serverSequence, null);
    }

    /**
     * Wait for a given server sequence to be seen by this participant's
     * client.  If the same sequence, or a greater sequence, has already been
     * seen, the method returns immediately.
     *
     * @param The server sequence to wait for.  When this method returns,
     * all tokens with smaller server sequences will have been processed by the
     * remote AEPServer.AEther.
     * @param optionalBlockCondition A blocking condition may be supplied, or a
     * new one will be allocated.
     */
    public void synch
    (long serverSequence, Condition optionalBlockCondition)
      throws AEPException
    {
      client.lock();
      if (client.getServerSequence() >= serverSequence) {
        client.unlock();
        return;
      }
      client.serverSequenceBinderMap.sequenceWait(
        serverSequence, false, optionalBlockCondition
      );
      client.unlock();
    }

    private void checkValidClientAndJoined
    (String location)
      throws AEPException
    {
      if (client == null) {
        throw new AEPException(
          "null AEPClient in " + location,
          Log.ERROR
        );
      }
      if (getId() < 0) {
        throw new AEPException(
          "Not joined in " + location,
          Log.ERROR
        );
      }
    }

  }


  protected abstract static class Token
    extends AEPCommon.Token {

    protected AEPClient client;

    protected long serverSequence;

    public static final int VOID = 0;

    public static final int CLIENT_TERMINATE = 1;

    public static final int SERVER_DISCONNECT = 2;

    public static final int NOTIFY = 3;

    public static final int ACK = 4;

    public static final int DENY = 5;

    public static final int ERROR = 6;

    public static String typeStrings[] = {
      "VOID", "CLIENT_TERMINATE", "CLIENT_DISCONNECT", "NOTIFY", "ACK",
      "DENY", "ERROR"
    };

    public static void checkType
    (int type)
      throws AEPException
    {
      if ((type < VOID)||(type > ERROR)) {
        throw new AEPException("Invalid AEPClient.Token type " + type);
      }
    }

    public abstract int getType
    ();

    String getTypeString
    ()
    {
      return typeStrings[getType()];
    }

    static String getTypeString
    (int type)
    {
      return typeStrings[type];
    }

    Token
    (long serverSequence)
    {
      client = null;
      this.serverSequence = serverSequence;
    }

    protected static Token factory
    (int type)
      throws AEPException
    {
      switch (type) {
      case VOID:
        return null;
      case SERVER_DISCONNECT:
        return new ServerDisconnectToken();
      case NOTIFY:
        return new NotifyToken();
      case ACK:
        return new AckToken();
      case DENY:
        return new DenyToken();
      default:
        throw new AEPException("Bad AEPClient.Token type " + type, Log.FATAL);
      }
    }

    public static void factorySerialise
    (Token token, DataOutputStream out)
      throws IOException, AEPException
    {
      int type;

      if (token != null) {
        type = ((AEPClient.Token)token).getType();
      } else {
        type = AEPClient.Token.VOID;
      }
      out.writeInt(type);
      if (token != null) {
        ((AEPClient.Token)token).serialise(out);
      }
    }

    public static Token factoryDeserialise
    (DataInputStream in)
      throws IOException, IntenseException, AEPException
    {
      int type;
      AEPClient.Token token;

      type = in.readInt();
      token = AEPClient.Token.factory(type);
      if (token != null) {
	token.deserialise(in);
      }
      return token;
    }

    public void setClient
    (AEPClient client)
    {
      this.client = client;
    }

    public long getServerSequence
    ()
    {
      return serverSequence;
    }

  }


  public static class ClientTerminateToken
    extends Token {

    String message;

    public ClientTerminateToken
    (String message)
    {
      super(-1);
      this.message = message;
    }

    public int getType
    ()
    {
      return CLIENT_TERMINATE;
    }

    /**
     * Outbound serialisation, compatable with the C++ libintense-serial
     * XDRBaseSerialiser.
     */
    public void serialise
    (DataOutputStream out)
      throws IOException, AEPException
    {
      throw new AEPException(
        "INTERNAL ERROR: ClientTerminateToken.serialise() called", Log.FATAL
      );
    }

    /**
     * Inbound serialisation, compatable with the C++ libintense-serial
     * XDRBaseSerialiser.
     */
    public void deserialise
    (DataInputStream in)
      throws IOException, AEPException
    {
      throw new AEPException(
        "INTERNAL ERROR: ClientTerminateToken.deserialise() called", Log.FATAL
      );
    }

    public boolean handle
    ()
      throws IOException, IntenseException, AEPException
    {
      if (!client.usingReceiverThread) {
        throw new AEPException(
          "INTERNAL ERROR: usingReceiverThread false in " +
          "ClientTerminateToken.handle()", Log.FATAL
        );
      }
      client.stopped = true;
      client.clientSequenceBinderMap.resumeAll();
      return false; 
    }

    public void aetpPrint
    (PrintStream os)
      throws IOException, AEPException
    {
      throw new AEPException(
        "INTERNAL ERROR: ClientTerminateToken.aetpPrint() called", Log.FATAL
      );
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      os.print("AEPClient::ClientTerminateToken");
      if (message != null) {
        os.print(": " + message);
      }
    }

  }


  public static class ServerDisconnectToken
    extends Token {

    public ServerDisconnectToken
    ()
    {
      super(-1);
    }

    public ServerDisconnectToken
    (long serverSequence)
    {
      super(serverSequence);
    }

    public int getType
    ()
    {
      return SERVER_DISCONNECT;
    }

    /**
     * Outbound serialisation, compatable with the C++ libintense-serial
     * XDRBaseSerialiser.
     */
    public void serialise
    (DataOutputStream out)
      throws IOException, AEPException
    {
      out.writeLong(serverSequence);
    }

    /**
     * Inbound serialisation, compatable with the C++ libintense-serial
     * XDRBaseSerialiser.
     */
    public void deserialise
    (DataInputStream in)
      throws IOException, AEPException
    {
      serverSequence = in.readLong();
    }

    public boolean handle
    ()
      throws IOException, IntenseException, AEPException
    {
      client.stopped = true;
      client.clientSequenceBinderMap.resumeAll();
      return false;
    }

    public void aetpPrint
    (PrintStream os)
      throws IOException, AEPException
    {
      os.print("DISCONNECT " + serverSequence + ";\n\n");
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      os.print(
        "AEPClient::ServerDisconnectToken(server:" + serverSequence + ")"
      );
    }

  }


  public static class NotifyToken
    extends Token implements Origin {


    public abstract static class Target {

      public static final int ASSIGN = 0;

      public static final int APPLY = 1;

      public static final int CLEAR = 2;

      public static final int KICK = 3;

      public static String typeStrings[] = {
        "ASSIGN", "APPLY", "CLEAR", "KICK"
      };

      protected long participantId;

      public static void checkType
      (int type)
        throws AEPException
      {
        if ((type < ASSIGN)||(type > KICK)) {
          throw new AEPException("Invalid NotifyToken.Target type " + type);
        }
      }

      public abstract int getType
      ();

      public String getTypeString
      ()
      {
        return typeStrings[getType()];
      }

      public static String getTypeString
      (int type)
      {
        return typeStrings[type];
      }

      /**
       * Outbound serialisation, compatable with the C++ libintense-serial
       * XDRBaseSerialiser.
       */
      public abstract void serialise
      (DataOutputStream out)
        throws IOException, AEPException;

      /**
       * Inbound serialisation, compatable with the C++ libintense-serial
       * XDRBaseSerialiser.
       */
      public abstract void deserialise
      (DataInputStream in)
        throws IOException, IntenseException, AEPException;

      public abstract void print
      (PrintStream os, int logLevel)
        throws IOException;

      public String toString
      ()
      {
        try {
          ByteArrayOutputStream bytes = new ByteArrayOutputStream();
          PrintStream os = new PrintStream(bytes);

          print(os, Log.DEBUG);
          os.flush();
          return bytes.toString();
        } catch (IOException e) {
          return "ERROR during call to Target.toString(): " + e.getMessage();
        }
      }

      public abstract void aetpPrint
      (PrintStream os)
        throws IOException, AEPException;

      public abstract void handle
      (NotifyToken token)
        throws IntenseException, AEPException;

      public static Target factory
      (int type)
        throws AEPException
      {
        switch (type) {
        case ASSIGN:
          return new AssignTarget();
        case APPLY:
          return new ApplyTarget();
        case CLEAR:
          return new ClearTarget();
        case KICK:
          return new KickTarget();
        default:
          throw new AEPException(
            "Bad AEPClient.NotifyToken.Target type " + type
          );
        }
      }

      public static void factorySerialise
      (Target target, DataOutputStream os)
        throws IOException, AEPException
      {
        int type;

        if (target != null) {
          type = (target).getType();
          if ((type < ASSIGN)||(type > KICK)) {
            throw new AEPException(
              "INTERNAL ERROR - Bad outgoing NotifyToken.Target type " + type,
              Log.FATAL
            );
          }
          os.writeInt(type);
          target.serialise(os);
        } else {
          throw new AEPException(
            "null target in AEPClient.NotifyToken.Target.factorySerialise",
            Log.FATAL
          );
        }
      }

      public static Target factoryDeserialise
      (DataInputStream is)
        throws IOException, IntenseException, AEPException
      {
        int type;
        AEPClient.NotifyToken.Target target;

        type = is.readInt();
        target = AEPClient.NotifyToken.Target.factory(type);
        target.deserialise(is);
        return target;
      }

      protected Target
      (long participantId)
      {
        this.participantId = participantId;
      }

      private Target
      ()
      {}

    }


    public static class AssignTarget
      extends Target {

      private int contextNodeIndex;

      private CompoundDimension dimension;

      private boolean dimensionIsExternal;

      private AEPCommon.Token assignToken;

      public AssignTarget
      ()
      {
        super(-1);
        this.contextNodeIndex = -1;
        this.dimension = null;
        this.dimensionIsExternal = false;
        this.assignToken = null;
      }

      public AssignTarget
        (long participantId, int contextNodeIndex)
      {
        super(participantId);
        this.contextNodeIndex = contextNodeIndex;
        this.dimension = null;
        this.dimensionIsExternal = false;
        this.assignToken = null;
      }

      public AssignTarget
      (long participantId, int contextNodeIndex, CompoundDimension dimension,
       boolean dimensionIsExternal)
      {
        super(participantId);
        this.contextNodeIndex = contextNodeIndex;
        this.dimension = dimension;
        this.dimensionIsExternal = dimensionIsExternal;
        this.assignToken = null;
      }

      public void setAssignToken
      (AEPCommon.Token assignToken)
      {
        this.assignToken = assignToken;
      }

      public void serialise
      (DataOutputStream os)
        throws IOException, AEPException
      {
        int haveDimension = (dimension != null) ? 1 : 0;

        os.writeLong(participantId);
        os.writeInt(contextNodeIndex);
        os.writeInt(haveDimension);
        if (dimension != null) {
          int dimensionIsExternalInt = (dimensionIsExternal) ? 1 : 0;

          dimension.serialise(os);
          os.writeInt(dimensionIsExternalInt);
        }
      }

      public void deserialise
      (DataInputStream is)
        throws IOException, IntenseException, AEPException
      {
        int haveDimension;

        participantId = is.readLong();
        contextNodeIndex = is.readInt();
        haveDimension = is.readInt();
        if (haveDimension != 0) {
          int dimensionIsExternalInt;

          dimension = new CompoundDimension();
          dimension.deserialise(is);
          dimensionIsExternalInt = is.readInt();
          dimensionIsExternal = (dimensionIsExternalInt != 0) ? true : false;
        } else {
          dimension = null;
          dimensionIsExternal = false;
        }
      }

      public void print
      (PrintStream os, int logLevel)
        throws IOException
      {
        os.print("AssignTarget(participant:" + participantId + ", dimension:");
        if (dimension != null) {
          os.print(dimension.canonical() + ", external:");
          if (dimensionIsExternal) {
            os.print('t');
          } else {
            os.print('f');
          }
        } else {
          os.print('-');
        }
        if (logLevel >= Log.DEBUG2) {
          os.print(", node:" + contextNodeIndex);
        }
        os.print(')');
      }

      public void aetpPrint
      (PrintStream os)
        throws IOException, AEPException
      {
        os.print("ASSIGN " + participantId + ' ' + contextNodeIndex);
        if (dimension != null) {
          os.print(" DIM " + dimension.canonical());
          if (dimensionIsExternal) {
            os.print(" EXT");
          } else {
            os.print(" INT");
          }
        } else {
          os.print(" ROOT");
        }
        os.print('\n');
      }

      public void handle
      (NotifyToken token)
        throws IntenseException, AEPException
      {
        String location = "AEPClient.NotifyToken.AssignTarget.handle";

        Participant participant = (Participant)token.client.getParticipant(
          participantId, location
        );

        if ((contextNodeIndex < 0)||(contextNodeIndex >= token.nodes.size())) {
          throw new AEPException(
            "Bad contextNodeIndex " + contextNodeIndex + " in " + location
          );
        }
        if (dimension == null) { 
          participant.assignNotify(
            token.getContext(contextNodeIndex, null), null, token
          );
        } else if (dimensionIsExternal) {
          participant.assignNotify(
            token.getContext(contextNodeIndex, null), dimension, token
          );
        } else {
          participant.assignNotify(
            token.getContext(contextNodeIndex, dimension), null, token
          );
        }
      }

      public int getType
      ()
      {
        return ASSIGN;
      }

    }


    public static class ApplyTarget
      extends Target {

      private int opNodeIndex;

      private CompoundDimension dimension;

      private boolean dimensionIsExternal;

      private AEPCommon.Token applyToken;

      public ApplyTarget
      ()
      {
        super(-1);
        this.opNodeIndex = -1;
        this.dimension = null;
        this.dimensionIsExternal = false;
        this.applyToken = null;
      }

      public ApplyTarget
      (long participantId, int opNodeIndex)
      {
        super(participantId);
        this.opNodeIndex = opNodeIndex;
        this.dimension = null;
        this.dimensionIsExternal = false;
        this.applyToken = null;
      }

      public ApplyTarget
      (long participantId, int opNodeIndex, CompoundDimension dimension,
       boolean dimensionIsExternal)
      {
        super(participantId);
        this.opNodeIndex = opNodeIndex;
        this.dimension = dimension;
        this.dimensionIsExternal = dimensionIsExternal;
        this.applyToken = null;
      }

      public void setApplyToken
      (AEPCommon.Token applyToken)
      {
        this.applyToken = applyToken;
      }

      public void serialise
      (DataOutputStream os)
        throws IOException, AEPException
      {
        int haveDimension = (dimension != null) ? 1 : 0;

        os.writeLong(participantId);
        os.writeInt(opNodeIndex);
        os.writeInt(haveDimension);
        if (dimension != null) {
          int dimensionIsExternalInt = (dimensionIsExternal) ? 1 : 0;

          dimension.serialise(os);
          os.writeInt(dimensionIsExternalInt);
        }
      }

      public void deserialise
      (DataInputStream is)
        throws IOException, IntenseException, AEPException
      {
        int haveDimension;

        participantId = is.readLong();
        opNodeIndex = is.readInt();
        haveDimension = is.readInt();
        if (haveDimension != 0) {
          int dimensionIsExternalInt;

          dimension = new CompoundDimension();
          dimension.deserialise(is);
          dimensionIsExternalInt = is.readInt();
          dimensionIsExternal = (dimensionIsExternalInt != 0) ? true : false;
        } else {
          dimension = null;
          dimensionIsExternal = false;
        }
      }

      public void print
      (PrintStream os, int logLevel)
        throws IOException
      {
        os.print("ApplyTarget(participant:" + participantId + ", dimension:");
        if (dimension != null) {
          os.print(dimension.canonical() + ", external:");
          if (dimensionIsExternal) {
            os.print('t');
          } else {
            os.print('f');
          }
        } else {
          os.print('-');
        }
        os.print(", node:" + opNodeIndex + ')');
      }

      public void aetpPrint
      (PrintStream os)
        throws IOException, AEPException
      {
        os.print("APPLY " + participantId + ' ' + opNodeIndex);
        if (dimension != null) {
          os.print(" DIM " + dimension.canonical());
          if (dimensionIsExternal) {
            os.print(" EXT");
          } else {
            os.print(" INT");
          }
        } else {
          os.print(" ROOT");
        }
        os.print('\n');
      }

      public void handle
      (NotifyToken token)
        throws IntenseException, AEPException
      {
        String location = "AEPClient.NotifyToken.ApplyTarget.handle";
        Participant participant = (Participant)token.client.getParticipant(
          participantId, location
        );

        if ((opNodeIndex < 0)||(opNodeIndex >= token.nodes.size())) {
          throw new AEPException(
           "Bad contextNodeIndex " + opNodeIndex + " in " + location,
           Log.ERROR
         );
        }
        if (dimension == null) { 
          participant.applyNotify(
            token.getContextOp(opNodeIndex, null), null, token
          );
        } else if (dimensionIsExternal) {
          participant.applyNotify(
            token.getContextOp(opNodeIndex, null), dimension, token
          );
        } else {
          participant.applyNotify(
            token.getContextOp(opNodeIndex, dimension), null, token
          );
        }
      }

      public int getType
      ()
      {
        return APPLY;
      }

    }


    public static class ClearTarget
      extends Target {

      CompoundDimension dimension;

      public ClearTarget
      ()
      {
        super(-1);
        this.dimension = null;
      }

      public ClearTarget
      (long participantId)
      {
        super(participantId);
        this.dimension = null;
      }

      public ClearTarget
      (long participantId, CompoundDimension dimension)
      {
        super(participantId);
        this.dimension = new CompoundDimension(dimension);
      }

      public void serialise
      (DataOutputStream os)
        throws IOException, AEPException
      {
        int haveDimension = (dimension != null) ? 1 : 0;

        os.writeLong(participantId);
        os.writeInt(haveDimension);
        if (dimension != null) {
          dimension.serialise(os);
        }
      }

      public void deserialise
      (DataInputStream is)
        throws IOException, IntenseException, AEPException
      {
        int haveDimension;

        participantId = is.readLong();
        haveDimension = is.readInt();
        if (haveDimension != 0) {
          dimension = new CompoundDimension();
          dimension.deserialise(is);
        } else {
          dimension = null;
        }
      }

      public void print
      (PrintStream os, int logLevel)
        throws IOException
      {
        os.print("ClearTarget(participant:" + participantId + ", dimension:");
        if (dimension != null) {
          os.print(dimension.canonical());
        } else {
          os.print('-');
        }
        os.print(')');
      }

      public void aetpPrint
      (PrintStream os)
        throws IOException, AEPException
      {
        os.print("CLEAR " + participantId);
        if (dimension != null) {
          os.print(" DIM " + dimension.canonical());
        } else {
          os.print(" ROOT");
        }
        os.print('\n');
      }

      public void handle
      (NotifyToken token)
        throws IntenseException, AEPException
      {
        String location = "AEPClient.NotifyToken.ClearTarget.handle";

        token.client.getParticipant(participantId, location).
          clearNotify(dimension, token);
      }

      public int getType
      ()
      {
        return CLEAR;
      }

    }


    public static class KickTarget
      extends Target {

      public KickTarget
      ()
      {
        super(-1);
      }

      public KickTarget
      (long participantId)
      {
        super(participantId);
      }

      public void serialise
      (DataOutputStream os)
        throws IOException, AEPException
      {
        os.writeLong(participantId);
      }

      public void deserialise
      (DataInputStream is)
        throws IOException, AEPException
      {
        participantId = is.readLong();
      }

      public void print
      (PrintStream os, int logLevel)
        throws IOException
      {
        os.print("KickTarget(participant:" + participantId + ')');
      }

      public void aetpPrint
      (PrintStream os)
        throws IOException, AEPException
      {
        os.print("KICK " + participantId + '\n');
      }

      public void handle
      (NotifyToken token)
        throws IntenseException, AEPException
      {
        String location = "AEPClient.NotifyToken.KickTarget.handle";

        token.client.getParticipant(participantId, location).
          kickNotify(token);
      }

      public int getType
      ()
      {
        return KICK;
      }

    }


    protected Vector<Target> targets;

    protected Vector<Context> nodes;

    public NotifyToken
    ()
    {
      super(-1);
      targets = new Vector<Target>();
      nodes = new Vector<Context>();
    }

    public NotifyToken
    (long serverSequence)
    {
      super(serverSequence);
      targets = new Vector<Target>();
      nodes = new Vector<Context>();
    }

    public Context getContext
    (int nodeIndex, CompoundDimension dimension)
      throws AEPException
    {
      Context node;

      if (nodeIndex >= nodes.size()) {
        throw new AEPException(
          "PROTOCOL ERROR: Attempt to reference notification node " +
          "with index " + nodeIndex + " >= " + nodes.size(),
          Log.ERROR
        );
      } else if ((node = ((Context)nodes.get(nodeIndex))).getType()
                 != Context.CONTEXT) {
        throw new AEPException(
          "INTERNAL ERROR: Attempt to reference notification non-Context " +
          "node with index " + nodeIndex + " as Context",
          Log.ERROR
        );
      }
      if (dimension != null) {
        return node.value(dimension);
      } else {
        return node;
      }
    }

    ContextOp getContextOp
    (int nodeIndex, CompoundDimension dimension)
      throws AEPException
    {
      Context node;

      if (nodeIndex >= nodes.size()) {
        throw new AEPException(
          "PROTOCOL ERROR: Attempt to reference notification node " +
          "with index " + nodeIndex + " >= " + nodes.size(),
          Log.ERROR
        );
      } else if ((node = ((Context)nodes.get(nodeIndex))).getType()
                 != Context.CONTEXTOP) {
        throw new AEPException(
          "INTERNAL ERROR: Attempt to reference notification non-ContextOp " +
          "node with index " + nodeIndex + " as ContextOp",
          Log.ERROR
        );
      }
      if (dimension != null) {
        return (ContextOp)node.value(dimension);
      } else {
        return (ContextOp)node;
      }
    }

    void addTarget
    (Target target)
    {
      targets.add(target);
    }


    public static class AddNodeResult {

      public int nodeIndex;

      public CompoundDimension internalDimension;

      public AddNodeResult
      ()
      {
        nodeIndex = -1;
        internalDimension = null;
      }

    }


    // For use by AEPServer.Participant:
    public void addNode
    (Context node, AddNodeResult result)
      throws IntenseException
    {
      // libintense has been changed to greatly facilitate things here - first
      // propagation, then preorder assignment or application (i.e., the
      // minimal context/op for an operation is always seen first, by a server
      // with participants in multiple places):
      if ((nodes.size() == 0)||
          ((node != nodes.lastElement())&&
           (!(((Context)nodes.lastElement()).isAncestor(node))))) {
        nodes.add(node);
        result.internalDimension = null;
      } else if (node == nodes.lastElement()) {
        result.internalDimension = null;
      } else {
        // We find the internal dimension path from node to nodes.back():
        result.internalDimension = new CompoundDimension();
        ((Context)nodes.lastElement()).
          findPathTo(node, result.internalDimension);
        result.internalDimension = result.internalDimension;
      }
      result.nodeIndex = nodes.size() - 1;
    }

    // For use by AETPClient:
    public void addNode
    (Context node)
    {
      nodes.add(node);
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      os.print(
        "AEPClient.NotifyToken(server:" + serverSequence + ", targets("
      );
      for (int i = 0; i < targets.size(); i++) {
        if (i > 0) {
          os.print(", ");
        }
        targets.get(i).print(os, logLevel);
      }
      os.print(')');
      if (logLevel >= Log.DEBUG1) {
        os.print(", nodes(");
        for (int i = 0; i < nodes.size(); i++) {
          if (i > 0) {
            os.print(", ");
          }
          os.print(((Context)nodes.get(i)).canonical());
        }
        os.print(')');
      }
      os.print(')');
    }

    public void shallowClear
    ()
    {
    }

    public int getType
    ()
    {
      return NOTIFY;
    }

    public void serialise
    (DataOutputStream out)
      throws IOException, AEPException
    {
      int targetsSize = targets.size();
      int nodesSize = nodes.size();

      out.writeLong(serverSequence);
      out.writeInt(targetsSize);
      for (int i = 0; i < targetsSize; i++) {
        Target.factorySerialise(targets.get(i), out);
      }
      out.writeInt(nodesSize);
      for (int i = 0; i < nodesSize; i++) {
        Context.factorySerialise(nodes.get(i), out);
      }
    }

    public void deserialise
    (DataInputStream in)
      throws IOException, IntenseException, AEPException
    {
      int targetsSize = targets.size();
      int nodesSize = nodes.size();

      serverSequence = in.readLong();
      targetsSize = in.readInt();
      for (int i = 0; i < targetsSize; i++) {
        Target target = Target.factoryDeserialise(in);

        targets.add(target);
      }
      nodesSize = in.readInt();
      for (int i = 0; i < nodesSize; i++) {
        Context node = Context.factoryDeserialise(in);

        nodes.add(node);
      }
    }

    public boolean handle
    ()
      throws IOException, IntenseException, AEPException
    {
      Iterator itr = targets.iterator();

      while (itr.hasNext()) {
        ((Target)itr.next()).handle(this);
      }
      return true;
    }

    public void aetpPrint
    (PrintStream os)
      throws IOException, AEPException
    {
      os.print(
        "NOTIFY " + serverSequence + ' ' + targets.size() + ' ' +
        nodes.size() + '\n'
      );
      for (int i = 0; i < targets.size(); i++) {
        ((Target)targets.get(i)).aetpPrint(os);
      }
      for (int i = 0; i < nodes.size(); i++) {
        os.print(((Context)nodes.get(i)).canonical() + '\n');
      }
      os.print(";\n\n");
    }

  }


  public abstract static class ReplyToken
    extends Token {

    protected long clientSequence;

    protected String message;

    public ReplyToken
    ()
    {
      super(-1);
      this.clientSequence = -1;
      this.message = null;
    }

    public ReplyToken
    (long serverSequence, long clientSequence, String message)
    {
      super(serverSequence);
      this.clientSequence = clientSequence;
      this.message = message;
    }

    public int getType
    ()
    {
      return ACK;
    }

    public long getClientSequence
    ()
    {
      return clientSequence;
    }

    public String getMessage
    ()
    {
      return message;
    }

    public boolean handle
    ()
      throws IOException, IntenseException, AEPException
    {
      throw new AEPException(
        "INTERNAL ERROR: " + getTypeString() + " token handle() called.",
        Log.FATAL
      );
    }

    public void serialise
    (DataOutputStream os)
      throws IOException, AEPException
    {
      os.writeLong(serverSequence);
      os.writeLong(clientSequence);
    }

    public void deserialise
    (DataInputStream is)
      throws IOException, AEPException
    {
      serverSequence = is.readLong();
      clientSequence = is.readLong();
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      os.print(
        "AEPClient.ReplyToken(type:" + getTypeString() + ", server:" +
        getServerSequence() + ", client:" + getClientSequence() + ")"
      );
      if (getMessage() != null) {
        os.print(", message: \"" + getMessage() + '"');
      }
      os.print(')');
    }

  }


  public static class AckToken
    extends ReplyToken {

    public AckToken
    ()
    {
      super();
    }

    public AckToken
    (long serverSequence, long clientSequence)
    {
      super(serverSequence, clientSequence, null);
    }

    public AckToken
    (long serverSequence, long clientSequence, String message)
    {
      super(serverSequence, clientSequence, message);
    }

    public int getType
    ()
    {
      return ACK;
    }

    public void serialise
    (DataOutputStream os)
      throws IOException, AEPException
    {
      os.writeLong(serverSequence);
      os.writeLong(clientSequence);
    }

    public void deserialise
    (DataInputStream is)
      throws IOException, AEPException
    {
      serverSequence = is.readLong();
      clientSequence = is.readLong();
    }

    public void aetpPrint
    (PrintStream os)
      throws IOException, AEPException
    {
      os.print("ACK " + serverSequence + ' ' + clientSequence + ";\n\n");
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      os.print(
        "AEPClient.AckToken: (s:" + getServerSequence() + ", c:" +
        getClientSequence() + ")"
      );
      if (getMessage() != null) {
        os.print(": " + getMessage());
      }
    }

  }


  public static class DenyToken
    extends ReplyToken {

    public DenyToken
    ()
    {
      super();
    }

    public DenyToken
    (long serverSequence, long clientSequence)
    {
      super(serverSequence, clientSequence, null);
    }

    public DenyToken
    (long serverSequence, long clientSequence, String message)
    {
      super(serverSequence, clientSequence, message);
    }

    public int getType
    ()
    {
      return DENY;
    }

    public void serialise
    (DataOutputStream os)
      throws IOException, AEPException
    {
      os.writeLong(serverSequence);
      os.writeLong(clientSequence);
    }

    public void deserialise
    (DataInputStream is)
      throws IOException, AEPException
    {
      serverSequence = is.readLong();
      clientSequence = is.readLong();
    }

    public void aetpPrint
    (PrintStream os)
      throws IOException, AEPException
    {
      os.print("DENY " + serverSequence + ' ' + clientSequence + ";\n\n");
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      os.print(
        "AEPClient.DenyToken: (s:" + getServerSequence() + ", c:" +
        getClientSequence() + ")"
      );
      if (getMessage() != null) {
        os.print(": " + getMessage());
      }
    }

  }


  public static class ErrorToken
    extends ReplyToken {

    public ErrorToken
    (String message)
    {
      super(-1, -1, message);
    }

    public ErrorToken
    (long serverSequence, long clientSequence)
    {
      super(serverSequence, clientSequence, null);
    }

    public ErrorToken
    (long serverSequence, long clientSequence, String message)
    {
      super(serverSequence, clientSequence, message);
    }

    public int getType
    ()
    {
      return ERROR;
    }

    public void serialise
    (DataOutputStream os)
      throws IOException, AEPException
    {
      os.writeLong(serverSequence);
      os.writeLong(clientSequence);
    }

    public void deserialise
    (DataInputStream is)
      throws IOException, AEPException
    {
      serverSequence = is.readLong();
      clientSequence = is.readLong();
    }

    public void aetpPrint
    (PrintStream os)
      throws IOException, AEPException
    {
      os.print("ERROR " + serverSequence + ' ' + clientSequence + ";\n\n");
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      os.print(
        "AEPClient::ErrorToken: (s:" + getServerSequence() + ", c:" +
        getClientSequence()+ ')'
      );
      if (getMessage() != null) {
        os.print(": " + getMessage());
      }
    }

  }


  protected static class SequenceBinder {

    protected long sequence;

    protected ReplyToken token;

    protected boolean terminate;

    protected Condition condition;

    public SequenceBinder
    (long sequence, boolean terminate, Condition condition)
    {
      this.sequence = sequence;
      token = null;
      this.terminate = terminate;
      this.condition = condition;
    }

  }


  protected static class SequenceBinderMap
    // public map<long long, SequenceBinder*>
    extends HashMap<Long, SequenceBinder> {

    static final long serialVersionUID = -702588304484581761L;

    protected AEPClient client;

    public SequenceBinderMap
    (AEPClient client)
    {
      super();
      this.client = client;
    }

    /**
     * client.lock() must be acquired prior to call, and is reacquired,
     * after call.
     */
    public ReplyToken sequenceWait
    (long sequence)
    {
      return sequenceWait(sequence, false, null);
    }

    /**
     * client.lock() must be acquired prior to call, and is reacquired,
     * after call.
     */
    public ReplyToken sequenceWait
    (long sequence, boolean terminate, Condition condition)
    {
      ReplyToken returnValue;
      SequenceBinder binder = new SequenceBinder(
        sequence, terminate,
        (condition != null) ? condition : client.newCondition()
      );

      put(new Long(sequence), binder);
      binder.condition.awaitUninterruptibly();
      returnValue = binder.token;
      return returnValue;
    }

    /**
     * client.lock() must be acquired prior to call.
     */
    public boolean resumeAll
    ()
    {
      Iterator itr = keySet().iterator();
      boolean keepRunning = true;

      while (itr.hasNext()) {
        SequenceBinder binder = (SequenceBinder)get(itr.next());

        binder.condition.signal();
        if (binder.terminate) {
          keepRunning = false;
        }
      }
      clear();
      return keepRunning;
    }

    /**
     * client.lock() must be acquired prior to call.
     */
    public boolean resume
    (ReplyToken token)
      throws AEPException
    {
      SequenceBinder binder =
        (SequenceBinder)get(new Long(token.getClientSequence()));
      boolean keepRunning = true;

      if (binder == null) {
        // Really only critical if we care about the server:
        throw new AEPException(
          "PROTOCOL ERROR: SequenceBinder with client sequence " +
          token.getClientSequence() + " not found in client map",
          Log.ERROR
        );
      }
      binder.token = token;
      binder.condition.signal();
      if (binder.terminate) {
        keepRunning = false;
      }
      remove(new Long(token.getClientSequence()));
      return keepRunning;
    }

    /**
     * client.lock() must be acquired prior to call.
     */
    public boolean resumeUntil
    (long sequence)
    {
      Iterator itr = keySet().iterator();
      boolean keepRunning = true;

      while (itr.hasNext()) {
        SequenceBinder binder = (SequenceBinder)get(itr.next());

        if (binder.sequence > sequence) {
          break;
        }
        binder.condition.signal();
        if (binder.terminate) {
          keepRunning = false;
        }
        remove(new Long(binder.sequence));
      }
      return keepRunning;
    }

  }


  // Clients keep their own sequence; Servers get their sequence from their
  // AEther.
  protected long sequence;

  protected long serverSequence;

  protected SequenceBinderMap clientSequenceBinderMap;

  protected SequenceBinderMap serverSequenceBinderMap;

  protected boolean stopped;

  protected Thread earThread;

  protected boolean usingReceiverThread;

  protected Thread receiverThread;

  protected int maxReceiveSize;

  protected LinkedList<Token> receiverQueue;

  protected ReentrantLock receiverQueueMutex;

  protected Condition receiverQueueCondition;

  // Assumes lock has been acquired:
  protected long nextSequence
  ()
  {
    return sequence++;
  }

  /**
   * Essentially instream->peek() in stream client, and condition wait in
   * shared client.  Returns true if lock is held.
   */
  protected abstract boolean blockForAvailableData
  ()
    throws IOException;

  protected abstract void send
  (AEPServer.Token token)
    throws IOException, AEPException;

  protected abstract Token receive
  ()
    throws IOException, IntenseException, AEPException;

  protected AEPClient
  (Log log, int logLevel, boolean errorTolerant, boolean useReceiverThread,
   int maxReceiveSize)
    throws AEPException
  {
    super(log, logLevel, errorTolerant);
    this.sequence = 0;
    this.serverSequence = 0;
    // explicit (non-static) inner class... matches C++ implementation:
    this.clientSequenceBinderMap = new SequenceBinderMap(this);
    this.serverSequenceBinderMap = new SequenceBinderMap(this);
    this.stopped = true;
    this.usingReceiverThread = useReceiverThread;
    this.maxReceiveSize = maxReceiveSize;
    this.receiverQueue = null;
    if (usingReceiverThread) {
      if (maxReceiveSize <= 0) {
        throw new AEPException(
          "ERROR: Invalid maxReceiveSize " + maxReceiveSize +
          " supplied to AEPClient",
          Log.CRITICAL
        );
      }
      receiverQueue = new LinkedList<Token>();
      receiverQueueMutex = new ReentrantLock();
      receiverQueueCondition = receiverQueueMutex.newCondition();
    }
  }

  protected ReplyToken waitForReply
  (long clientSequence, String location, boolean terminate,
   Condition optionalBlockCondition)
    throws AEPException
  {
    ReplyToken reply;

    if ((reply = clientSequenceBinderMap.
         sequenceWait(clientSequence, terminate, condition)) == null) {
      throw new AEPException(
        "ERROR: NULL reply Token in " + location +
        " (perhaps the client was killed with active participants joined)",
        Log.CRITICAL
      );
    }
    return reply;
  }


  protected class EarThread
    extends Thread {

    public void run
    ()
    {
      try {
        earMain();
      } catch (Throwable ignored) {
      }
    }

  }


  protected void earMain
  ()
  {
    boolean running = true;
    Token token = null;
    boolean haveLock;

    if ((log != null)&&(logLevel >= Log.NOTICE)) {
      log.priority(Log.NOTICE).add(
        "AEPClient ear thread " + Thread.currentThread().toString() +
        ": starting execution..."
      ).end();
    }
    try {
      // Wake up the start caller:
      if (usingReceiverThread) {
        receiverQueueMutex.lock();
        receiverQueueCondition.signal();
      } else {
        lock();
        signal();
        unlock();
      }
      while (running) {
        if (usingReceiverThread) {
          while (receiverQueue.size() == 0) {
            // Ear waits for receiver to populate queue:
            receiverQueueCondition.awaitUninterruptibly();
          }
          if (receiverQueue.size() >= maxReceiveSize) {
            // Wake up the receiver:
            receiverQueueCondition.signal();
          }
          token = (Token)receiverQueue.removeFirst();
          receiverQueueMutex.unlock();
          running = processToken(token);
        } else {
          boolean haveReceptionLock = blockForAvailableData();

          if (!haveReceptionLock) {
            if (locksReception()) {
              receptionMutex.lock();
            }
          }
          try {
            if (stopped) {
              running = false;
            } else {
              token = receive();
              if (token == null) {
                throw new AEPException(
                  "INTERNAL ERROR: Received null AEPClient::Token", Log.FATAL
                );
              }
            }
          } finally {
            if (locksReception()) {
              receptionMutex.unlock();
            }
          }
          if (running) {
            running = processToken(token);
          }
        }
      }
    } catch (Exception e) {
      if ((log != null)&&(logLevel >= Log.FATAL)) {
        log.priority(Log.FATAL).add(
          "FATAL ERROR: Caught Exception at top of AEPClient ear thread " +
          Thread.currentThread().toString() + ": " + e.getMessage() +
          "\nTrace:\n\n" + Log.getTrace(e)
        ).end();
      }
    } catch (Throwable t) {
      if ((log != null)&&(logLevel >= Log.FATAL)) {
        log.priority(Log.FATAL).add(
          "FATAL ERROR: Caught Throwable type at top of " +
          "AEPClient ear thread " + Thread.currentThread().toString() +
          "\nTrace:\n\n" + Log.getTrace(t)
        ).end();
      }
    }
    if ((log != null)&&(logLevel >= Log.NOTICE)) {
      log.priority(Log.NOTICE).add(
        "AEPClient ear thread " + Thread.currentThread() + ": END OF LINE."
      ).end();
    }
  }


  protected class ReceiverThread
    extends Thread {

    public void run
    ()
    {
      try {
        receiverMain();
      } catch (Throwable ignored) {
      }
    }

  }


  void receiverMain
  ()
  {
    boolean running = true;

    if ((log != null)&&(logLevel >= Log.NOTICE)) {
      log.priority(Log.NOTICE).add(
        "AEPClient receiver thread " + Thread.currentThread() +
        ": starting execution..."
      ).end();
    }
    while (running) {
      AEPClient.Token token = null;
      boolean haveReceptionLock;

      try {
        haveReceptionLock = blockForAvailableData();
        if (!haveReceptionLock) {
          if (locksReception()) {
            receptionMutex.lock();
          }
        }
      } catch (IOException e) {
        String message = "AEPClient receiver thread caught IOException " +
            "during block for available data:\n\n" + Log.getTrace(e);

        receiverQueue.addLast(new ClientTerminateToken(message));
        if ((log != null)&&(logLevel >= Log.ERROR)) {
          log.priority(Log.ERROR).add(message).end();
        }
        return;
      }
      if (stopped) {
        running = false;
      } else {
        try {
          token = receive();
          if (token == null) {
            throw new AEPException(
              "INTERNAL ERROR: Received NULL AEPClient.Token", Log.FATAL
            );
          }
        } catch (Exception e) {
          String message = "AEPClient receiver thread caught exception: " +
            e.getMessage() + "\nTrace:\n\n" + Log.getTrace(e);
          receiverQueue.addLast(new ClientTerminateToken(message));
          if ((log != null)&&(logLevel >= Log.ERROR)) {
            log.priority(Log.ERROR).add(message).end();
          }
          running = false;
        } catch (Throwable t) {
          String message = "AEPClient receiver thread caught unknown " +
            "exception\nTrace:\n\n" + Log.getTrace(t);

          receiverQueue.addLast(new ClientTerminateToken(message));
          if ((log != null)&&(logLevel >= Log.ERROR)) {
            log.priority(Log.ERROR).add(message).end();
          }
          running = false;
        }
      }
      if (locksReception()) {
        receptionMutex.unlock();
      }
      if (running) {
        receiverQueueMutex.lock();
        receiverQueue.addLast(token);
        // Wake up the ear, if it's waiting:
        receiverQueueCondition.signal();
        while (receiverQueue.size() >= maxReceiveSize) {
          // We wait for the ear to signal us that it has processed the queue,
          // and we log the "overflow":
          if ((log != null)&&(logLevel >= Log.WARN)) {
            log.priority(Log.WARN).add(
              "Maximum receiver queue size (" + maxReceiveSize +
              ") reached... suspending receiver..."
            ).end();
          }
          receiverQueueCondition.awaitUninterruptibly();
          if ((log != null)&&(logLevel >= Log.WARN)) {
            log.priority(Log.WARN).add("Receiver resumed.").end();
          }
        }
        receiverQueueMutex.unlock();
      }
    }
  }

  boolean processToken
  (Token token)
    throws IOException, IntenseException, AEPException
  {
    boolean keepRunning = false;

    lock();
    // AEPExceptions thrown during token handling may not terminate the
    // session, if the client is error tolerant:
    try {
      token.setClient(this);
      if ((log != null)&&(logLevel >= Log.DEBUG)) {
        log.priority(Log.DEBUG).add("AEPClient: Received token: ");
        token.print(log, logLevel);
        log.end();
      }
      try {
        serverSequence = token.getServerSequence();
        switch (token.getType()) {
        case AEPClient.Token.VOID:
          throw new AEPException(
            "INTERNAL ERROR: Received VOID AEPClient.Token", Log.FATAL
          );
        case AEPClient.Token.ACK:
        case AEPClient.Token.DENY:
        case AEPClient.Token.ERROR:
          keepRunning = clientSequenceBinderMap.resume((ReplyToken)token);
          serverSequenceBinderMap.
            resumeUntil(((ReplyToken)token).getServerSequence());
          break;
        case AEPClient.Token.CLIENT_TERMINATE:
        case AEPClient.Token.SERVER_DISCONNECT:
        case AEPClient.Token.NOTIFY:
          keepRunning = token.handle();
          break;
        default:
          {
            throw new AEPException(
              "INTERNAL ERROR: Got bad AEPClient.Token type int " +
              token.getType(),
              Log.FATAL
            );
          }
        }
      } catch (AEPException e) {
        if ((log != null)&&(logLevel >= e.getPriority())) {
          log.priority(e.getPriority()).add(
            "Caught AEPException in processing of AEPClient.Token: " +
            e.getMessage() + "\nTrace:\n\n" + Log.getTrace(e)
          ).end();
        }
        if ((!errorTolerant)||(e.getPriority() <= Log.FATAL)) {
          throw e;
        }
      } catch (IntenseException e) {
        if ((log != null)&&(logLevel >= Log.CRITICAL)) {
          log.priority(Log.CRITICAL).add(
            "Caught IntenseException in processing of AEPClient.Token: " +
            e.getMessage() + "\nTrace:\n\n" + Log.getTrace(e)
          ).end();
        }
        throw new AEPException(e);
      } catch (Exception e) {
        if ((log != null)&&(logLevel >= Log.CRITICAL)) {
          log.priority(Log.CRITICAL).add(
            "Caught std.exception in processing of AEPClient.Token: " +
            e.getMessage() + "\nTrace:\n\n" + Log.getTrace(e)
          ).end();
        }
        throw new AEPException(e);
      } catch (Throwable t) {
        if ((log != null)&&(logLevel >= Log.CRITICAL)) {
          log.priority(Log.CRITICAL).add(
            "Caught unknown object in processing of AEPClient.Token" +
            "\nTrace:\n\n" + Log.getTrace(t)
          ).end();
        }
        throw new AEPException(new Exception(t));
      }
      if (stopped) {
        keepRunning = false;
      }
      return keepRunning;
    } finally {
      unlock();
    }
  }

  /**
   * Assumes client has been locked and unlocks it before returning.
   */
  private long transaction
  (AEPCommon.Token outgoingToken, boolean allowDeny, boolean terminate,
   Condition blockCondition)
    throws IOException, AEPException
  {
    long serverSequence = -1;
    AEPServer.Token outgoing = (AEPServer.Token)outgoingToken;
    Token reply = null;
    String typeString = outgoing.getTypeString();
    long clientSequence = outgoing.getClientSequence();

    try {
      if ((log != null)&&(logLevel >= Log.DEBUG1)) {
        log.priority(Log.DEBUG1).add("AEPClient.transaction: sending: ");
        outgoing.print(log, logLevel);
        log.end();
      }
      send(outgoing);
      if ((log != null)&&(logLevel >= Log.DEBUG1)) {
        log.priority(Log.DEBUG1).add("AEPClient.transaction: ").
          add("token sent; waiting for reply...").end();
      }
      reply =
        waitForReply(clientSequence, typeString, terminate, blockCondition);
      if ((log != null)&&(logLevel >= Log.DEBUG1)) {
        log.priority(Log.DEBUG1).
          add("AEPClient.transaction: received reply: ");
        reply.print(log, logLevel);
        log.end();
      }
      switch (reply.getType()) {
      case Token.ACK:
        // Success:
        serverSequence = ((AckToken)reply).getServerSequence();
        break;
      case Token.DENY:
        if (allowDeny) {
          String message = ((DenyToken)reply).getMessage();
          StringBuffer buffer = new StringBuffer();

          buffer.append(typeString + " DENIED");
          if (message != null) {
            buffer.append(": " + message);
          }
          throw new AEPException(buffer.toString(), Log.WARN);
        }
        // else fall through:
      case Token.ERROR:
        {
          String message = ((ErrorToken)reply).getMessage();
          StringBuffer buffer = new StringBuffer();

          buffer.append(typeString + " ERROR");
          if (message != null) {
            buffer.append(": " + message);
          }
          throw new AEPException(buffer.toString(), Log.ERROR);
        }
      default:
        {
          throw new AEPException(
            "INTERNAL ERROR: Bad Token type " + reply.getTypeString() +
            " in " + typeString,
            Log.FATAL
          );
        }
      }
      reply = null;
    } finally {
      unlock();
    }
    return serverSequence;
  }

  // Direct operations on the remote aether, with no use of participants.
  // These have the same effect as assign, apply and clear operations on
  // a root-node participant:

  /**
   * Assign a Context to the AEther to which this client is connected.
   *
   * @param context The Context to assign to the AEther.
   * @return The server sequence of the AEP assign.
   */
  public long assign
  (Context context)
    throws IOException, IntenseException, AEPException
  {
    return assign(context, null, 0, null);
  }

  /**
   * Assign a Context to the AEther to which this client is connected.
   *
   * @param context The Context to assign to the AEther.
   * @return The server sequence of the AEP assign.
   * @param dim The relative dimension to which the Context is assigned (null
   * if the desired target node in the AEther is the same as the client's root
   * node in the AEther).
   * @return The server sequence of the AEP assign.
   */
  public long assign
  (Context context, CompoundDimension dim)
    throws IOException, IntenseException, AEPException
  {
    return assign(context, dim, 0, null);
  }

  /**
   * Assign a Context to the AEther to which this client is connected.
   *
   * @param context The Context to assign to the AEther.
   * @return The server sequence of the AEP assign.
   * @param dim The relative dimension to which the Context is assigned (null
   * if the desired target node in the AEther is the same as the client's root
   * node in the AEther).
   * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
   * AEPServer.postFenceBit, AEPServer.notifySelfBit,
   * AEPServer.notifyClientBit.
   * @return The server sequence of the AEP assign.
   */
  public long assign
  (Context context, CompoundDimension dim, int flags)
    throws IOException, IntenseException, AEPException
  {
    return assign(context, dim, flags, null);
  }

  /**
   * Assign a Context to the AEther to which this client is connected.
   *
   * @param context The Context to assign to the AEther.
   * @param dim The relative dimension to which the Context is assigned (null
   * if the desired target node in the AEther is the same as the client's root
   * node in the AEther).
   * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
   * AEPServer.postFenceBit, AEPServer.notifySelfBit,
   * AEPServer.notifyClientBit.
   * @param optionalBlockCondition A blocking condition may be supplied, or a
   * new one will be allocated.
   * @return The server sequence of the AEP assign.
   */
  public long assign
  (Context context, CompoundDimension dim, int flags, Condition blockCondition)
    throws IOException, IntenseException, AEPException
  {
    lock();
    return transaction(
      new AEPServer.AssignToken(nextSequence(), -1, context, dim, 0),
      true, false, blockCondition
    );
  }

  /**
   * Apply a ContextOp to the AEther to which this client is connected.
   *
   * @param context The ContextOp to apply to the AEther.
   * @return The server sequence of the AEP apply.
   */
  public long apply
  (ContextOp op)
    throws IOException, IntenseException, AEPException
  {
    return apply(op, null, 0, null);
  }

  /**
   * Apply a ContextOp to the AEther to which this client is connected.
   *
   * @param context The ContextOp to apply to the AEther.
   * @param dim The relative dimension to which the ContextOp is applied (null
   * if the desired target node in the AEther is the same as the client's root
   * node in the AEther).
   * @return The server sequence of the AEP apply.
   */
  public long apply
  (ContextOp op, CompoundDimension dim)
    throws IOException, IntenseException, AEPException
  {
    return apply(op, dim, 0, null);
  }

  /**
   * Apply a ContextOp to the AEther to which this client is connected.
   *
   * @param context The ContextOp to apply to the AEther.
   * @param dim The relative dimension to which the ContextOp is applied (null
   * if the desired target node in the AEther is the same as the client's root
   * node in the AEther).
   * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
   * AEPServer.postFenceBit, AEPServer.notifySelfBit,
   * AEPServer.notifyClientBit.
   * @return The server sequence of the AEP apply.
   */
  public long apply
  (ContextOp op, CompoundDimension dim, int flags)
    throws IOException, IntenseException, AEPException
  {
    return apply(op, dim, flags, null);
  }

  /**
   * Apply a ContextOp to the AEther to which this client is connected.
   *
   * @param context The ContextOp to apply to the AEther.
   * @param dim The relative dimension to which the ContextOp is applied (null
   * if the desired target node in the AEther is the same as the client's root
   * node in the AEther).
   * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
   * AEPServer.postFenceBit, AEPServer.notifySelfBit,
   * AEPServer.notifyClientBit.
   * @param optionalBlockCondition A blocking condition may be supplied, or a
   * new one will be allocated.
   * @return The server sequence of the AEP apply.
   */
  public long apply
  (ContextOp op, CompoundDimension dim, int flags, Condition blockCondition)
    throws IOException, IntenseException, AEPException
  {
    lock();
    return transaction(
      new AEPServer.ApplyToken(nextSequence(), -1, op, dim, flags),
      true, false, blockCondition
    );
  }

  /**
   * Clear the AEther to which this client is connected.
   *
   * @param dim The relative dimension under which the clear is to be performed
   * (null if the desired target node in the AEther is the same as the client's
   * root node in the AEther).
   * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
   * AEPServer.postFenceBit, AEPServer.notifySelfBit,
   * AEPServer.notifyClientBit.
   * @param optionalBlockCondition A blocking condition may be supplied, or a
   * new one will be allocated.
   * @return The server sequence of the AEP clear.
   */
  public long clear
  ()
    throws IOException, AEPException
  {
    return clear(null, 0, null);
  }

  /**
   * Clear the AEther to which this client is connected.
   *
   * @param dim The relative dimension under which the clear is to be performed
   * (null if the desired target node in the AEther is the same as the client's
   * root node in the AEther).
   * @return The server sequence of the AEP clear.
   */
  public long clear
  (CompoundDimension dim)
    throws IOException, AEPException
  {
    return clear(dim, 0, null);
  }

  /**
   * Clear the AEther to which this client is connected.
   *
   * @param dim The relative dimension under which the clear is to be performed
   * (null if the desired target node in the AEther is the same as the client's
   * root node in the AEther).
   * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
   * AEPServer.postFenceBit, AEPServer.notifySelfBit,
   * AEPServer.notifyClientBit.
   * @return The server sequence of the AEP clear.
   */
  public long clear
  (CompoundDimension dim, int flags)
    throws IOException, AEPException
  {
    return clear(dim, flags, null);
  }

  /**
   * Clear the AEther to which this client is connected.
   *
   * @param dim The relative dimension under which the clear is to be performed
   * (null if the desired target node in the AEther is the same as the client's
   * root node in the AEther).
   * @param flags Optional bit flags may be supplied: AEPServer.preFenceBit,
   * AEPServer.postFenceBit, AEPServer.notifySelfBit,
   * AEPServer.notifyClientBit.
   * @param optionalBlockCondition A blocking condition may be supplied, or a
   * new one will be allocated.
   * @return The server sequence of the AEP clear.
   */
  public long clear
  (CompoundDimension dim, int flags, Condition blockCondition)
    throws IOException, AEPException
  {
    lock();
    return transaction(
      new AEPServer.ClearToken(nextSequence(), -1, dim, flags),
      true, false, blockCondition
    );
  }

  /**
   * Send an AEP synch token through the server to which this client is
   * connected.  A synch token (as with join, leave, and disconnect) is
   * synchronous, i.e., is guranteed to have been processed by the remote
   * AEPServer.AEther (after tokens with smaller server sequences), prior to an
   * ack being sent to the client.
   *
   * @param optionalBlockCondition A blocking condition may be supplied, or a
   * new one will be allocated.
   * @return The server sequence of the AEP synch.  When this method returns,
   * all tokens with smaller server sequences will have been processed by the
   * remote AEPServer.AEther.
   */
  public long synch
  (Condition blockCondition)
    throws IOException, AEPException
  {
    lock();
    return transaction(
      new AEPServer.SynchToken(nextSequence()), true, false, blockCondition
    );
  }

  /**
   * Wait for a given server sequence to be seen by this client.  If the same
   * sequence, or a greater sequence, has already been seen, the method
   * returns immediately.
   *
   * @param serverSequence The server sequence to wait for.  When this method
   * returns, all tokens with smaller server sequences will have been
   * processed by the remote AEPServer.AEther.
   * @param optionalBlockCondition A blocking condition may be supplied, or a
   * new one will be allocated.
   */
  public void synch
  (long serverSequence, Condition blockCondition)
    throws IOException, AEPException
  {
    lock();
    serverSequenceBinderMap.sequenceWait(
      serverSequence, false, blockCondition
    );
    unlock();
  }

  /**
   * Obtian the last server sequence number seen by this client's ear thread.
   *
   * @return The last server sequence seen.
   */
  public long getServerSequence
  ()
  {
    return serverSequence;
  }

  /**
   * Disconnet this client.
   *
   * @return The server sequence of the AEP disconnect.
   */
  public long disconnect
  ()
    throws IOException, AEPException
  {
    return disconnect(null);
  }

  /**
   * Disconnet this client.
   *
   * @param optionalBlockCondition A blocking condition may be supplied, or a
   * new one will be allocated.
   * @return The server sequence of the AEP disconnect.
   */
  public long disconnect
  (Condition blockCondition)
    throws IOException, AEPException
  {
    lock();
    return transaction(
      new AEPServer.ClientDisconnectToken(nextSequence()), true,
      true, blockCondition
    );
  }

  /**
   * Start this client's ear thread.
   */
  public void start
  ()
    throws IOException, AEPException
  {
    lock();
    if (!stopped) {
      unlock();
      return;
    }
    stopped = false;
    if (usingReceiverThread) {
      receiverQueueMutex.lock();
      earThread = new EarThread();
      try {
        earThread.start();
      } catch (IllegalThreadStateException e) {
        stopped = true;
        throw new AEPException(
          "Could not create AEPClient ear thread", e
        );
      }
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEPClient.start created ear thread " + earThread + "."
        ).end();
      }
      receiverQueueCondition.awaitUninterruptibly();
      // Ear is now waiting on receiverCondition in aep()...
      receiverThread = new ReceiverThread();
      try {
        receiverThread.start();
      } catch (IllegalThreadStateException e) {
        stopped = true;
        throw new AEPException(
          "Could not create AEPClient receiver thread", e
        );
      }
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEPClient::start created receiver thread " + receiverThread + "."
        ).end();
      }
      receiverQueueMutex.unlock();
    } else {
      earThread = new EarThread();
      try {
        earThread.start();
      } catch (IllegalThreadStateException e) {
        stopped = true;
        throw new AEPException(
          "Could not create AEPClient ear thread", e
        );
      }
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEPClient.start created ear thread " + earThread + "."
        ).end();
      }
      condWait();
    }
    unlock();
  }

  /**
   * Stop this client's ear thread.
   */
  public void stop
  ()
    throws InterruptedException, IOException, AEPException
  {
    boolean runningFlag;

    if (locksReception()) {
      receptionMutex.lock();
    }
    lock();
    sendLock();
    if (stopped) {
      unlock();
      return;
    }
    stopped = true;
    unlock();
    if (locksReception()) {
      receptionMutex.unlock();
    }
    if (usingReceiverThread) {
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEPClient::stop cancelling and joining receiver thread " +
          receiverThread + "..."
        ).end();
      }
      closeConnection();
      receiverThread.join();
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEPClient::stop joined receiver thread " + receiverThread +
          "; sending ClientTerminateToken to and joining ear thread " +
          earThread + "..."
        ).end();
      }
      receiverQueue.addLast(new ClientTerminateToken(
        "Terminated via AEPClient::stop()"
      ));
      // Wait for the ear to finish running:
      earThread.join();
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEPClient::stop joined ear thread " + earThread + "."
        ).end();
      }
    } else {
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEPClient.stop cancelling and joining ear thread " + earThread +
          "..."
        ).end();
      }
      closeConnection();
      earThread.join();
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEPClient::stop joined ear thread " + earThread + "."
        ).end();
      }
    }
    sendUnlock();
  }

  protected abstract void closeConnection
  ()
    throws IOException;

}
