// ****************************************************************************
//
// ContextOp.java - Intensional context operators.
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


import java.lang.*;
import java.util.*;
import java.io.*;


/**
 * Intensional Context Operators.
 */
public class ContextOp
  extends Context {

  static final long serialVersionUID = 1358630066795242105L;

  /**
   * A boolean denoting whether this ContextOp will clear the base value of
   * its operand.
   */
  protected boolean clearBaseFlag;

  /**
   * A boolean denoting whether this ContextOp will clear all sub-Contexts of
   * its operand, except those explicitly included in the ContextOp.
   */
  protected boolean clearDimsFlag;

  /**
   * The total number of - and -- flags under this ContextOp.
   */
  protected int blankcount;

  /**
   * Simple inner class for representing the int-pair return values of
   * contextOpVsetHelper and contextOpVmodHelper.
   */
  protected class OpHelperRC {

    public int deltaBasecount;

    public int deltaBlankcount;

    /**
     * Initialization constructor.
     */
    public OpHelperRC
    (int deltaBlankcount, int deltaBasecount)
    {
      this.deltaBlankcount = deltaBlankcount;
      this.deltaBasecount = deltaBasecount;
    }

  }

  /**
   * Get the blankcount in this ContextOp.
   *
   * @return The integer blankcount basecount of the ContextOp.
   */
  public int blankCount
  ()
  {
    return blankcount;
  }

  /**
   * Adds a delta to all the blankcounts of all ancestors of this ContextOp.
   *
   * @param delta The integer blankcount delta to apply.
   */
  protected void propagateBlankcount
  (int delta)
  {
    Context contextMarker;

    for (contextMarker = parent; contextMarker != null;
         contextMarker = contextMarker.parent) {
      ((ContextOp)(contextMarker)).blankcount += delta;
      contextMarker.canonicalString = null;
    }
  }

  /**
   * Recursive operation helper for apply().
   *
   * @param op An operator (ContextOp) to apply to this ContextOp.
   * @return An OpHelperRC that contains the blankcount delta and basecount
   * delta that results from applying the given ContextOp to this ContextOp.
   */
  void applyHelper
  (ContextOp op)
  {
    OpHelperRC returnValue = new OpHelperRC(0, 0);
    Iterator opItr = op.iterator(), itr = iterator();
    Pair opItrPair, itrPair, insertPair, tempPair;
    ContextOp contextOpMarker;
    boolean mustClearCurrentDimension;
    Node nodeMarker;
    int initialChildBasecount;
    int initialChildBlankcount;

    if (op.clearBaseFlag) {
      if (base != null) {
        basecount--;
        clearBaseFlag = true;
        blankcount++;
        base = null;
      } else if (!clearBaseFlag) {
        clearBaseFlag = true;
        blankcount++;
      }
    } else if (op.base != null) {
      if (base == null) {
        if (clearBaseFlag()) {
          clearBaseFlag = false;
          blankcount--;
        }
        basecount++;
      }
      base = (BaseValue)(op.base.clone());
    }
    if (op.clearDimsFlag) {
      itrPair = (Pair)(itr.next());
      for (opItrPair = (Pair)(opItr.next()); opItrPair != null;
           opItrPair = (Pair)(opItr.next())) {
        while ((itrPair != null)&&
               (!(itrPair.dim.equals(opItrPair.dim)))) {
          // This is a dimension that is in *this, but not in op, so
          // we have to delete it:
          blankcount -= ((ContextOp)(itrPair.context)).blankcount;
          basecount -= itrPair.context.basecount;
          tempPair = (Pair)itr.next();
          delete(itrPair);
          itrPair = tempPair;
        }
        mustClearCurrentDimension = false;
        if (itrPair != null) {
          // This dimension already exists in *this.
          contextOpMarker = (ContextOp)(itrPair.context);
          initialChildBasecount = contextOpMarker.basecount;
          initialChildBlankcount = contextOpMarker.blankcount;
          contextOpMarker.applyHelper((ContextOp)(opItrPair.context));
          basecount += contextOpMarker.basecount - initialChildBasecount;
          blankcount += contextOpMarker.blankcount - initialChildBlankcount;
        } else {
          // This dimension did not already exist in *this.
          contextOpMarker = new ContextOp();
          itrPair = new Pair(opItrPair.dim, contextOpMarker);
          itr = iterator(probe(itrPair));
          // If the clearDimsFlag is true in this ContextOp or we are
          // already pruning from above here, any new dimensions
          // added from op have to contain actual basevalues:
          if (clearDimsFlag) {
            contextOpMarker.clearBaseFlag = true;
            contextOpMarker.clearDimsFlag = true;
            contextOpMarker.blankcount = 2;
            initialChildBasecount = contextOpMarker.basecount;
            initialChildBlankcount = contextOpMarker.blankcount;
            contextOpMarker.applyHelper((ContextOp)(opItrPair.context));
            if (contextOpMarker.basecount == 0) {
              // *contextOpMarker contains no actual base values.
              // It's toast:
              mustClearCurrentDimension = true;
            } else {
              // *contextOpMarker contains some base values.  We
              // keep it:
              basecount += contextOpMarker.basecount - initialChildBasecount;
              blankcount +=
                2 + contextOpMarker.blankcount - initialChildBlankcount;
            }
          } else {
            initialChildBasecount = contextOpMarker.basecount;
            initialChildBlankcount = contextOpMarker.blankcount;
            contextOpMarker.applyHelper((ContextOp)(opItrPair.context));
            basecount += contextOpMarker.basecount - initialChildBasecount;
            blankcount += contextOpMarker.blankcount - initialChildBlankcount;
          }
        }
        // Prune any extraneous dim:<> sub-obs we created, if this
        // ContextOp has its clear_dims_flag set:
        if ((!mustClearCurrentDimension)&&
            (contextOpMarker.basecount == 0)&&
            (contextOpMarker.blankcount == 2)&&
            (contextOpMarker.clearBaseFlag)&&
            (contextOpMarker.clearDimsFlag)) {
          blankcount -= 2;
          mustClearCurrentDimension = true;
        }
        if (mustClearCurrentDimension) {
          tempPair = (Pair)(itr.next());
          delete(itrPair);
          itrPair = tempPair;
        } else {
          itrPair = (Pair)(itr.next());
        }
      }
      // Now we have to empty any remaining dimensions in *this:
      while (itrPair != null) {
        blankcount -= ((ContextOp)itrPair.context).blankcount;
        basecount -= itrPair.context.basecount;
        tempPair = (Pair)itr.next();
        delete(itrPair);
        itrPair = tempPair;
      }
      // Now, since we applied a -- to *this, *this must get one.
      if (!clearDimsFlag) {
        blankcount++;
        clearDimsFlag = true;
      }
    } else { // op.clearDimsFlag is false
      for (opItrPair = (Pair)(opItr.next()); opItrPair != null;
           opItrPair = (Pair)(opItr.next())) {
        mustClearCurrentDimension = false;
        nodeMarker = find(opItrPair);
        if (nodeMarker != null) {
          itrPair = (Pair)(nodeMarker.element);
          contextOpMarker = (ContextOp)(itrPair.context);
          initialChildBasecount = contextOpMarker.basecount;
          initialChildBlankcount = contextOpMarker.blankcount;
          contextOpMarker.applyHelper((ContextOp)(opItrPair.context));
          basecount += contextOpMarker.basecount - initialChildBasecount;
          blankcount += contextOpMarker.blankcount - initialChildBlankcount;
        } else {
          contextOpMarker = new ContextOp();
          itrPair = new Pair(opItrPair.dim, contextOpMarker);
          insert(itrPair);
          // If the clear_dims_flag is true in *this, any new
          // dimensions added from op have to contain actual
          // basevalues:
          if (clearDimsFlag) {
            contextOpMarker.clearBaseFlag = true;
            contextOpMarker.clearDimsFlag = true;
            contextOpMarker.blankcount = 2;
            initialChildBasecount = contextOpMarker.basecount;
            initialChildBlankcount = contextOpMarker.blankcount;
            contextOpMarker.applyHelper((ContextOp)(opItrPair.context));
            if (contextOpMarker.basecount == 0) {
              // *contextOpMarker contains no actual base values.
              // It's toast:
              mustClearCurrentDimension = true;
            } else {
              basecount += contextOpMarker.basecount - initialChildBasecount;
              blankcount +=
                2 + contextOpMarker.blankcount - initialChildBlankcount;
            }
          } else {
            initialChildBasecount = contextOpMarker.basecount;
            initialChildBlankcount = contextOpMarker.blankcount;
            contextOpMarker.applyHelper((ContextOp)(opItrPair.context));
            basecount += contextOpMarker.basecount - initialChildBasecount;
            blankcount += contextOpMarker.blankcount - initialChildBlankcount;
          }
        }
        if ((!clearDimsFlag)&&(contextOpMarker.empty())) {
          // This is a vmod, and we have a vanilla vmod under a
          // subdimension; we have to delete it:
          mustClearCurrentDimension = true;
        } else if ((!mustClearCurrentDimension)&&
                   clearDimsFlag&&
                   (contextOpMarker.basecount == 0)&&
                   (contextOpMarker.blankcount == 2)&&
                   (contextOpMarker.clearBaseFlag)&&
                   (contextOpMarker.clearDimsFlag)) {
          // Prune any extraneous dim:<> sub-obs we created, if
          // this ContextOp has its clear_dims_flag set:
          mustClearCurrentDimension = true;
          blankcount -= 2;
        }
        if (mustClearCurrentDimension) {
          delete(itrPair);
        }
      }
    }
    canonicalString = null;
  }

  /**
   * Overridden new-ContextOp/Context constructor.
   *
   * @param dim not used.
   * @return A freshly allocated ContextOp with this ContextOp as its parent.
   */
  protected Context valueRefFactory
  (Dimension dim)
  {
    return new ContextOp(this, dim);
  }

  /**
   * A new-ContextOp/Context constructor, as per valueRefFactory, that retains
   * pruning (sets -+--) in a child, if -- is set in the parent.
   *
   * @param dim not used.
   * @return A freshly allocated ContextOp with this ContextOp as its parent.
   */
  protected ContextOp valueRefFactoryRetainPruning
  (Dimension dim)
    throws IntenseException
  {
    ContextOp op = new ContextOp(this, dim);

    if (clearDimsFlag) {
      op.setClearBaseFlag();
      op.setClearDimsFlag();
    }
    return op;
  }

  /**
   * Get a reference to the sub-ContextOp under dimension dim, with optional
   * retention of pruning (optionally set -+-- in a child, if -- is set in the
   * parent).
   *
   * @param dim A possibly-compound dimension String whose Context value to
   * return.
   * @param retainPruning If true, retains pruning in newly created children.
   * @return The ContextOp value under the argument dimension.
   */
  public ContextOp value
  (CompoundDimension dim, boolean retainPruning)
  {
    if (retainPruning) {
      java.util.Iterator itr = dim.iterator();
      ContextOp marker = this;

      while (itr.hasNext()) {
        Pair insertPair = new Pair((Dimension)itr.next(), null);
        Pair pair;

        if ((pair = (Pair)insert(insertPair)) != null) {
          // There already was a dimension with this value.
          marker = (ContextOp)pair.context;
        } else {
          marker =
            (ContextOp)(insertPair.context = valueRefFactory(insertPair.dim));
        }
      }
      return marker;
    } else {
      return (ContextOp)value(dim);
    }
  }

  /**
   * Void constructor.
   */
  public ContextOp
  ()
  {
    super();
    clearBaseFlag = false;
    clearDimsFlag = false;
    blankcount = 0;
  }

  /**
   * Parse constructor.
   *
   * @param contextOpString A canonical-form context operator string to parse.
   * @throws IntenseException if a parse error occurs.
   */
  public ContextOp
  (String contextOpString)
    throws IntenseException
  {
    super();
    clearBaseFlag = false;
    clearDimsFlag = false;
    blankcount = 0;
    parse(contextOpString);
  }

  /**
   * Copy constructor.
   *
   * @param src A ContextOp to copy.
   */
  public ContextOp
  (ContextOp src)
    throws IntenseException
  {
    super();
    assign(src);
  }

  /**
   * Void constructor with parent.
   *
   * @param parent The parent ContextOp of the new ContextOp.
   * @param parentDim The Dimension under which this ContextOp is located in
   * parent.
   */
  public ContextOp
  (Context parent, Dimension parentDim)
  {
    super(parent, parentDim);
    clearBaseFlag = false;
    clearDimsFlag = false;
    blankcount = 0;
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
    return CONTEXTOP;
  }

  /**
   * Assign the value of a ContextOp to this ContextOp, cloning all
   * basevalues in the process.
   *
   * @param op The ContextOp to assign to this ContextOp.
   * @return The resulting ContextOp, after the assignment is performed.
   */
  public ContextOp assign
  (ContextOp op)
    throws IntenseException
  {
    Pair insertPair, opItrPair;
    Iterator opItr = op.iterator();

    clear();
    canonicalString = op.canonicalString;
    basecount = op.basecount;
    blankcount = op.blankcount;
    if (op.base != null) base = (BaseValue)op.base.clone();
    else base = null;
    clearBaseFlag = op.clearBaseFlag;
    clearDimsFlag = op.clearDimsFlag;
    while (opItr.hasNext()) {
      // Note that, on all these inserts, we assume that there isn't
      // already an element under the same key, because we clear()ed,
      // above:
      opItrPair = (Pair)opItr.next();
      insertPair = new Pair(opItrPair.dim, new ContextOp(this, opItrPair.dim));
      insert(insertPair);
      ((ContextOp)(insertPair.context)).assign(((ContextOp)opItrPair.context));
    }
    return this;
  }

  /**
   * Assign the contents of another ContextOp to this one, only at the root
   * dimension level, and without cloning sub-Contexts.  Leaves the parent in
   * the target Context as null.  After a shallow assignment is made, it can
   * safely be assumed that modifying a sub-Context of the argument Context
   * will probably also modify the target Context, and vice versa.
   *
   * @param op The source ContextOp.
   * @return A reference to the target ContextOp, after the assignment is
   * performed.
   */
  public ContextOp shallowAssign
  (ContextOp op)
  {
    super.shallowAssign(op);
    clearBaseFlag = op.clearBaseFlag;
    clearDimsFlag = op.clearDimsFlag;
    blankcount = op.blankcount;
    return this;
  }

  /**
   * Test the vanilla property of this ContextOp.
   *
   * @return A boolean denoting whether this ContextOp is vanilla.
   */
  public boolean empty
  ()
  {
    return ((blankcount == 0)&&(basecount == 0));
  }

  /**
   * Clone this ContextOp (full copy).
   *
   * @return A new ContextOp that is a copy of this one.
   */
  public Object clone
  ()
  {
    try {
      return (new ContextOp()).assign(this);
    } catch (IntenseException ignored) {
      return null;
    }
  }

  /**
   * Test equality between ContextOps.
   *
   * @param op The ContextOp to test equality with.
   * @return A boolean denoting where the two ContextOps are equal.
   */
  public boolean equals
  (ContextOp op)
  {
    Iterator itr, opItr;
    Pair itrPair, opItrPair;
    Node nodeMarker;
    ContextOp opMarker;

    // ContextOps must test for equality of flags, as well as equality of
    // BaseValues and subcontexts.
    //
    // First, return (something) unless both contexts are non-vanilla:
    if (empty()) {
      if (op.empty()) return true; else return false;
    } else {
      if (op.empty()) return false;
    }
    // *** This might not be good, but if it is, it saves time:
    if (basecount != op.basecount) return false;
    // If we have equal flags and, either neither of us have BaseValues,
    // or we have equal BaseValues:
    if (((clearBaseFlag == op.clearBaseFlag)&&
         (clearDimsFlag == op.clearDimsFlag))&&
        (((base == null)&&(op.base == null))||
         ((base != null)&&(op.base != null)&&(base.equals(op.base))))) {
      itrPair = (Pair)((itr = iterator()).next());
      opItrPair = (Pair)((opItr = op.iterator()).next());
      while (itrPair != null) {
        while ((opItrPair != null)&&
               (!(opItrPair.dim.equals(itrPair.dim)))) {
          // Hmmm.  We're not in synch, so either this branch
          // is vanilla, or this is a shooting offense:
          if (!((ContextOp)(opItrPair.context)).empty()) {
            return false;
          }
          opItrPair = (Pair)opItr.next();
        }
        nodeMarker = op.find(itrPair);
        if ((((ContextOp)(itrPair.context)).empty())&&
            ((nodeMarker != null)&&
             (!((ContextOp)(((Pair)nodeMarker.element).context)).empty()))) {
          return false;
        } else if ((nodeMarker == null)||
                   (!(((ContextOp)(((Pair)nodeMarker.element).context))).
                    equals(((ContextOp)(itrPair.context))))) {
          return false;
        }
        itrPair = (Pair)itr.next();
        if (opItrPair != null) opItrPair = (Pair)opItr.next();
      }
      // Now we have to see if any remaining dimensions are vanilla:
      while (opItrPair != null) {
        if (!((ContextOp)(opItrPair.context)).empty()) return false;
        opItrPair = (Pair)opItr.next();
      }
      return true;
    }
    // One of us has a BaseValue and the other doesn't, or, we have
    // unequal BaseValues:
    return false;
  }

  /**
   * Applying a Context to a ContextOp as an operator turns the ContextOp
   * into a pure vset to the argument Context.
   *
   * @param c The Context to apply as a VSET.
   * @return A reference to the resulting ContextOp, after applying the VSET.
   * Note that this value is returned as a Context, since Java does not
   * support covariant return types.
   */
  public Context apply
  (Context c)
    throws IntenseException
  {
    super.assign(c);
    return this;
  }

  /**
   * Applying a ContextOp to a ContextOp as an operator turns the target
   * ContextOp into a (potentially new) ContextOp, representing the
   * succession of both operations.
   *
   * @param op The ContextOp to apply.
   * @return A reference to the resulting ContextOp, after applying the
   * source ContextOp - note this is returned as a Context, since Java
   * doesn't support covariant return types - cast return value to a
   * ContextOp, if you want to apply this method again directly.
   */
  public Context apply
  (ContextOp op)
    throws IntenseException
  {
    ContextOp contextOpMarker;
    int deltaBasecount = basecount;
    int deltaBlankcount = blankcount;

    applyHelper(op);
    deltaBasecount = basecount - deltaBasecount;
    deltaBlankcount = blankcount - deltaBlankcount;
    for (contextOpMarker = (ContextOp)parent; contextOpMarker != null;
         contextOpMarker = (ContextOp)(contextOpMarker.parent)) {
      contextOpMarker.basecount += deltaBasecount;
      contextOpMarker.blankcount += deltaBlankcount;
      contextOpMarker.canonicalString = null;
    }
    return this;
  }

  /**
   * Canonical-form String generation.
   *
   * @param retain If true, retain the canonical-form string in this node.
   * Note, if false, any previously-retained canonical-form string will not be
   * erased, and will be used directly to avoid recalculation.
   * @param retainInChildren If true, will retain canonical-form strings in
   * all child nodes, as well (this can lead to a large waste of space, so use
   * with care).  Note, if there is a non-null canonical string in this node,
   * children will not be visited, and retainInChildren will be ignored.
   * @return The canonical-form String of the Context.
   */
  public String canonical
  (boolean retain, boolean retainInChildren)
  {
    String returnValue;

    if (canonicalString == null) {
      Iterator itr = iterator();
      Pair itrPair = (Pair)(itr.next());
      StringBuffer canonicalStringBuffer;

      canonicalStringBuffer = new StringBuffer("[");
      if (clearBaseFlag) {
        if (clearDimsFlag) {
          canonicalStringBuffer.append("---");
        } else {
          canonicalStringBuffer.append('-');
        }
      } else if (base != null) {
        if (clearDimsFlag) {
          canonicalStringBuffer.append("--+" + base.canonical());
        } else {
          canonicalStringBuffer.append(base.canonical());
        }
      } else if (clearDimsFlag) {
        canonicalStringBuffer.append("--");
      } else if (itrPair != null) {
        // This section is just to put in at least one term,
        // before using a '+':
        // while ((itr != end_itr)&&(itr->second->empty())) itr++;
        canonicalStringBuffer.append(
          itrPair.dim.canonical() + ':' +
          itrPair.context.canonical(retainInChildren, retainInChildren)
        );
        itrPair = (Pair)(itr.next());
      }
      while (itrPair != null) {
        canonicalStringBuffer.append(
          "+" + itrPair.dim + ':' +
          itrPair.context.canonical(retainInChildren, retainInChildren)
        );
        itrPair = (Pair)(itr.next());
      }
      canonicalStringBuffer.append(']');
      returnValue = canonicalStringBuffer.toString();
      if (retain) {
        canonicalString = returnValue;
      }
    } else {
      returnValue = canonicalString;
    }
    return returnValue;
  }

  /**
   * The shorthand canonical form string.  Vsets are written in <>
   * delimeters, and the - (unless there is a base) and -- flag delimeters
   * from the corresponding vmod are removed.
   *
   * @param retain If true, retain the canonical-form string in this node.
   * Note, if false, any previously-retained canonical-form string will not be
   * erased, and will be used directly to avoid recalculation.
   * @param retainInChildren If true, will retain canonical-form strings in
   * all child nodes, as well (this can lead to a large waste of space, so use
   * with care).  Note, if there is a non-null canonical string in this node,
   * children will not be visited, and retainInChildren will be ignored.
   * @return The short-canonical-form String of the Context.
   */
  public String shortCanonical
  (boolean retain, boolean retainInChildren)
  {
    StringBuffer canonicalStringBuffer;

    if (canonicalString != null) {
      return canonicalString;
    } else {
      Iterator itr = iterator();
      Pair itrPair = (Pair)(itr.next());
      boolean isVset = false;
      String returnValue;

      if (clearBaseFlag) {
        if (clearDimsFlag) {
          canonicalStringBuffer = new StringBuffer("<");
          isVset = true;
          if (itrPair != null) {
            canonicalStringBuffer.append(
              itrPair.dim.canonical() + ':' +
              itrPair.context.shortCanonical(
                retainInChildren, retainInChildren
              )
            );
            itrPair = (Pair)(itr.next());
          }
        } else {
          canonicalStringBuffer = new StringBuffer("[-");
        }
      } else if (base != null) {
        if (clearDimsFlag) {
          canonicalStringBuffer = new StringBuffer("<" + base.canonical());
          isVset = true;
        } else {
          canonicalStringBuffer = new StringBuffer("[" + base.canonical());
        }
      } else if (clearDimsFlag) {
        canonicalStringBuffer = new StringBuffer("[--");
      } else if (itrPair != null) {
        canonicalStringBuffer =
          new StringBuffer(
            "[" + itrPair.dim + ':' +
            itrPair.context.shortCanonical(retainInChildren, retainInChildren)
          );
        itrPair = (Pair)(itr.next());
      } else {
        canonicalStringBuffer = new StringBuffer("[");
      }
      while (itrPair != null) {
        canonicalStringBuffer.append(
          "+" + itrPair.dim + ":" +
          itrPair.context.shortCanonical(retainInChildren, retainInChildren)
        );	 
        itrPair = (Pair)(itr.next());
      }
      if (isVset) {
        canonicalStringBuffer.append('>');
      } else {
        canonicalStringBuffer.append(']');
      }
      returnValue = canonicalStringBuffer.toString();
      if (retain) {
        canonicalString = returnValue;
      }
      return returnValue;
    }
  }

  /**
   * Set the ContextOp's BaseValue reference.
   *
   * @param newBase A new BaseValue to add.
   * @return The previous value of the Context's BaseValue reference.
   */
  public void setBase
  (BaseValue newBase)
    throws IntenseException
  {
    if (clearBaseFlag) {
      resetClearBaseFlag();
    }
    super.setBase(newBase);
  }

  /**
   * Get this ContextOp's clear-base flag.
   *
   * @return true if this ContextOp will explicitly clear its operand's
   * basevalue (regardless of whether it is a VSET with no explicit
   * basevalue), false otherwise.
   */
  public boolean clearBaseFlag
  ()
  {
    return clearBaseFlag;
  }

  /**
   * Set this ContextOp's clear-base flag, ie mark this ContextOp such that
   * it will clear its operand's basevalue.
   */
  public void setClearBaseFlag
  ()
    throws IntenseException
  {
    // We have to be careful to "cover all the bases" :)
    if (base != null) {
      base = null;
      basecount--;
    }
    if (!clearBaseFlag) {
      blankcount++;
      propagateBlankcount(1);
    }
    clearBaseFlag = true;
  }

  /**
   * Clear this ContextOp's clear-base flag, ie mark this ContextOp such that
   * it will not clear its operand's basevalue, unless it is a VSET with no
   * explicit basevalue.
   */
  public void resetClearBaseFlag
  ()
    throws IntenseException
  {
    if (clearBaseFlag) {
      blankcount--;
      propagateBlankcount(-1);
    }
    clearBaseFlag = false;
  }

  /**
   * Get this ContextOp's clear-dims flag.
   *
   * @return true if this ContextOp will clear all of the sub-Contexts under
   * those dimensions not explicitly contained in this ContextOp unless it is
   * a VSET; false otherwise.
   */
  public boolean clearDimsFlag
  ()
  {
    return clearDimsFlag;
  }

  /**
   * Set this ContextOp's clear-dims flag, ie mark it such that it will clear
   * all of the sub-Contexts under those dimensions not explicitly contained
   * in this ContextOp unless it is a VSET.
   */
  public void setClearDimsFlag
  ()
    throws IntenseException
  {
    if (!clearDimsFlag) {
      blankcount++;
      propagateBlankcount(1);
    }
    clearDimsFlag = true;
  }

  /**
   * Clear this ContextOp's clear-dims flag, ie mark it such that it will not
   * clear all of the sub-Contexts under those dimensions not explicitly
   * contained in this ContextOp unless it is a VSET.
   */
  public void resetClearDimsFlag
  ()
    throws IntenseException
  {
    if (clearDimsFlag()) {
      blankcount--;
      propagateBlankcount(-1);
    }
    clearDimsFlag = false;
  }

  void parse
  (Reader r)
    throws IntenseException, IOException
  {
    ContextOpLexer lexer = new ContextOpLexer(r);
    int initialBasecount = basecount;
    int initialBlankcount = blankcount;

    clear();
    recogniseNode(lexer);
    if (basecount != initialBasecount) {
      propagateBasecount(basecount - initialBasecount);
    }
    if (blankcount != initialBlankcount) {
      propagateBlankcount(blankcount - initialBlankcount);
    }
  }

  // A simple in-place recursive descent parser for ContextOp nodes, to replace
  // inefficient bison/JavaCUP implementations in C++ and Java, respectively.
  // The minimal state machine looks like this, where "term" refers to a base
  // value, a dimension:<subcontext>, '-', a '--', or a '---'.  The '('
  // transition refers to a '<' or a '[' and ')' refers to a '>' or a ']', where
  // angle brackets are balanced with angle brackets and square brackets are
  // balanced with square brackets.
  //
  //                term
  //       START/1--------->END/0<---+
  //         |               ^       | ')'
  //         | '('           | ')'   |
  //         |               |       |  '+'
  //         +-------------->2------>3------>4
  //                           term  ^       |
  //                                 |       |
  //                                 +-------+
  //                                   term
  //
  // I.e., with the same effect as (but with three less states than):
  //
  //                                   term
  //                                 +-------+
  //                                 |       |
  //                           term  v       |
  //         +-------------->5------>6------>7
  //         |               |       |
  //         | '['           | ']'   | ']'
  //         |      term     v       |
  //       START/1--------->END/0<---+
  //         |               ^       |
  //         | '<'           | '>'   | '>'
  //         |               |       |  '+'
  //         +-------------->2------>3------>4
  //                           term  ^       |
  //                                 |       |
  //                                 +-------+
  //                                   term
  //
  public void recogniseNode
  (IntenseLexer lexer)
    throws IntenseException, IOException
  {
    Token token = new Token();
    int state = 1;
    boolean inLAngles = false;
    Context tempParent = parent;
    parent = null;

    while (state != 0) {
      switch (lexer.getToken(token)) {
      case Token.BASEVALUE:
        switch (state) {
        case 1:
        case 2:
        case 4:
          if (clearBaseFlag) {
            clearBaseFlag = false;
            blankcount--;
          }
          if (base == null) {
            basecount++;
          }
          base = (BaseValue)token.value;
          if (state == 1) {
            state = 0;
          } else {
            state = 3;
          }
          break;
        case 3:
          throw new IntenseException(
            "Base value found after preceding term (with no preceding '+') " +
            "in " + className() + " parse"
          );
        }
        break;
      case Token.DIMENSION:
        switch (state) {
        case 1:
        case 2:
        case 4:
          {
            Pair insertPair = new Pair((Dimension)token.value, null);
            Pair pair;
            ContextOp child;
            int initialChildBasecount;
            int initialChildBlankcount;

            if ((pair = (Pair)insert(insertPair)) == null) {
              insertPair.context = valueRefFactory(insertPair.dim);
              child = (ContextOp)insertPair.context;
            } else {
              child = (ContextOp)(pair.context);
            }
            initialChildBasecount = child.basecount;
            initialChildBlankcount = child.blankcount;
            child.recogniseNode(lexer);
            basecount += child.basecount - initialChildBasecount;
            blankcount += child.blankcount - initialChildBlankcount;
          }
          if (state == 1) {
            state = 0;
          } else {
            state = 3;
          }
          break;
        case 3:
          throw new IntenseException(
            "Dimension \"" + (String)(token.value) +
            "\" found after term (with no preceding '+') in " +
            className() + " parse"
          );
        }
        break;
      case Token.LANGLE:
        switch (state) {
        case 1:
          inLAngles = true;
          if (base != null) {  
            base = null;
            basecount--;
          }
          if (!clearBaseFlag) {
            blankcount++;
          }
          clearBaseFlag = true;
          if (!clearDimsFlag) {
            blankcount++;
          }
          clearDimsFlag = true;
          state = 2;
          break;
        case 2:
          throw new IntenseException(
            "'<' found after initial delimiter in " + className() + " parse"
          );
        case 3:
          throw new IntenseException(
            "'<' found after term in " + className() + " parse"
          );
        case 4:
          throw new IntenseException(
            "'<' found after '+' in " + className() + " parse"
          );
        }
        break;
      case Token.RANGLE:
        switch (state) {
        case 1:
          throw new IntenseException(
            "'>' found at start of context expression in " + className()
          );
        case 2:
        case 3:
          if (inLAngles) {
            // Match:
            state = 0;
          } else {
            throw new IntenseException(
              "'>' found in context expression which started with '[' in " +
              className() + " parse"
            );
          }
          break;
        case 4:
          throw new IntenseException(
            "'>' found after '+' in " + className() + " parse"
          );
        }
        break;
      case Token.PLUS:
        switch (state) {
        case 1:
          throw new IntenseException(
            "'+' found at the start of context expression in " + className() +
            " parse"
          );
        case 2:
          throw new IntenseException(
            "'+' found after initial delimiter in " + className() +
            " parse"
          );
        case 3:
          state = 4;
          break;
        case 4:
          throw new IntenseException(
            "'+' found after preceiding '+' in "+ className() + " parse"
          );
        }
        break;
      case Token.LSQUARE:
        switch (state) {
        case 1:
          state = 2;
          break;
        case 2:
          throw new IntenseException(
            "'[' found after initial delimiter in " + className() + " parse"
          );
        case 3:
          throw new IntenseException(
            "'[' found after term in " + className() + " parse"
          );
        case 4:
          throw new IntenseException(
            "'[' found after '+' in " + className() + " parse"
          );
        }
        break;
      case Token.RSQUARE:
        switch (state) {
        case 1:
          throw new IntenseException(
            "']' found at start of context expression in " + className() +
            " parse"
          );
        case 2:
        case 3:
          if (!inLAngles) {
            // Match:
            state = 0;
          } else {
            throw new IntenseException(
              "']' found in context expression which started with '<' in " +
              className() + " parse"
            );
          }
          break;
        case 4:
          throw new IntenseException(
            "']' found after '+' in " + className() + " parse"
          );
        }
        break;
      case Token.DASH:
        switch (state) {
        case 1:
        case 2:
        case 4:
          if (base != null) {
            base = null;
            basecount--;
          }
          if (!clearBaseFlag) {
            blankcount++;
          }
          clearBaseFlag = true;
          if (state == 1) {
            state = 0;
          } else {
            state = 3;
          }
          break;
        case 3:
          throw new IntenseException(
            "'-' found after preceding term (with no preceding '+') in " +
            className() + " parse"
          );
        }
        break;
      case Token.DASHDASH:
        switch (state) {
        case 1:
        case 2:
        case 4:
          if (!clearDimsFlag) {
            blankcount++;
          }
          clearDimsFlag = true;
          if (state == 1) {
            state = 0;
          } else {
            state = 3;
          }
          break;
        case 3:
          throw new IntenseException(
            "'--' found after preceding term (with no preceding '+') in " +
            className() + " parse"
          );
        }
        break;
      case Token.DASHDASHDASH:
        switch (state) {
        case 1:
        case 2:
        case 4:
          if (base != null) {  
            base = null;
            basecount--;
          }
          if (!clearBaseFlag) {
            blankcount++;
          }
          clearBaseFlag = true;
          if (!clearDimsFlag) {
            blankcount++;
          }
          clearDimsFlag = true;
          if (state == 1) {
            state = 0;
          } else {
            state = 3;
          }
          break;
        case 3:
          throw new IntenseException(
            "'---' found after preceding term (with no preceding '+') in " +
            className() + " parse"
          );
        }
        break;
      case Token.ERROR:
        throw new IntenseException(
          "Lexical error \"" + (String)(token.value) + "\" in " +
          className() + " parse"
        );
      case Token.EOF:
        throw new IntenseException(
          "End of input reached in " + className() + " parse"
        );
      default:
        throw new IntenseException(
          "INTERNAL ERROR: Unknown token type " + token.getType() +
          " found in " + className() + " parse"
        );
      }
    }
    parent = tempParent;
  }

  public String className
  ()
  {
    return "ContextOp";
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

  /**
   * Outbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  public void serialise
  (DataOutputStream out, BoundManager boundSerialiser)
    throws IOException
  {
    int flags = 0;

    if (clearBaseFlag) {
      flags |= 0x02;
    }
    if (clearDimsFlag) {
      flags |= 0x01;
    }
    out.writeInt(flags);
    out.writeInt(blankcount);
    super.serialise(out, boundSerialiser);
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
    int initialBasecount = basecount;
    int initialBlankcount = blankcount;

    clear();
    deserialiseHelper(in, boundSerialiser, (Origin)null);
    propagateBasecount(basecount - initialBasecount);
    if (initialBlankcount != blankcount) {
      propagateBlankcount(blankcount - initialBlankcount);
    }
  }

  protected void deserialiseHelper
  (DataInputStream in, BoundManager boundSerialiser, Origin ignored)
    throws IOException, IntenseException
  {
    int tempInt = in.readInt();

    clearBaseFlag = ((tempInt&0x02) != 0x00);
    clearDimsFlag = ((tempInt&0x01) != 0x00);
    tempInt = in.readInt();
    if (blankcount < 0) {
      throw new IOException(
        "Negative blankcount " + tempInt + " in ContextOp.deserialise()"
      );
    }
    blankcount = tempInt;
    super.deserialiseHelper(in, boundSerialiser, (Origin)null);
  }

}
