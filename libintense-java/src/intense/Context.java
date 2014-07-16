// ****************************************************************************
//
// Context.java - Intensional contexts.
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
 * Intensional contexts.  Extends a right-threaded AVL tree implementation for
 * optimal space and time efficiency and scalability.
 */
public class Context
  extends RightThreadedAVLTree<Context.Pair> {

  static final long serialVersionUID = 803096928308882105L;

  public static final int VOID = 0;

  public static final int CONTEXT = 1;

  public static final int CONTEXTOP = 2;

  public static final int AETHER = 3;

  public static String typeStrings[] = {
    "VOID", "CONTEXT", "CONTEXTOP", "AETHER"
  };

  public static void checkType
  (int type)
    throws IntenseException
  {
    if ((type < VOID)||(type > AETHER)) {
      throw new IntenseException("Invalid Context type " + type);
    }
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
    return CONTEXT;
  }

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

  public static Context factory
  (int type)
    throws IntenseException
  {
    switch (type) {
    case VOID:
      return null;
    case CONTEXT:
      return new Context();
    case CONTEXTOP:
      return new ContextOp();
    case AETHER:
      return new AEther();
    default:
      throw new IntenseException("INTERNAL ERROR: Bad Context type " + type);
    }
  }

  public static void factorySerialise
  (Context context, DataOutputStream out)
    throws IOException
  {
    int type;

    if (context != null) {
      type = ((Context)context).getType();
    } else {
      type = Context.VOID;
    }
    out.writeInt(type);
    if (context != null) {
      context.serialise(out);
    }
  }

  public static Context factoryDeserialise
  (DataInputStream is)
    throws IOException, IntenseException
  {
    int type;
    Context context;

    type = is.readInt();
    context = Context.factory(type);
    if (context != null) {
      context.deserialise(is);
    }
    return context;
  }

  /**
   * The parent Context of this Context.
   */
  protected Context parent;

  /**
   * The dimension under which this Context resides in its parent Context.
   */
  protected Dimension parentDim;

  /**
   * The canonical-form string for this Context.
   */
  protected String canonicalString;

  /**
   * The total number of BaseValues under this Context.
   */
  protected int basecount;

  /**
   * The BaseValue reference for this node in the Context hierarchy.  Should
   * either be null, or point to a BaseValue subclass.
   */
  protected BaseValue base;

  /**
   * The maximum possible length (ie for purposes of deserialisation buffer
   * overflow security) of a dimension string.
   */
  public static final int MAX_DIMENSION_LENGTH = 1024;

  /**
   * An inner class to wrap String/Context pairs for RightThreadedAVLTree
   * elements, providing type checking and getting rid of lots of casts.
   */
  protected static class Pair
    implements Serializable {

    static final long serialVersionUID = -731839158710278497L;

    public Dimension dim;

    public Context context;

    /**
     * Void constructor:
     */
    public Pair
    ()
    {
      dim = null;
      context = null;
    }

    /**
     * Initialization constructor:
     */
    public Pair
    (Dimension dim, Context context)
    {
      this.dim = dim;
      this.context = context;
    }

  }

  /**
   * Subclasses of RightThreadedAVLTree must define an element comparator.
   *
   * @param key1 left key object in comparison.
   * @param key2 right key object in comparison.
   * @return -1 for <, 0 for ==, and 1 for >.
   */
  protected int cmp
  (Object key1, Object key2)
  {
    return (((Pair)key1).dim.compareTo(((Pair)key2).dim));
  }

  /**
   * Subclasses of RightThreadedAVLTree must define this factory, returning
   * an instance of the subclass.
   *
   * @return An instance of a subclass of RightThreadedAVLTree.
   */
  protected RightThreadedAVLTree<Context.Pair> rightThreadedAVLTreeFactory
  ()
  {
    return new Context();
  }

  /**
   * Verifies that all of a child-originating basecount in a context comes
   * from one subcontext.
   *
   * @return A boolean denoting that there is only one legitimate child of
   * the current Context.
   */
  protected boolean hasOneLegitChild
  ()
  {
    int count = 0;
    Iterator itr = iterator();
    Pair pair;

    while ((pair = (Pair)itr.next()) != null) {
      if (pair.context.basecount != 0) {
        if (++count > 1) {
          return false;
        }
      }
    }
    return count != 0;
  }

  /**
   * Add a delta to all the basecounts of all ancestors of this Context.
   *
   * @param delta The integer basecount delta to apply.
   */
  protected void propagateBasecount
  (int delta)
  {
    for (Context marker = parent; marker != null; marker = marker.parent) {
      marker.basecount += delta;
      marker.canonicalString = null;
    }
  }

  /**
   * Notify (participants in AEther) of an assignment to this node.  In
   * Context, this does nothing.
   */
  protected void assignNotify
  (Context c, Origin origin)
    throws IntenseException
  {
  }

  /**
   * This must be defined by subclasses (such as AEther) that need to
   * perform notifications when operations are applied.
   *
   * @param op The operation to notify of.
   */
  protected void applyNotify
  (ContextOp op, Origin origin)
    throws IntenseException
  {
  }

  /**
   * Get a reference to the Context under the non-compound dimension dim.
   *
   * @param dim A non-compound dimension whose Context value to return.
   */
  protected Context valueRef
  (Dimension dim)
  {
    Pair insertPair = new Pair(dim, null);
    Pair pair;

    if ((pair = (Pair)insert(insertPair)) != null) {
      // There already was a dimension with this value.
      return pair.context;
    } else {
      return insertPair.context = valueRefFactory(insertPair.dim);
    }
  }

  /**
   * All subclasses of Context must define this factory to instantiate
   * themselves.
   *
   * @param dim The dimension argument is solely for the benefit of
   * subclasses such as AEther, which need it.
   * @return A reference to a newly instantiated object of a subclass of
   * Context.
   */
  protected Context valueRefFactory
  (Dimension dim)
  {
    return new Context(this, dim);
  }

  /**
   * Mark the canonical string in the parent of this Context as invalid, as
   * well as the canonical strings in all other ancestors of this Context.
   */
  protected void propagateCanonicalInvalid
  ()
  {
    for (Context marker = parent; marker != null; marker = marker.parent) {
      marker.canonicalString = null;
    }
  }

  /**
   * Void constructor.
   */
  public Context
  ()
  {
    super();
    parent = null;
    parentDim = null;
    canonicalString = null;
    basecount = 0;
    base = null;
  }

  /**
   * Copy constructor.
   *
   * @param src A Context to copy.
   */
  public Context
  (Context src)
    throws IntenseException
  {
    super();
    assign(src);
  }

  /**
   * Parse constructor.
   *
   * @param contextString A canonical-form context string to parse.
   * @throws IntenseException if a parse error occurs.
   */
  public Context
  (String contextString)
    throws IntenseException
  {
    super();
    parent = null;
    parentDim = null;
    canonicalString = null;
    basecount = 0;
    base = null;
    parse(contextString);
  }

  /**
   * Empty-Context constructor with parent specification (useful with factory
   * for subcontexts).
   *
   * @param parent A reference to the new Context's parent.
   * @param parentDim The Dimension under which this Context node resides in
   * parent.
   */
  protected Context
  (Context parent, Dimension parentDim)
  {
    super();
    this.parent = parent;
    this.parentDim = parentDim;
    canonicalString = null;
    basecount = 0;
    base = null;
  }

  /**
   * Copy constructor with parent assignment.
   *
   * @param parent The parent Context of the new Context.
   * @param parentDim The Dimension under which this Context node resides in
   * parent.
   * @param c A Context to copy (uses assign(c)).
   */
  protected Context
  (Context parent, Dimension parentDim, Context c)
    throws IntenseException
  {
    super();
    this.parent = parent;
    this.parentDim = parentDim;
    canonicalString = null;
    basecount = 0;
    base = null;
    assign(c);
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
    int deltaBasecount;
    Context contextMarker;

    deltaBasecount = c.basecount - basecount;
    clearNoPropagate();
    for (contextMarker = parent; contextMarker != null;
         contextMarker = contextMarker.parent) {
      contextMarker.basecount += deltaBasecount;
      contextMarker.canonicalString = null;
    }
    assignHelper(c, null);
    return this;
  }

  protected void assignHelper
  (Context c, Origin origin)
    throws IntenseException
  {
    Iterator cItr = c.iterator();
    Pair pair, insertPair;

    canonicalString = c.canonicalString;
    basecount = c.basecount;
    if (c.base != null) {
      base = (BaseValue)(c.base.clone());
    } else {
      base = null;
    }
    assignNotify(c, origin);
    while ((pair = ((Pair)(cItr.next()))) != null) {
      if (pair.context.basecount > 0) {
        valueRef(pair.dim).assignHelper(pair.context, origin);
      }
    }
  }

  /**
   * Test the empty property of this Context.
   *
   * @return A boolean denoting whether this Context is empty.
   */
  public boolean empty
  ()
  {
    return basecount == 0;
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
    Context marker;
    int deltaBasecount;

    deltaBasecount = basecount;
    applyHelper(op, null);
    deltaBasecount = basecount - deltaBasecount;
    for (marker = parent; marker != null; marker = marker.parent) {
      marker.basecount += deltaBasecount;
      marker.canonicalString = null;
    }
    canonicalString = null;
    return this;
  }

  /**
   * For use by applyHelper, to empty a dimension behind a given iterator.
   * In a subclass such as AEther, this can be overridden, to avoid wiping
   * out certain dimensions (ie those with a headcount, for AEthers).
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
    delete(pair);
  }

  /**
   * Recursive ContextOp operation helper.
   *
   * @param op The ContextOp to apply to this Context.
   */
  protected void applyHelper
  (ContextOp op, Origin origin)
    throws IntenseException
  {
    Iterator itr = iterator();
    Iterator opItr = op.iterator();
    Pair itrPair, opItrPair, tempPair;
    Node nodeMarker;
    int initialChildBasecount;

    if (op.clearBaseFlag()) {
      if (base != null) {
        base = null;
        basecount--;
      }
    } else if (op.base != null) {
      if (base == null) {
        basecount++;
      }
      base = (BaseValue)(op.base.clone());
    }
    applyNotify(op, origin);
    if (op.clearDimsFlag()) {
      itrPair = (Pair)(itr.next());
      for (opItrPair = (Pair)(opItr.next()); opItrPair != null;
           opItrPair = (Pair)(opItr.next())) {
        while ((itrPair != null)&&(!(itrPair.dim.equals(opItrPair.dim)))) {
          // This is a dimension that is in this Context, but not in
          // op, so we have to delete it:
          basecount -= itrPair.context.basecount;
          tempPair = itrPair;
          itrPair = (Pair)(itr.next());
          checkedClear(tempPair, origin);
        }
        if (itrPair != null) {
          // This dimension already exists in *this.
          initialChildBasecount = itrPair.context.basecount;
          itrPair.context.applyHelper((ContextOp)(opItrPair.context), origin);
          basecount += itrPair.context.basecount - initialChildBasecount;
        } else {
          // This dimension did not already exist in *this.
          insert(
            itrPair = new Pair(opItrPair.dim, valueRefFactory(opItrPair.dim))
          );
          initialChildBasecount = itrPair.context.basecount;
          itrPair.context.applyHelper((ContextOp)(opItrPair.context), origin);
          basecount += itrPair.context.basecount - initialChildBasecount;
        }
        // Now, if that vmod resulted in an empty subcontext, we
        // just prune it here and now:
        if (itrPair.context.empty()) {
          tempPair = itrPair;
          itrPair = (Pair)(itr.next());
          checkedClear(tempPair, origin);
        } else {
          // This will be null if itrPair was originally null above
          // (ie itr.next() already returned null):
          itrPair = (Pair)(itr.next());
        }
      }
      // Now we have to empty any remaining dimensions in *this:
      while (itrPair != null) {
        basecount -= itrPair.context.basecount;
        tempPair = itrPair;
        itrPair = (Pair)(itr.next());
        checkedClear(tempPair, origin);
      }
    } else { // op.clearDimsFlag is false
      for (opItrPair = (Pair)(opItr.next()); opItrPair != null;
           opItrPair = (Pair)(opItr.next())) {
        tempPair = new Pair(opItrPair.dim, null);
        nodeMarker = probe(tempPair);
        if (nodeMarker.element != tempPair) {
          initialChildBasecount =
            ((Pair)(nodeMarker.element)).context.basecount;
          ((Pair)(nodeMarker.element)).context.
            applyHelper((ContextOp)(opItrPair.context), origin);
          basecount += ((Pair)(nodeMarker.element)).context.basecount -
            initialChildBasecount;
        } else {
          // This dimension did not already exist in *this.
          tempPair.context = valueRefFactory(opItrPair.dim);
          initialChildBasecount = tempPair.context.basecount;
          tempPair.context.
            applyHelper(((ContextOp)(opItrPair.context)), origin);
          basecount += tempPair.context.basecount - initialChildBasecount;
        }
        // As above, if that vmod resulted in an empty subcontext, we
        // just prune it here and now:
        if (((Pair)(nodeMarker.element)).context.empty()) {
          checkedClear((Pair)(nodeMarker.element), origin);
        }
      }
    }
    canonicalString = null;
  }

  /**
   * Clone this Context (full copy).
   *
   * @return A new Context that is a copy of this one.
   */
  public Object clone
  ()
  {
    try {
      return (new Context()).assign(this);
    } catch (IntenseException ignored) {
      return null;
    }
  }


  /**
   * @return The root node of this context (or this context, if it has no
   * parent).
   */
  public Context root
  ()
  {
    Context marker = this;

    while (marker.parent != null) {
      marker = marker.parent;
    }
    return marker;
  }


  /**
   * Test to see if this node is an ancestor of the argument node.
   *
   * @param context A possible descendant node.
   * @return true if this node is an ancestor of the argument node, and false
   * otherwise.
   */
  public boolean isAncestor
  (Context context)
  {
    Context marker = context.parent;

    while (marker != null) {
      if (marker == this) {
        return true;
      }
      marker = marker.parent;
    }
    return false;
  }


  /**
   * @return the depth of this node (0 for the root node).
   */
  public int getDepth
  ()
  {
    Context marker;
    int depth = 0;
  
    for (marker = this; marker != null; marker = marker.parent) {
      depth++;
    }
    return depth;
  }

  /**
   * Get the root node of the least subtree containing this node and the given
   * node.
   * @return A reference to the root node of the least common subtree.
   * @throws IntenseException if this node and the given node have no common
   * ancestors and are not the same node.
   */
  public Context leastCommonRoot
  (Context node)
    throws IntenseException
  {
    int depth = getDepth();
    int nodeDepth = node.getDepth();
    int minDepth;
    Context marker = this;
    Context nodeMarker = node;

    if (depth < nodeDepth) {
      minDepth = depth;
      for (int i = 0; i < (nodeDepth - depth); i++) {
        nodeMarker = nodeMarker.parent;
      }
    } else {
      minDepth = nodeDepth;
      for (int i = 0; i < (depth - nodeDepth); i++) {
        marker = marker.parent;
      }
    }
    while (marker != null) {
      if (marker == nodeMarker) {
        return marker;
      }
      marker = (Context)marker.parent;
      nodeMarker = (Context)nodeMarker.parent;
    }
    throw new IntenseException(
      "Attempt to find least common subtree containing two unrelated " +
      "Context nodes"
    );
  }

  /**
   * Test equality between this Context and the argument Context.
   *
   * @param c A Context to compare to this Context.
   * @return true if both Contexts are equal.
   */
  public boolean equals
  (Context c)
  {
    if (basecount != c.basecount) {
      return false;
    } else {
      return compare(c) == 0;
    }
  }

  /**
   * Test the intensional refinement partial ordering relation between this
   * Context and the argument Context.
   *
   * @param c A Context to compare to this Context.
   * @return true if this Context refines to the argument Context.
   */
  public boolean refinesTo
  (Context c)
  {
    Iterator itr = iterator();
    Pair itrPair;
    Node findNode;
    Context subContext;

    if (empty()) {
      return true;
    } else if (c.empty()) {
      return false;
    }
    if ((base != null)&&
        ((c.base == null)||
         ((c.base != null)&&(!(base.refinesTo(c.base)))))) {
      return false;
    }
    for (itrPair = (Pair)(itr.next()); itrPair != null;
         itrPair = (Pair)(itr.next())) {
      if (itrPair.context.empty()) {
        continue;
      } else {
        findNode = c.find(itrPair);
        // If it's not in c or it's in c and itrPair.context doesn't
        // refine to it, return false:
        if ((findNode == null)||
            (!(itrPair.context.
               refinesTo(((Pair)(findNode.element)).context)))) {
          return false;
        }
      }
    }
    return true;
  }

  /**
   * Perform efficient lexicographic comparison between this Context and the
   * argument Context.
   *
   * @param c A Context to compare to this Context.
   * @return -1 for <, 0 for ==, and 1 for >.
   */
  public int compare
  (Context c)
  {
    Iterator itr, cItr;
    Pair itrPair, cItrPair;
    int rc;

    // Probable speed enhancement:
    /*
      if (empty()) {
      if (c.empty()) return 0;
      else return -1;
      } else if (c.empty())
      return 1;
    */
    if (base != null) {
      if (c.base != null) {
        if (base.lessThan(c.base)) {
          return -1;
        } else if (c.base.lessThan(base)) {
          return 1;
        }
        // base == c.base, so carry on to subdimensions...
      } else {
        // *base > empty:
        return 1;
      }
    } else if (c.base != null) {
      // empty < *c.base
      return -1;
    }
    itr = iterator();
    cItr = c.iterator();
    // Advance the iterators to the first non-empty dimensions:
    itrPair = (Pair)itr.next();
    while ((itrPair != null)&&(itrPair.context.empty())) {
      itrPair = (Pair)itr.next();
    }
    cItrPair = (Pair)cItr.next();
    while ((cItrPair != null)&&(cItrPair.context.empty())) {
      cItrPair = (Pair)cItr.next();
    }
    while (itrPair != null) {
      if (cItrPair == null) {
        return 1; // c < *this
      }
      // c < this:
      if (itrPair.dim.compareTo(cItrPair.dim) < 0) {
        return 1;
      }
      // this < c:
      if (cItrPair.dim.compareTo(itrPair.dim) < 0) {
        return 1;
      }
      // Recurse and terminate on non-equality:
      if ((rc = (itrPair.context).compare(cItrPair.context)) != 0) {
        return rc;
      }
      // Advance the iterators to the next non-empty dimensions:
      while (((itrPair = (Pair)(itr.next())) != null)&&
             ((itrPair.context.empty())));
      while (((cItrPair = (Pair)(cItr.next())) != null)&&
             ((cItrPair.context.empty())));
    }
    // Everything's been the same so far:
    if (cItr.hasNext()&&(basecount != c.basecount)) {
      return -1;
    }
    return 0;
  }

  /**
   * Lexicographic < operation.
   *
   * @param c The Context to compare this Context to.
   * @return true if this Context is lexicographically less than the argument
   * Context.
   */
  public boolean lessThan
  (Context c)
  {
    return compare(c) == -1;
  }

  /**
   * Get a reference to the sub-Context under dimension dim.
   *
   * @param dim A possibly-compound dimension String whose Context value to
   * return.
   * @return The Context value under the argument dimension.
   */
  public Context value
  (CompoundDimension dim)
  {
    java.util.Iterator itr = dim.iterator();
    Context marker = this;

    while (itr.hasNext()) {
      marker = marker.valueRef((Dimension)itr.next());
    }
    return marker;
  }

  /**
   * Get a reference to the sub-Context under dimension dim.
   *
   * @param compoundDimension A possibly-compound dimension String which is
   * parsed and used as a CompoundDimension key.
   * @return The Context value under the argument dimension.
   */
  public Context value
  (String compoundDimension)
    throws IntenseException
  {
    return value(new CompoundDimension(compoundDimension));
  }

  /**
   * Find a path to a given node under this Context.
   *
   * @param descendant The node to find a path to.
   * @param result The CompoundDimension to prepend the path to (NOTE - the
   * result is not cleared first).
   * @throws IntenseException if the given node is not a descendant of this
   * node.
   */
  public void findPathTo
  (Context descendant, CompoundDimension result)
    throws IntenseException
  {
    for (Context marker = descendant; marker != this; marker = marker.parent) {
      if (marker.parent == null) {
        throw new IntenseException(
          "Attempt to find path of non-descendant node in Context::findPathTo"
        );
      }
      result.prepend(marker.parentDim);
    }
  }

  /**
   * Determine the ancestry of this node with respect to another node.
   *
   * @param node The node to compare ancestry with.
   * @param path If path is non-null, it is cleared and then populated with the
   * path from ancestor to descendant.
   * @return -1 if this node is a descendant of the argument node, 0 if this
   * node is the same node as the argument node, and 1 if this node is an
   * ancestor of the argument node.
   * @throws IntenseException if neither node is a descendant of the other, and
   * the two nodes are not the same.
   */
  public int getAncestry
  (Context node, CompoundDimension path)
    throws IntenseException
  {
    if (path != null) {
      path.clear();
    }
    if (this == node) {
      return 0;
    } else if (node.isAncestor(this)) {
      if (path != null) {
        node.findPathTo(this, path);
      }
      return -1;
    } else if (!(isAncestor(node))) {
      throw new IntenseException(
        "node " + node.canonical() + " is not an ancestor of node " +
        canonical() + ", or vice versa, and the two nodes are not the same"
      );
    }
    if (path != null) {
      findPathTo(node, path);
    }
    return 1;
  }

  /**
   * Canonical-form String generation, with no retentional of canonical-form
   * strings.
   *
   * @return The canonical-form String of the Context.
   */
  public String canonical
  ()
  {
    return canonical(false, false);
  }

  public String toString
  ()
  {
    return canonical();
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
      Pair itrPair = (Pair)itr.next();
      StringBuffer canonicalStringBuffer;

      canonicalStringBuffer = new StringBuffer("<");
      if (!(empty())) {
        if (base != null) {
          canonicalStringBuffer.append(base.canonical());
        } else if (itrPair != null) {
          // This section is just to put in at least one term,
          // before using a '+':
          while ((itrPair != null)&&(itrPair.context.empty()))
            itrPair = (Pair)(itr.next());
          if (itrPair != null) {
            canonicalStringBuffer.append(
              itrPair.dim.canonical() + ':' + itrPair.context.canonical()
            );
            itrPair = (Pair)itr.next();
          }
        }
        while (itrPair != null) {
          if (!(((Context)(itrPair.context)).empty())) {
            canonicalStringBuffer.append(
              '+' + itrPair.dim.canonical() + ':' +
              itrPair.context.canonical());
          }
          itrPair = (Pair)itr.next();
        }
      }
      canonicalStringBuffer.append(">");
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
   * Short-canonical-form String generation.  For Contexts, this is the same
   * as canonical().
   *
   * @return The short-canonical-form String of the Context.
   */
  public String shortCanonical
  ()
  {
    return shortCanonical(false, false);
  }

  /**
   * Short-canonical-form String generation.  For Contexts, this is the same
   * as canonical().
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
    return shortCanonical(retain, retainInChildren);
  }

  /**
   * Check to see if this Context node has a BaseValue.
   *
   * @return True, if there is a BaseValue at this node; false otherwise.
   */
  public boolean hasBase
  ()
  {
    return base != null;
  }

  /**
   * Set the basevalue in this Context node.
   *
   * @param bv The basevalue to assign to this context.
   * @return The resulting Context, after the basevalue is set.
   */
  public void setBase
  (BaseValue bv)
    throws IntenseException
  {
    Context marker;

    if (base == null) {
      for (marker = this; marker != null; marker = marker.parent) {
        marker.basecount++;
        marker.canonicalString = null;
      }
    } else {
      for (marker = this; marker != null; marker = marker.parent)
        marker.canonicalString = null;
      base = null;
    }
    base = (BaseValue)(bv.clone());
  }

  /**
   * Set the base value in this Context node to a given String basevalue (i.e.,
   * as a StringBaseValue).
   *
   * @param bv The basevalue to assign to this context.
   * @return The resulting Context.
   */
  public void setBase
  (String bv)
    throws IntenseException
  {
    setBase(new StringBaseValue(bv));
  }

  /**
   * Set the base value in this Context node to a given long int basevalue
   * (i.e., as a NumberBaseValue).
   *
   * @param bv The basevalue to assign to this context.
   * @return The resulting Context.
   */
  public void setBase
  (long bv)
    throws IntenseException
  {
    setBase(new NumberBaseValue(bv));
  }

  /**
   * Set the base value in this Context node to a given double basevalue (i.e.,
   * as a NumberBaseValue).
   *
   * @param bv The basevalue to assign to this context.
   * @return The resulting Context.
   */
  public void setBase
  (double bv)
    throws IntenseException
  {
    setBase(new NumberBaseValue(bv));
  }

  /**
   * Get the base value in this Context node.
   *
   * @return The base value in this Context node.
   */
  public BaseValue getBase
  ()
  {
    return base;
  }

  /**
   * Clear this Context's BaseValue.
   */
  public void clearBase
  ()
    throws IntenseException
  {
    if (base != null) {
      basecount--;
      propagateBasecount(-1);
    }
    base = null;
  }

  /**
   * Clear the Context of all values and subcontexts.
   */
  public void clear
  ()
    throws IntenseException
  {
    boolean hadBasecount = basecount > 0;
    int tempBasecount = basecount;

    clearNoPropagate();
    if (hadBasecount) {
      propagateBasecount(-tempBasecount);
    }
  }

  /**
   * Clear the Context of all values and subcontexts, without propagating
   * the change in basecount to ancestors.
   */
  public void clearNoPropagate
  ()
  {
    basecount = 0;
    canonicalString = null;
    base = null;
    super.clearAll();
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
      Node node = marker.find(
        new Pair(nextDimension, null)
      );
      if ((node != null)&&((pair = (Pair)node.element) != null)) {
        deltaBasecount = -pair.context.basecount;
        basecount += deltaBasecount;
        propagateBasecount(deltaBasecount);
        checkedClear((Pair)node.element, null);
      }
    } else {
      // Perhaps EVIL, but it's essentially what they asked for:
      clear();
    }
  }

  /**
   * Clear the Context of all values and subcontexts under the given
   * dimension.
   *
   * @param dim The String dimension under which to clear.
   */
  public void clear
  (String compoundDimension)
    throws IntenseException
  {
    clear(new CompoundDimension(compoundDimension));
  }

  /**
   * Get the base-value count in this Context.
   *
   * @return The integer basecount of the Context.
   */
  public int baseCount
  ()
  {
    return basecount;
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
    ContextOpLexer lexer = new ContextOpLexer(r);
    int initialBasecount = basecount;

    clearNoPropagate();
    recogniseNode(lexer, null);
    if (basecount != initialBasecount) {
      propagateBasecount(basecount - initialBasecount);
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
    try {
      //      parse(new InputStreamReader(new StringBufferInputStream(s), "UTF-8"));
      parse(new StringReader(s));
    } catch (IOException e) {
      throw new IntenseException(
        "Caught IOException during " + className() + " parse of String"
      );
    }
  }

  public static class Token {

    // token types:
    public static final int VOID = 0;
    public static final int DIMENSION = 1;
    public static final int BASEVALUE = 2;
    public static final int DASH = 3;
    public static final int DASHDASH = 4;
    public static final int DASHDASHDASH = 5;
    public static final int PLUS = 6;
    public static final int LANGLE = 7;
    public static final int RANGLE = 8;
    public static final int LSQUARE = 9;
    public static final int RSQUARE = 10;
    public static final int ERROR = 11;
    public static final int EOF = 12;

    protected int type;

    protected Object value;

    public Object getValue
    ()
    {
      return value;
    }

    public Token
    ()
    {
      type = VOID;
    }

    public int getType
    ()
    {
      return type;
    }

    public int setStringDimension
    (StringDimension dimension)
    {
      value = dimension;
      return type = DIMENSION;
    }

    public int setIntegerDimension
    (IntegerDimension dimension)
    {
      value = dimension;
      return type = DIMENSION;
    }

    public int setBaseValue
    (BaseValue bv)
    {
      value = bv;
      return type = BASEVALUE;
    }

    public int setDash
    ()
    {
      return type = DASH;
    }

    public int setDashDash
    ()
    {
      return type = DASHDASH;
    }

    public int setDashDashDash
    ()
    {
      return type = DASHDASHDASH;
    }

    public int setPlus
    ()
    {
      return type = PLUS;
    }

    public int setLAngle
    ()
    {
      return type = LANGLE;
    }

    public int setRAngle
    ()
    {
      return type = RANGLE;
    }

    public int setLSquare
    ()
    {
      return type = LSQUARE;
    }

    public int setRSquare
    ()
    {
      return type = RSQUARE;
    }

    public int setError
    (String errorText)
    {
      value = errorText;
      return type = ERROR;
    }

    public int setEOF
    ()
    {
      return type = EOF;
    }

  }

  public void recogniseNode
  (IntenseLexer lexer)
    throws IntenseException, IOException
  {
    recogniseNode(lexer, null);
  }


  // A simple in-place recursive descent parser for Context nodes, to replace
  // inefficient bison/JavaCUP implementations in C++ and Java, respectively.
  // The minimal state machine looks like this, where "term" refers to a base
  // value or a dimension:<subcontext>.
  //
  //                term
  //       START/1--------->END/0<---+
  //         |               ^       | '>'
  //         | '<'           | '>'   |
  //         |               |       |  '+'
  //         +-------------->2------>3------>4
  //                           term  ^       |
  //                                 |       |
  //                                 +-------+
  //                                   term
  //
  public void recogniseNode
  (IntenseLexer lexer, Origin origin)
    throws IntenseException, IOException
  {
    Token token = new Token();
    int state = 1;
    Context tempParent = parent;
    parent = null;

    while (state != 0) {
      switch (lexer.getToken(token)) {
      case Token.BASEVALUE:
        switch (state) {
        case 1:
          // MATCH:
          if (base == null) {
            basecount++;
          }
          base = (BaseValue)token.value;
          state = 0;
          break;
        case 2:
        case 4:
          if (base == null) {
            basecount++;
          }
          base = (BaseValue)token.value;
          state = 3;
          break;
        case 3:
          throw new IntenseException(
            "Base value found after preceding term in " + className() +
            " parse"
          );
        }
        break;
      case Token.DIMENSION:
        switch (state) {
        case 1:
        case 2:
        case 4:
          {
            Context child = valueRef((Dimension)(token.value));
            int initialChildBasecount = child.basecount;

            child.recogniseNode(lexer, origin);
            basecount += child.basecount - initialChildBasecount;
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
            "\" found after term (with no preceding '+') in " + className() +
            " parse"
          );
        }
        break;
      case Token.LANGLE:
        switch (state) {
        case 1:
          state = 2;
          break;
        case 2:
          throw new IntenseException(
            "'<' found after initial '<' in " + className() + " parse"
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
            "'>' found at start of context expression in " + className() +
            " parse"
          );
        case 2:
        case 3:
          // Match:
          state = 0;
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
            "'+' found after initial '<' in " + className() + " parse"
          );
        case 3:
          state = 4;
          break;
        case 4:
          throw new IntenseException(
            "'+' found after preceiding '+' in " + className() + " parse"
          );
        }
        break;
      case Token.LSQUARE:
        throw new IntenseException(
          "ContextOp '[' encountered in " + className() + " parse"
        );
      case Token.RSQUARE:
        throw new IntenseException(
          "ContextOp ']' encountered in " + className() + " parse"
        );
      case Token.DASH:
        throw new IntenseException(
          "ContextOp '-' encountered in " + className() + " parse"
        );
      case Token.DASHDASH:
        throw new IntenseException(
          "ContextOp '--' encountered in " + className() + " parse"
        );
      case Token.DASHDASHDASH:
        throw new IntenseException(
          "ContextOp '---' encountered in " + className() + " parse"
        );
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
    // For the benefit of AEthers - does nothing in Context:
    assignNotify(this, origin);
    parent = tempParent;
  }

  public String className
  ()
  {
    return "Context";
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
  (DataOutputStream out, BoundManager boundManager)
    throws IOException
  {
    Iterator itr = iterator();

    out.writeInt(basecount);
    BaseValue.factorySerialise(base, out, boundManager);
    out.writeInt(size());
    while (itr.hasNext()) {
      Pair itrPair = (Pair)itr.next();

      Dimension.factorySerialise(itrPair.dim, out);
      itrPair.context.serialise(out);
    }
    out.flush();
  }

  /**
   * Outbound serialisation, compatable with the C++ libintense-serial
   * XDRBaseSerialiser.
   */
  public void serialise
  (DataOutputStream out)
    throws IOException
  {
    serialise(out, null);
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

    clear();
    deserialiseHelper(in, boundSerialiser, null);
    propagateBasecount(basecount - initialBasecount);
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
  (DataInputStream in)
    throws IOException, IntenseException
  {
    deserialise(in, null);
  }

  protected void deserialiseHelper
  (DataInputStream in, BoundManager boundManager, Origin origin)
    throws IOException, IntenseException
  {
    int tempInt;

    clear();
    tempInt = in.readInt();
    if (basecount < 0) {
      throw new IOException(
        "Negative base count " + basecount + " in Context.deserialiseHelper()"
      );
    }
    basecount = tempInt;
    base = BaseValue.factoryDeserialise(in, boundManager);
    // The number of dimensions:
    tempInt = in.readInt();
    if (tempInt < 0) {
      throw new IOException(
        "Negative dimension count " + tempInt +
        " in Context.deserialiseHelper()"
      );
    }
    // Next, read in the dimension names and (recursively) their values:
    for (int i = 0; i < tempInt; i++) {
      Dimension dimension = Dimension.factoryDeserialise(in);

      if (dimension == null) {
        throw new IOException("Deserialised a null dimension");
      }
      valueRef(dimension).deserialiseHelper(in, boundManager, origin);
    }
    assignNotify(this, origin);
  }

}
