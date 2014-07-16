// ****************************************************************************
//
// AEther.java - Contexts with participants.
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
 * Contexts with participants.
 */
public class AEther
  extends Context {

   static final long serialVersionUID = -6922287485892859354L;

  /**
   * A list of Participant objects for this node.
   */
  LinkedList<AEtherParticipant> participants;

  /**
   * The total number of Participants connected at this point in the AEther
   * or beneath it.
   */
  int headcount;

  /**
   * Initialization constructor for internal use.
   *
   * @param parent The parent AEther of this AEther.
   * @param parentDim The dimension in the parent AEther under which this
   * AEther is located.
   */
  protected AEther
  (AEther parent, Dimension parentDim)
  {
    super(parent, parentDim);
    participants = new LinkedList<AEtherParticipant>();
    headcount = 0;
  }

  /**
   * Overridden new-AEther/Context constructor for operator[] helper.
   *
   * @param dimension under which the new AEther is to be created.
   * @return A reference to the new AEther.
   */
  protected Context valueRefFactory
  (Dimension dim)
  {
    return new AEther(this, dim);
  }

  /**
   * Get the total number of participants at this AEther node and in children
   * of this node.
   *
   * @return The headcount.
   */
  public int headCount
  ()
  {
    return headcount;
  }

  /**
   * Void constructor.
   */
  public AEther
  ()
  {
    super();
    parentDim = null;
    participants = new LinkedList<AEtherParticipant>();
    headcount = 0;
  }

  /**
   * Parse constructor.
   *
   * @param contextString A canonical-form context string to parse.
   * @throws IntenseException if a parse error occurs.
   */
  public AEther
  (String contextString)
    throws IntenseException
  {
    super();
    parentDim = null;
    participants = new LinkedList<AEtherParticipant>();
    headcount = 0;
    parse(contextString);
  }

  /**
   * Get the type of this node (compatability with C++ implementation).
   *
   * @return The type of the node (Currently, VOID, CONTEXT, CONTEXTOP, or
   * AETHER).
   */
  public int getType
  ()
  {
    return AETHER;
  }

  /**
   * Attach a given AEtherParticipant to an AEther.
   *
   * @param p The AEtherParticipant to be attached.
   */
  public void attach
  (AEtherParticipant p)
  {
    AEther aetherMarker;

    for (aetherMarker = this; aetherMarker != null;
         aetherMarker = (AEther)(aetherMarker.parent)) {
      aetherMarker.headcount++;
    }
    participants.add(p);
  }

  /**
   * Detach a given AEtherParticipant from this AEther (ie at this level only).
   *
   * @param p The AEtherParticipant to be detached.
   */
  public void detach
  (AEtherParticipant p)
  {
    AEther aetherMarker;

    participants.remove(p);
    for (aetherMarker = this; aetherMarker != null;
         aetherMarker = (AEther)(aetherMarker.parent)) {
      aetherMarker.headcount--;
    }
  }

  /**
   * For use by applyHelper, to clear a dimension behind a given iterator.  In
   * a subclass such as AEther, this can be overridden, to avoid wiping out
   * certain dimensions (ie those with a headcount, for AEthers).
   *
   * @param itr The Iterator to delete the current element of and increment.
   * @param origin An object denoting the source of the operation, null by
   * default.
   * @return The Pair pointed to by the value after the current value of itr.
   */
  protected void checkedClear
  (Pair pair, Origin origin)
    throws IntenseException
  {
    if (((AEther)(pair.context)).headcount > 0) {
      ((AEther)pair.context).clearNoPropagate(origin);
    } else {
      delete(pair);
    }
  }

  /**
   * For use by clear(dim), to empty a dimension behind a given iterator.
   * In a subclass such as AEther, this can be overridden, to avoid wiping
   * out certain dimensions (ie those with a headcount, for AEthers).
   *
   * @param itr The Iterator to delete the current element of and increment.
   * @param origin An object denoting the source of the operation, null by
   * default.
   * @return The Pair pointed to by the value after the current value of itr.
   */
  protected void checkedClearAndPropagateNotify
  (Pair pair, Origin origin)
    throws IntenseException
  {
    int initialBasecount = pair.context.basecount;

    if (((AEther)pair.context).headcount > 0) {
      ((AEther)pair.context).clearNoPropagate(origin);
      if (initialBasecount != 0) {
        ((AEther)pair.context).propagateClearNotify(origin);
      }
    } else {
      if (initialBasecount != 0) {
        pair.context.clear();
        pair.context.base = null;
        pair.context.basecount = 0;
        ((AEther)pair.context).propagateClearNotify(origin);
      }
      delete(pair);
    }
  }

  /**
   * Assign the contents of another context to this one.  Leaves the parent
   * in the target Context as null.
   *
   * @param c The source Context.
   * @param origin An object denoting the source of the operation, null by
   * default.
   * @return A reference to the target Context, after the assignment is
   * performed.
   */
  public Context assign
  (Context c, Origin origin)
    throws IntenseException
  {
    int deltaBasecount;
    Context contextMarker;

    deltaBasecount = c.basecount - basecount;
    clearNoPropagate(origin);
    for (contextMarker = parent; contextMarker != null;
         contextMarker = contextMarker.parent) {
      contextMarker.basecount += deltaBasecount;
      contextMarker.canonicalString = null;
    }
    propagateAssignNotify(c, origin);
    assignHelper(c, origin);
    return this;
  }

  /**
   * Assign the contents of another context to this one.  Leaves the parent
   * in the target Context as null.
   *
   * @param c The source Context.
   * @return A reference to the target Context, after the assignment is
   * performed.
   */
  public Context assign
  (Context c)
    throws IntenseException
  {
    return assign(c, null);
  }

  /**
   * Apply a ContextOp operation to a Context.
   *
   * @param op The ContextOp to apply.
   * @param origin An object denoting the source of the operation, null by
   * default.
   * @return The resulting Context after the operation is applied.
   */
  public Context apply
  (ContextOp op, Origin origin)
    throws IntenseException
  {
    Context marker;
    int deltaBasecount;

    deltaBasecount = basecount;
    propagateApplyNotify(op, origin);
    applyHelper(op, origin);
    deltaBasecount = basecount - deltaBasecount;
    for (marker = parent; marker != null; marker = marker.parent) {
      marker.basecount += deltaBasecount;
      marker.canonicalString = null;
    }
    canonicalString = null;
    return this;
  }

  /**
   * Apply a ContextOp operation to a Context.
   *
   * @param op The ContextOp to apply.
   * @return The resulting Context after the operation is applied.
   */
  public Context apply
  (ContextOp op)
    throws IntenseException
  {
    return apply(op, null);
  }

  /**
   * Clear the Context of all values and subcontexts.
   *
   * @param origin An object denoting the source of the operation, null by
   * default.
   */
  public void clear
  (Origin origin)
    throws IntenseException
  {
    boolean hadBasecount = basecount > 0;
    int tempBasecount = basecount;

    clearNoPropagate(origin);
    if (hadBasecount) {
      propagateBasecount(-tempBasecount);
      propagateClearNotify(origin);
    }
  }

  /**
   * Clear the Context of all values and subcontexts.
   */
  public void clear
  ()
    throws IntenseException
  {
    clear((Origin)null);
  }

  /**
   * Clear the Context of all values and subcontexts, without propagating
   * the change in basecount to ancestors.
   *
   * @param origin An object denoting the source of the operation, null by
   * default.
   */
  public void clearNoPropagate
  (Origin origin)
    throws IntenseException
  {
    boolean hadBasecount = basecount > 0;

    if (headcount == 0) {
      super.clearNoPropagate();
      return;
    } else {
      Iterator itr = iterator();

      basecount = 0;
      canonicalString = null;
      base = null;
      while (itr.hasNext()) {
        Pair pair = (Pair)itr.next();
        AEther child = (AEther)pair.context;

        if (child.headCount() > 0) {
          child.clearNoPropagate(origin);
        } else {
          itr.remove();
        }
      }
      if (hadBasecount&&(participants.size() > 0)) {
        clearNotify(origin);
      }
    }
  }

  /**
   * Clear the Context of all values and subcontexts under the given
   * dimension.
   *
   * @param dim The String dimension under which to clear.
   * @param origin An object denoting the source of the operation, null by
   * default.
   */
  public void clear
  (CompoundDimension compoundDimension, Origin origin)
    throws IntenseException
  {
    java.util.Iterator itr = compoundDimension.iterator();

    if (itr.hasNext()) {
      int deltaBasecount;
      Context marker = this;
      Pair pair;
      Dimension nextDimension;

      nextDimension = (Dimension)itr.next();
      while (itr.hasNext()) {
        Node node = marker.find(new Pair(nextDimension, null));

        if (node == null) {
          // The dimension doesn't exist, anyway:
          return;
        }
        marker = marker.valueRef(nextDimension);
        nextDimension = (Dimension)itr.next();
      }
      Node node = marker.find(new Pair(nextDimension, null));
      if ((node != null)&&((pair = (Pair)node.element) != null)) {
        deltaBasecount = -pair.context.basecount;
        basecount += deltaBasecount;
        propagateBasecount(deltaBasecount);
        checkedClearAndPropagateNotify((Pair)node.element, origin);
      }
    } else {
      // Perhaps EVIL, but it's essentially what they asked for:
      clear(origin);
    }
  }

  /**
   * Clear the Context of all values and subcontexts under the given
   * dimension.
   *
   * @param dim The String dimension under which to clear.
   */
  public void clear
  (CompoundDimension compoundDimension)
    throws IntenseException
  {
    clear(compoundDimension, null);
  }

  /**
   * Set the AEther's BaseValue reference.
   *
   * @param newBase A new BaseValue to add.
   * @param origin An object denoting the source of the operation, null by
   * default.
   * @return The previous value of the Context's BaseValue reference.
   */
  public void setBase
  (BaseValue newBase, Origin origin)
    throws IntenseException
  {
    ContextOp tempOp = new ContextOp();

    super.setBase(newBase);
    tempOp.setBase(newBase);
    propagateApplyNotify(tempOp, origin);
    applyNotify(tempOp, origin);
  }

  /**
   * Set the AEther's BaseValue reference.
   *
   * @param newBase A new BaseValue to add.
   * @return The previous value of the Context's BaseValue reference.
   */
  public void setBase
  (BaseValue newBase)
    throws IntenseException
  {
    setBase(newBase, null);
  }

  /**
   * Clear this AEther's BaseValue.
   *
   * @param origin An object denoting the source of the operation, null by
   * default.
   */
  public void clearBase
  (Origin origin)
    throws IntenseException
  {
    boolean hadBase = base != null;

    super.clearBase();
    if (hadBase) {
      ContextOp tempOp = new ContextOp();

      tempOp.setClearBaseFlag();
      propagateApplyNotify(tempOp, origin);
      applyNotify(tempOp, origin);
    }
  }

  /**
   * Clear this AEther's BaseValue.
   */
  public void clearBase
  ()
    throws IntenseException
  {
    clearBase(null);
  }

  /**
   * Recursively kick participants at and under this AEther node.
   *
   * @param origin An object denoting the source of the operation, null by
   * default.
   */
  public void kick
  (Origin origin)
    throws IntenseException
  {
    kick(true, origin);
  }

  /**
   * Recursively kick participants at and under this AEther node.
   */
  public void kick
  ()
    throws IntenseException
  {
    kick(true, null);
  }

  /**
   * Kick participants in this AEther.
   *
   * @param kickInChildNodes If true, will cause a full recursive kick of
   * all participants under this AEther node.
   * @param origin An object denoting the source of the operation, null by
   * default.
   */
  public void kick
  (boolean kickInChildNodes, Origin origin)
    throws IntenseException
  {
    int tempHeadcount = headcount;
    AEther marker;

    if (headcount == 0) {
      return;
    }
    if (participants.size() > 0) {
      java.util.Iterator participantsItr = participants.iterator();

      while (participantsItr.hasNext()) {
        AEtherParticipant participant =
          (AEtherParticipant)participantsItr.next();

        participant.aether = null;
        participant.kickNotify(origin);
      }
    }
    if (kickInChildNodes) {
      Iterator itr = iterator();

      while (itr.hasNext()) {
        AEther child = (AEther)((Pair)itr.next()).context;

        if (child.headCount() > 0) {
          child.kick(true, origin);
        }
      }
    }
    headcount = 0;
    for (marker = (AEther)parent; marker != null;
         marker = (AEther)marker.parent) {
      marker.headcount -= tempHeadcount;
    }
  }

  /**
   * Kick participants in this AEther.
   *
   * @param kickInChildNodes If true, will cause a full recursive kick of
   * all participants under this AEther node.
   */
  public void kick
  (boolean kickInChildNodes)
    throws IntenseException
  {
    kick(kickInChildNodes, null);
  }

  /**
   * Notify only those participants at this node in the AEther of a given
   * ContextOp.
   *
   * @param op The ContextOp to send to the participants in this AEther node.
   * @param origin An object denoting the source of the operation, null by
   * default.
   */
  protected void applyNotify
  (ContextOp op, Origin origin)
    throws IntenseException
  {
    java.util.Iterator participantItr = participants.iterator();

    while (participantItr.hasNext()) {
      ((AEtherParticipant)participantItr.next()).applyNotify(op, null, origin);
    }
  }

  /**
   * Notify all participants at this node and at ancestor nodes of a given
   * ContextOp (Ancestors are notified of the operation under an appropriate
   * dimension).
   *
   * @param op The ContextOp to send to the participants in this AEther node
   * and in ancestors of this node.
   * @param origin An object denoting the source of the operation, null by
   * default.
   */
  protected void propagateApplyNotify
  (ContextOp op, Origin origin)
    throws IntenseException
  {
    if (parent != null) {
      AEther aetherMarker;
      CompoundDimension totalDim = new CompoundDimension();

      totalDim.append(parentDim);
      for (aetherMarker = (AEther)(this.parent); aetherMarker != null;
           aetherMarker = (AEther)(aetherMarker.parent)) {
        java.util.Iterator participantItr =
          aetherMarker.participants.iterator();
        while (participantItr.hasNext()) {
          ((AEtherParticipant)participantItr.next()).
            applyNotify(op, totalDim, origin);
        }
        if (aetherMarker.parent != null) {
          totalDim.prepend(aetherMarker.parentDim);
        }
      }
    }
  }

  /**
   * Notify only those participants at this node in the AEther of a given
   * Context assignment.
   *
   * @param origin An object denoting the source of the operation, null by
   * default.
   */
  protected void assignNotify
  (Context c, Origin origin)
    throws IntenseException
  {
    java.util.Iterator participantItr = participants.iterator();

    while (participantItr.hasNext()) {
      assignNotify((AEtherParticipant)participantItr.next(), c, null, origin);
    }
  }

  /**
   * Notify a participants of a given Context assignment, potentially
   * converting the assignment to an operator, first.
   *
   * @param participant The participant to notify.
   * @param context The Context assigned to the AEther.
   * @param dimensions The dimension under which the assignment occurred (null if the
   * desired target node in the AEther is the same as the Participant's node in
   * the AEther).
   * @param origin An implementation-specific origin object, denoting the
   * source of the operation (null by default).
   */
  protected void assignNotify
  (Participant participant, Context context, CompoundDimension dimension,
   Origin origin)
    throws IntenseException
  {
    if (participant.isPure()) {
      // Participant notification via operator:
      ContextOp op = new ContextOp();

      op.apply(context);
      participant.applyNotify(op, dimension, origin);
    } else {
      participant.assignNotify(context, dimension, origin);
    }
  }

  /**
   * Notify all participants at this node and at ancestor nodes of a given
   * Context assignment (Ancestors are notified of the operation under an
   * appropriate dimension).
   *
   * @param origin An object denoting the source of the operation, null by
   * default.
   */
  protected void propagateAssignNotify
  (Context c, Origin origin)
    throws IntenseException
  {
    if (parent != null) {
      AEther aetherMarker;
      CompoundDimension totalDim = new CompoundDimension();

      totalDim.append(parentDim);
      for (aetherMarker = (AEther)(this.parent); aetherMarker != null;
           aetherMarker = (AEther)(aetherMarker.parent)) {
        java.util.Iterator participantItr =
          aetherMarker.participants.iterator();
        while (participantItr.hasNext()) {
          assignNotify(
            (AEtherParticipant)participantItr.next(), c, totalDim, origin
          );
        }
        if (aetherMarker.parent != null) {
          totalDim.prepend(aetherMarker.parentDim);
        }
      }
    }
  }

  /**
   * Notify only those participants at this node in the AEther that it has been
   * cleared.
   *
   * @param origin An object denoting the source of the operation, null by
   * default.
   */
  protected void clearNotify
  (Origin origin)
    throws IntenseException
  {
    java.util.Iterator participantItr = participants.iterator();
    while (participantItr.hasNext()) {
      clearNotify((AEtherParticipant)participantItr.next(), null, origin);
    }
  }

  private static ContextOp clearOp = null;

  /**
   * Notify a participants of a Context clear, potentially converting the clear
   * to an operator, first.
   *
   * @param participant The participant to notify.
   * @param dimensions The dimension under which the clear occurred (null if
   * the desired target node in the AEther is the same as the Participant's
   * node in the AEther).
   * @param origin An implementation-specific origin object, denoting the
   * source of the operation (null by default).
   */
  protected void clearNotify
  (Participant participant, CompoundDimension dimension, Origin origin)
    throws IntenseException
  {
    if (participant.isPure()) {
      // Participant notification via operator:
      if (clearOp == null) {
        synchronized(AEther.class) {
          if (clearOp == null) {
            try {
              clearOp = new ContextOp("[---]");
            } catch (Throwable ignored) {
            }
          }
        }
      }
      participant.applyNotify(clearOp, dimension, origin);
    } else {
      participant.clearNotify(dimension, origin);
    }
  }

  /**
   * Notify all Participant at this node and at ancestor nodes of a clear
   * (Ancestors are notified of the operation under an appropriate dimension).
   *
   * @param origin An object denoting the source of the operation, null by
   * default.
   */
  protected void propagateClearNotify
  (Origin origin)
    throws IntenseException
  {
    if (parent != null) {
      AEther aetherMarker;
      CompoundDimension totalDim = new CompoundDimension();

      totalDim.append(parentDim);
      for (aetherMarker = (AEther)(this.parent); aetherMarker != null;
           aetherMarker = (AEther)(aetherMarker.parent)) {
        java.util.Iterator participantItr =
          aetherMarker.participants.iterator();

        while (participantItr.hasNext()) {
          clearNotify(
            (AEtherParticipant)participantItr.next(), totalDim, origin
          );
        }
        if (aetherMarker.parent != null) {
          totalDim.prepend(aetherMarker.parentDim);
        }
      }
    }
  }

  public String className
  ()
  {
    return "AEther";
  }

  private void writeObject(java.io.ObjectOutputStream out)
    throws IOException
  {
    DataOutputStream dataOutputStream = new DataOutputStream(out);

    serialise(dataOutputStream);
    dataOutputStream.flush();
  }

  private void readObject(java.io.ObjectInputStream in)
    throws IOException, ClassNotFoundException, IntenseException
  {
    DataInputStream dataInputStream = new DataInputStream(in);

    deserialise(dataInputStream);
  }

  void parse
  (Reader r, Origin origin)
    throws IntenseException, IOException
  {
    ContextOpLexer lexer = new ContextOpLexer(r);
    int initialBasecount = basecount;

    clearNoPropagate(origin);
    recogniseNode(lexer, origin);
    if (basecount != initialBasecount) {
      propagateBasecount(basecount - initialBasecount);
    }
    propagateAssignNotify(this, origin);
  }

  /**
   * Parse a stream into this Context.
   *
   * @param r The stream to read from.
   */
  void parse
  (Reader r)
    throws IntenseException, IOException
  {
    parse(r, null);
  }

  /**
   * Parse the contents of a String into a Context, assuming canonical
   * form.
   *
   * @param s The String to parse.
   */
  public void parse
  (String s, Origin origin)
    throws IntenseException
  {
    try {
      parse(new StringReader(s), origin);
    } catch (IOException e) {
      throw new IntenseException(
        "Caught IOException during " + className() + " parse of String"
      );
    }
  }

  /**
   * Parse the contents of a String into a Context, assuming canonical
   * form.
   *
   * @param s The String to parse.
   */
  public void parse
  (String s)
    throws IntenseException
  {
    parse(s, null);
  }

  /**
   * Inbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.  Note, this is designed to be useful for direct
   * deserialisation into any node in a populated AEther, with appropriate
   * notification and propagation of basecount and notification, etc.
   *
   * @param in A stream to read the context from.
   */
  public void deserialise
  (DataInputStream in, BoundManager boundSerialiser, Origin origin)
    throws IOException, IntenseException
  {
    int initialBasecount = basecount;

    clear();
    deserialiseHelper(in, boundSerialiser, origin);
    propagateBasecount(basecount - initialBasecount);
    propagateAssignNotify(this, origin);
  }

  /**
   * Inbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.  Note, this is designed to be useful for direct
   * deserialisation into any node in a populated AEther, with appropriate
   * notification and propagation of basecount and notification, etc.
   *
   * @param in A stream to read the context from.
   */
  public void deserialise
  (DataInputStream in, BoundManager boundSerialiser)
    throws IOException, IntenseException
  {
    deserialise(in, boundSerialiser, null);
  }

}
