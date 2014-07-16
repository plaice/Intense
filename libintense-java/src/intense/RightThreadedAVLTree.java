// ****************************************************************************
//
// RightThreadedAVLTree.java - A (very Java-mangled) Java conversion of the
// right-threaded AVL tree implementation in Ben Pfaff's excellent libavl C
// library for AVL trees (with many of Pfaff's comments left intact), which is
// in turn based on Donald Knuth's treatment of the same topic.
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
 * Right-threaded AVL trees with O(n) forward iterator traversal.
 */
public abstract class RightThreadedAVLTree<Contained extends Object>
  implements Serializable {

  static final long serialVersionUID = 9039882662148671098L;

  private final static int PLUS = 1;

  private final static int MINUS = -1;

  private final static int AVL_MAX_HEIGHT = 32;

  private Node<Contained> root;

  private int nodeCount;


  public static class Node<Contained extends Object>
    implements Serializable {

    static final long serialVersionUID = 997058580753744046L;

    public Contained element; // Contained item.

    private Node<Contained> linkLeft;   // Subtrees or threads.

    private Node<Contained> linkRight;   // Subtrees or threads.

    private int bal;       // Balance factor.

    private int cache;     // Used during insertion.

    private int rtag;      // Right thread tag.

    private Node<Contained> getLink
    (int index)
    {
      return (index == 0) ? linkLeft : linkRight;
    }

    private Node<Contained> getLink
    ()
    {
      return (cache == 0) ? linkLeft : linkRight;
    }

    private void setLink
    (int index, Node<Contained> node)
    {
      if (index == 0) {
        linkLeft = node;
      } else {
        linkRight = node;
      }
    }

    private void setLink
    (Node<Contained> node)
    {
      if (cache == 0) {
        linkLeft = node;
      } else {
        linkRight = node;
      }
    }

    /**
     * Void constructor.
     */
    private Node
    ()
    {
      element = null;
      linkLeft = null;
      linkRight = null;
    }

  }


  /**
   * An algorithm wrapper for node element copying.
   */
  public abstract static class ElementCopier<Contained extends Object> {

    /**
     * Copy a RightThreadedAVLTree node's element data object.
     */
    public abstract Contained copy
    (Contained element);

  }


  /**
   * An Iterator for RightThreadedAVLTrees.
   */
  public static class Iterator<Contained extends Object>
    implements java.util.Iterator {

    private RightThreadedAVLTree<Contained> tree;

    private Node<Contained> node;

    protected Iterator
    (RightThreadedAVLTree<Contained> tree)
    {
      this.tree = tree;
      node = tree.root;
    }

    protected Iterator
    (RightThreadedAVLTree<Contained> tree, Node<Contained> node)
    {
      this.tree = tree;
      this.node = node;
    }

    protected Iterator
    (Iterator<Contained> itr)
    {
      this.tree = itr.tree;
      this.node = itr.node;
    }

    /**
     * Check to see if this Iterator has another item to point to.
     *
     * @return a boolean where true denotes there are more items, false
     * otherwise.
     */
    public boolean hasNext
    ()
    {
      Node<Contained> marker = node;

      // This is so that hasNext() and next() will always return false
      // and null, respectively, after they have returned so already:
      if (node == null) {
        return false;
      }
      // Knuth's Algorithm 2.3.1S (threaded inorder successor).
      if (marker.rtag == MINUS) {
        marker = marker.linkRight;
      } else {
        marker = marker.linkRight;
        while (marker.linkLeft != null) marker = marker.linkLeft;
      }
      if (marker == tree.root) {
        return false;
      } else {
        return true;
      }
    }

    /**
     * Get the next element in the RightThreadedAVLTree.
     *
     * @return The next Object element, or null if there are no more
     * elements.
     */
    public Contained next
    ()
    {
      Node<Contained> marker = node;

      // This is so that hasNext() and next() will always return false
      // and null, respectively, after they have returned so already:
      if (node == null) {
        return null;
      }
      // Knuth's Algorithm 2.3.1S (threaded inorder successor).
      if (marker.rtag == MINUS) {
        marker = marker.linkRight;
      } else {
        marker = marker.linkRight;
        while (marker.linkLeft != null) {
          marker = marker.linkLeft;
        }
      }
      if (marker == tree.root) {
        node = null;
        return null;
      } else {
        node = marker;
        return marker.element;
      }
    }

    /**
     * Removes from the underlying RightThreadedAVLTree the value referred
     * to by the last call to next().
     */
    public void remove
    ()
    {
      tree.delete(node.element);
    }

  }

  /**
   * Get a new Iterator for this tree.
   *
   * @return A freshly allocated RightThreadedAVLTree.Iterator.
   */
  public Iterator<Contained> iterator
  ()
  {
    return new RightThreadedAVLTree.Iterator<Contained>(this);
  }

  /**
   * Get an Iterator starting at a given Node.
   *
   * @param node A reference to a node in the tree.
   * @return A freshly allocated RightThreadedAVLTree.Iterator.
   */
  public Iterator<Contained> iterator
  (Node<Contained> node)
  {
    return new Iterator<Contained>(this, node);
  }

  /**
   * Get an Iterator starting at a Node with a given element.
   *
   * @param element A reference to an element to search for.
   * @return A freshly allocated RightThreadedAVLTree.Iterator.
   */
  public Iterator<Contained> iterator
  (Contained element)
  {
    return new Iterator<Contained>(this, probe(element));
  }

  /**
   * A subclass must define a key comparator.
   *
   * @param key1 left key object in comparison.
   * @param key2 right key object in comparison.
   * @return -1 for <, 0 for ==, and 1 for >.
   */
  protected abstract int cmp
  (Object key1, Object key2);

  /**
   * A subclass must define this factory, returning an instance of the
   * subclass.
   *
   * @return An instance of a subclass of RightThreadedAVLTree.
   */
  protected abstract RightThreadedAVLTree<Contained>
  rightThreadedAVLTreeFactory
  ();

  /**
   * Void constructor.
   */
  public RightThreadedAVLTree
  ()
  {
    root = new Node<Contained>();
    root.linkLeft = null;
    root.linkRight = root;
    root.rtag = PLUS;
    nodeCount = 0;
  }

  /**
   * Perform a shallow assignment between trees.  After the assignment is
   * made, modifying nodes in the source tree will probably modify the target
   * tree (fairly straightforward).
   *
   * @param t A RightThreadedAVLTree to assign to this tree.
   * @return The resulting tree.
   */
  public RightThreadedAVLTree shallowAssign
  (RightThreadedAVLTree<Contained> t)
  {
    root = t.root;
    nodeCount = t.nodeCount;
    return this;
  }

  /**
   * Get the number of nodes in a tree.
   *
   * @return The number of nodes.
   */
  public int getNodeCount
  ()
  {
    return nodeCount;
  }

  /**
   * Get the number of nodes in a tree.
   *
   * @return The number of nodes.
   */
  public int size
  ()
  {
    return nodeCount;
  }

  /**
   * Copy the contents of TREE to a new tree in arena OWNER.  If COPY is
   * non-null, then each element item is passed to function COPY, and the
   * return values are inserted into the new tree; otherwise, the items are
   * copied verbatim from the old tree to the new tree.
   *
   * @param copier An ElementCopier instance with a copy() method defined.
   * @return the new tree.
   */
  public RightThreadedAVLTree<Contained> copy
  (ElementCopier<Contained> copier)
  {
    /*
      Knuth's Algorithm 2.3.1C (copying a binary tree).  Additionally
      uses Algorithm 2.3.1I (insertion into a threaded binary tree) and
      Algorithm 2.3.1 exercise 17 (preorder successor in threaded
      binary tree).
    */
    RightThreadedAVLTree<Contained> new_tree;
    Node<Contained> p;
    Node<Contained> q;

    new_tree = rightThreadedAVLTreeFactory();
    new_tree.nodeCount = nodeCount;
    p = root;
    if (p.linkLeft == p)
      return new_tree;
    q = new_tree.root;

    for (;;) {
      // C4.  This is Algorithm 2.3.1 exercise 23 for insertion to the
      // left in a right-threaded binary tree.
      if (p.linkLeft != null) {
        Node<Contained> r = new Node<Contained>();

        q.linkLeft = r;
        r.linkLeft = null;
        r.linkRight = q;
        r.rtag = MINUS;
      }

      // C5: Find preorder successors of P and Q.  This is Algorithm
      // 2.3.1 exercise 17 but applies its actions to Q as well as
      // P.
      if (p.linkLeft != null) {
        p = p.linkLeft;
        q = q.linkLeft;
      } else {
        while (p.rtag == MINUS) {
          p = p.linkRight;
          q = q.linkRight;
        }
        p = p.linkRight;
        q = q.linkRight;
      }

      // C6.
      if (p == root) {
        return new_tree;
      }
      
      // C2.  This is Algorithm 2.3.1 exercise 23 for insertion to the
      // right in a right-threaded binary tree.
      if (p.rtag == PLUS) {
        Node<Contained> r = new Node<Contained>();

        r.linkRight = q.linkRight;
        r.rtag = q.rtag;
        q.linkRight = r;
        q.rtag = PLUS;
        r.linkLeft = null;
      }
      // C3.
      q.bal = p.bal;
      if (copier == null) {
        q.element = p.element;
      } else {
        q.element = copier.copy(p.element);
      }
    }
  }

  /**
   * Inserts an item into the tree without replacement and returns the
   * (possibly new) Node containing the element or the duplicate element.
   * The main purpose of this routine (in lieu of insert()) is to obtain an
   * Iterator from the returned Node.  Whether or not the item was actually
   * inserted in a new Node, the element reference in the returned Node can
   * be changed by the caller, or the returned Node element can be directly
   * manipulated, but the key element in the element must not be changed.
   * This lack of encapsulation (and thus lack of datastructure integrity
   * enforcement) is in the name of efficiency.
   *
   * @param item The item to insert.
   * @return a reference to the Node containing the item.  A test of
   * (node.element==item) will show whether the item was inserted.
   */
  public Node<Contained> probe
  (Contained item)
  {
    // Uses Knuth's Algorithm 6.2.3A (balanced tree search and
    // insertion), modified for a right-threaded binary tree.  Caches
    // results of comparisons.  In empirical tests this eliminates about
    // 25% of the comparisons seen under random insertions.
    //
    // A1.
    Node<Contained> t;
    Node<Contained> s, p, q, r;

    t = root;
    s = p = t.linkLeft;
    if (s == null) {
      nodeCount++;
      q = t.linkLeft = new Node<Contained>();
      q.element = item;
      q.linkLeft = null;
      q.linkRight = t;
      q.rtag = MINUS;
      q.bal = 0;
      return q;
    }
    for (;;) {
      // A2.
      int diff = cmp(item, p.element);

      // A3.
      if (diff < 0) {
        p.cache = 0;
        q = p.linkLeft;
        if (q == null) {
          // Algorithm 2.3.1 exercise 23 for insertion to the left in
          // a right-threaded binary tree.
          q = new Node<Contained>();
          p.linkLeft = q;
          q.linkLeft = null;
          q.linkRight = p;
          q.rtag = MINUS;
          break;
        }
      } else if (diff > 0) {
        // A4.
        p.cache = 1;
        q = p.linkRight;
        if (p.rtag == MINUS) {
          // Algorithm 2.3.1 exercise 23 for insertion to the right
          // in a right-threaded binary tree.
          q = new Node<Contained>();
          q.linkRight = p.linkRight;
          q.rtag = p.rtag;
          p.linkRight = q;
          p.rtag = PLUS;
          q.linkLeft = null;
          break;
        }
      } else {
        // A2.
        return p;
      }
      // A3, A4.
      if (q.bal != 0) {
        t = p;
        s = q;
      }
      p = q;
    }
    // A5.
    nodeCount++;
    q.element = item;
    q.bal = 0;
    // A6.
    r = p = s.getLink();
    while (p != q) {
      p.bal = p.cache * 2 - 1;
      p = p.getLink();
    }
    // A7.
    if (s.cache == 0) {
      // a = -1.
      if (s.bal == 0) {
        s.bal = -1;
        return q;
      } else if (s.bal == 1) {
        s.bal = 0;
        return q;
      }
      if (r.bal == -1) {
        // A8.
        p = r;
        if (r.rtag == MINUS) {
          s.linkLeft = null;
          r.linkRight = s;
          r.rtag = PLUS;
        } else {
          s.linkLeft = r.linkRight;
          r.linkRight = s;
        }
        s.bal = r.bal = 0;
      } else {
        // A9.
        p = r.linkRight;
        r.linkRight = p.linkLeft;
        p.linkLeft = r;
        s.linkLeft = p.linkRight;
        p.linkRight = s;
        if (p.bal == -1) {
          s.bal = 1;
          r.bal = 0;
        }
        else if (p.bal == 0)
          s.bal = r.bal = 0;
        else {
          s.bal = 0;
          r.bal = -1;
        }
        p.bal = 0;
        p.rtag = PLUS;
        if (s.linkLeft == s)
          s.linkLeft = null;
        if (r.linkRight == null) {
          r.linkRight = p;
          r.rtag = MINUS;
        }
      }
    } else {
      // a == 1.
      if (s.bal == 0) {
        s.bal = 1;
        return q;
      } else if (s.bal == -1) {
        s.bal = 0;
        return q;
      }
      if (r.bal == 1) {
        // A8.
        p = r;
        if (r.linkLeft == null) {
          s.rtag = MINUS;
          r.linkLeft = s;
        } else {
          s.linkRight = r.linkLeft;
          s.rtag = PLUS;
          r.linkLeft = s;
        }
        s.bal = r.bal = 0;
      } else {
        // A9.
        p = r.linkLeft;
        r.linkLeft = p.linkRight;
        p.linkRight = r;
        s.linkRight = p.linkLeft;
        p.linkLeft = s;
        if (p.bal == 1) {
          s.bal = -1;
          r.bal = 0;
        }
        else if (p.bal == 0)
          s.bal = r.bal = 0;
        else {
          s.bal = 0;
          r.bal = 1;
        }
        p.rtag = PLUS;
        if (s.linkRight == null) {
          s.linkRight = p;
          s.rtag = MINUS;
        }
        if (r.linkLeft == r)
          r.linkLeft = null;
        p.bal = 0;
      }
    }
		
    // A10.
    if (t != root && s == t.linkRight)
      t.linkRight = p;
    else
      t.linkLeft = p;
    return q;
  }

  /**
   * Clear all nodes from this tree.
   */
  public void clearAll
  ()
  {
    root = new Node<Contained>();
    root.linkLeft = null;
    root.linkRight = root;
    root.rtag = PLUS;
    nodeCount = 0;
  }

  /**
   * Search this tree for an item matching the argument and return a
   * reference to it if found.
   *
   * @return The found Object, or null if a matching Object was not found.
   */
  public Node<Contained> find
  (Object item)
  {
    Node<Contained> marker;

    marker = root.linkLeft;
    if (marker == null)
      return null;
    for (;;) {
      int diff = cmp(item, marker.element);

      // A3.
      if (diff < 0) {
        marker = marker.linkLeft;
        if (marker == null) return null;
      } else if (diff > 0) {
        if (marker.rtag == MINUS)
          return null;
        marker = marker.linkRight;
      } else {
        return marker;
      }
    }
  }

  /**
   * Searches for an item matching item.  If found, the item is
   * removed from the tree.
   *
   * @return The found item if it existed, otherwise null.
   */
  public Contained delete
  (Contained item)
  {
    // Uses Ben Pfaff's algorithm DTR, which can be found at
    // http://www.msu.edu/user/pfaffben/avl.  Algorithm DT is based on
    // Knuth's Algorithms 6.2.2D (Tree deletion), 6.2.3A (Balanced tree
    // search and insertion), 2.3.1I (Insertion into a threaded binary
    // trees), and the notes on pages 465-466 of Vol. 3.

    // D1.
    Vector<Node<Contained>> pa =
      new Vector<Node<Contained>>(AVL_MAX_HEIGHT);

    int[] a = new int[AVL_MAX_HEIGHT]; // Stack P: Bits.
    int k = 1; // Stack P: Pointer.
    Node<Contained> p;

    a[0] = 0;
    pa.add(0, root);
    p = root.linkLeft;
    if (p == null) {
      return null;
    }
    for (;;) {
      // D2.
      int diff = cmp(item, p.element);

      if (diff == 0) {
        break;
      }
      // D3, D4.
      pa.add(k, p);
      if (diff < 0) {
        if (p.linkLeft != null) {
          p = p.linkLeft;
          a[k] = 0;
        } else {
          return null;
        }
      } else if (diff > 0) {
        if (p.rtag == PLUS) {
          p = p.linkRight;
          a[k] = 1;
        } else {
          return null;
        }
      }
      k++;
    }
    nodeCount--;
    item = p.element;

    Node<Contained> t = p;
    // *** POINTER PROBLEM ***
    Node<Contained> q = pa.get(k - 1);
    int q_link_index = a[k - 1];

    // D5.
    if (t.rtag == MINUS) {
      if (t.linkLeft != null) {
        Node<Contained> x = t.linkLeft;

        q.setLink(q_link_index, x);
        q.getLink(q_link_index).bal = 0;
        if (x.rtag == MINUS) {
          if (a[k - 1] == 1) {
            x.linkRight = t.linkRight;
          } else {
            x.linkRight = pa.get(k - 1);
          }
        }
      } else {
        q.setLink(q_link_index, t.getLink(a[k - 1]));
        if (a[k - 1] == 0) {
          pa.get(k - 1).linkLeft = null;
        } else {
          pa.get(k - 1).rtag = MINUS;
        }
      }
    } else {
      // D6.
      Node<Contained> r = t.linkRight;

      if (r.linkLeft == null) {
        r.linkLeft = t.linkLeft;
        r.bal = t.bal;
        if (r.linkLeft != null) {
          Node<Contained> s = r.linkLeft;
          while (s.rtag == PLUS)
            s = s.linkRight;
          s.linkRight = r;
        }
        q.setLink(q_link_index, r);
        a[k] = 1;
        pa.add(k++, r);
      } else {
        // D7.
        Node<Contained> s = r.linkLeft;

        a[k] = 1;
        pa.add(k++, t);

        a[k] = 0;
        pa.add(k++, r);
	    
        // D8.
        while (s.linkLeft != null) {
          r = s;
          s = r.linkLeft;
          a[k] = 0;
          pa.add(k++, r);
        }
        // D9.
        t.element = s.element;
        if (s.rtag == PLUS) {
          r.linkLeft = s.linkRight;
        } else {
          r.linkLeft = null;
        }
        p = s;
      }
    }
    // D10.
    while (--k != 0) {
      Node<Contained> s = pa.get(k);

      if (a[k] == 0) {
        Node<Contained> r = s.linkRight;

        // D10.
        if (s.bal == -1) {
          s.bal = 0;
          continue;
        } else if (s.bal == 0) {
          s.bal = 1;
          break;
        }
        if (s.rtag == MINUS || r.bal == 0) {
          // D11.
          s.linkRight = r.linkLeft;
          r.linkLeft = s;
          r.bal = -1;
          pa.get(k - 1).setLink((int)a[k - 1], r);
          break;
        } else if (r.bal == 1) {
          // D12.
          if (r.linkLeft != null) {
            s.rtag = PLUS;
            s.linkRight = r.linkLeft;
          } else {
            s.rtag = MINUS;
          }
          r.linkLeft = s;
          s.bal = r.bal = 0;
          pa.get(k - 1).setLink(a[k - 1], r);
        } else {
          // D13.
          p = r.linkLeft;
          if (p.rtag == PLUS) {
            r.linkLeft = p.linkRight;
          } else {
            r.linkLeft = null;
          }
          p.linkRight = r;
          p.rtag = PLUS;
          if (p.linkLeft == null) {
            s.linkRight = p;
            s.rtag = MINUS;
          } else {
            s.linkRight = p.linkLeft;
            s.rtag = PLUS;
          }
          p.linkLeft = s;
          if (p.bal == 1) {
            s.bal = -1;
            r.bal = 0;
          }
          else if (p.bal == 0) {
            s.bal = r.bal = 0;
          } else {
            s.bal = 0;
            r.bal = 1;
          }
          p.bal = 0;
          pa.get(k - 1).setLink((int)a[k - 1], p);
          if (a[k - 1] == 1) {
            pa.get(k - 1).rtag = PLUS;
          }
        }
      } else {
        Node<Contained> r = s.linkLeft;

        // D10.
        if (s.bal == 1) {
          s.bal = 0;
          continue;
        } else if (s.bal == 0) {
          s.bal = -1;
          break;
        }
        if (s.linkLeft == null || r.bal == 0) {
          // D11.
          s.linkLeft = r.linkRight;
          r.linkRight = s;
          r.bal = 1;
          pa.get(k - 1).setLink((int)a[k - 1], r);
          break;
        } else if (r.bal == -1) {
          // D12.
          if (r.rtag == PLUS) {
            s.linkLeft = r.linkRight;
          } else {
            s.linkLeft = null;
          }
          r.linkRight = s;
          r.rtag = PLUS;
          s.bal = r.bal = 0;
          pa.get(k - 1).setLink(a[k - 1], r);
        } else {
          // D13.
          p = r.linkRight;
          if (p.linkLeft != null) {
            r.rtag = PLUS;
            r.linkRight = p.linkLeft;
          } else {
            r.rtag = MINUS;
          }
          p.linkLeft = r;
          if (p.rtag == MINUS) {
            s.linkLeft = null;
          } else {
            s.linkLeft = p.linkRight;
          }
          p.linkRight = s;
          p.rtag = PLUS;
          if (p.bal == -1) {
            s.bal = 1;
            r.bal = 0;
          }
          else if (p.bal == 0) {
            s.bal = r.bal = 0;
          } else {
            s.bal = 0;
            r.bal = -1;
          }
          p.bal = 0;
          if (a[k - 1] == 1) {
            pa.get(k - 1).rtag = PLUS;
          }
          pa.get(k - 1).setLink((int)a[k - 1], p);
        }
      }
    }
    return item;
  }

  /**
   * Inserts an item into the tree without replacement.
   *
   * @param item The item to insert.
   * @return null if the item was inserted, otherwise a reference to the
   * duplicate item.
   */
  public Contained insert
  (Contained item)
  {
    Node<Contained> marker;
    marker = probe(item);
    return (marker.element == item) ? null : marker.element;
  }

  /**
   * Insert with replacement.
   *
   * @param item The item to insert.
   * @return null if the item did not exist in the tree, otherwise a
   * reference to the replaced item.
   */
  public Contained replace
  (Contained item)
  {
    Node<Contained> marker;
    marker = probe(item);
    if (marker.element == item) {
      return null;
    } else {
      Contained r = marker.element;
      marker.element = item;
      return r;
    }
  }

}
