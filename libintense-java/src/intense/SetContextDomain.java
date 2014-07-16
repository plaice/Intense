// ****************************************************************************
//
// SetContextDomain.java - ContextDomain implemented as a set of Contexts.
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


import java.util.Iterator;


/**
 * Defines a set of Contexts, which can be used to perform intensional best
 * fits.
 */
public class SetContextDomain
<VersionElement extends Context>
  extends ContextDomain<VersionElement> {

  static final long serialVersionUID = -510291926424590056L;

  private RightThreadedAVLTree<VersionElement> domain;


  private static class ContextSet<VersionElement>
    extends RightThreadedAVLTree<VersionElement> {

    static final long serialVersionUID = 3948218087044610691L;

    private ContextSet()
    {
      super();
    }

    /**
     * A subclass of RightThreadedAVLTree must define a key comparator.
     *
     * @param key1 left key object in comparison.
     * @param key2 right key object in comparison.
     * @return -1 for <, 0 for ==, and 1 for >.
     */
    protected int cmp
    (Object key1, Object key2)
    {
      return ((Context)key1).compare((Context)key2);
    }

    /**
     * Subclasses of RightThreadedAVLTree must define this factory, returning
     * an instance of the subclass.
     *
     * @return A new ContextDomain.
     */
    protected RightThreadedAVLTree<VersionElement> rightThreadedAVLTreeFactory
    ()
    {
      return new ContextSet<VersionElement>();
    }

  }


  private static class SetContextDomainElement<VersionElement extends Context>
    implements ContextDomain.Element<VersionElement> {

    private RightThreadedAVLTree.Iterator<VersionElement> itr;

    private VersionElement context;

    protected SetContextDomainElement
    (RightThreadedAVLTree<VersionElement> set)
    {
      itr = set.iterator();
      if (itr.hasNext()) {
        context = (VersionElement)itr.next();
      } else {
        context = null;
      }
    }

    /**
     * Returns nul when end of iteration is reached (subsequent calls must
     * also return null).
     */
    public ContextDomain.Element<VersionElement> getNext
    ()
    {
      if (itr.hasNext()) {
        context = itr.next();
        return this;
      } else {
        return null;
      }
    }

    public VersionElement getVersion
    ()
    {
      return context;
    }

  }


  /**
   * Void constructor.
   */
  public SetContextDomain
  ()
  {
    super();
    domain = new ContextSet<VersionElement>();
  }

  /**
   * Subclasses of ContextDomain must define this factory for a "first"
   * ContextDomain.Element, allowing access to the context domain.
   *
   * @return The first ContextDomain.Element in the subclass' context domain
   * (list).
   */
  protected ContextDomain.Element<VersionElement> getFirstElement
  ()
  {
    if (domain.size() == 0) {
      return null;
    } else {
      return new SetContextDomainElement<VersionElement>(domain);
    }
  }

  /**
   * Subclasses must define a means for inserting a Context (or ContextBinder,
   * etc) into a domain.
   *
   * @param context The Context to insert.
   * @param canReplace If true, will allow the replacement of a Context with
   * the same value (as per Context.compare()) as the inserted Context.
   * @return True if the context was inserted, false otherwise (i.e., if
   * canReplace is true, then the method returns true).
   */
  public boolean insert
  (VersionElement context, boolean canReplace)
  {
    if (canReplace) {
      domain.replace(context);
      return true;
    } else {
      if (domain.insert(context) == null) {
        return true;
      } else {
        return false;
      }
    }
  }

  /**
   * Remove a Context (or ContextBinder, etc) from this domain.
   *
   * @param context The Context to insert.
   * @return True if the context was inserted, false otherwise.
   */
  public void remove
  (Context context)
  {
    RightThreadedAVLTree.Node<VersionElement> node = domain.find(context);

    if (node != null) {
      domain.delete(node.element);
    }
  }

  /**
   * Clear this domain of all values.
   */
  public void clear
  ()
  {
    domain.clearAll();
  }

}
