// ****************************************************************************
//
// ContextDomain.java - Sets of Contexts comprising the basis for an
// intensional domain (not really the domain itself), primarily for use with
// best fits.  ImplicitContextDomain was too much of a mouthful :-).
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


import java.util.*;
import java.io.*;


/**
 * Defines a set of Contexts, which can be used to perform intensional best
 * fits.
 */
public abstract class ContextDomain
<VersionElement extends Context>
  implements Serializable {

  protected static final int VERSIONDOMAIN_MAX_SERIAL_VERSIONS = 10000;

  public interface Element<VersionElement extends Context> {

    /**
     * Returns null when end of iteration is reached (subsequent calls must
     * also return null).
     */
    abstract public Element<VersionElement> getNext
    ();

    abstract public VersionElement getVersion
    ();

  }

  /**
   * Void constructor.
   */
  public ContextDomain
  ()
  {
  }

  /**
   * Subclasses of ContextDomain must define this factory for a "first"
   * Element, allowing access to the context domain.
   *
   * @return The first Element in the subclass' context domain (list).
   */
  protected abstract Element<VersionElement> getFirstElement
  ();

  private VersionElement bestReturn
  (Context requested, boolean canThrow, VersionElement bestFit)
    throws IntenseException
  {
    if ((canThrow)&&(bestFit == null)) {
      throw new IntenseException(
        "Failed best fit for requested context " + requested.canonical()
      );
    }
    return bestFit;
  }

  /**
   * This code was taken (and modified) from the old libintensional
   * ContextSpace class in ISE 1.  Returns either a best-fit context, or
   * populates a vector of pointers to near-fit Contexts in this domain.  In
   * the latter case, the returned Contexts are "maximally refined" with
   * respect to the requested context (i.e., none of them refine to one
   * another).  If there is exactly one, this is the best fit.
   */
  private VersionElement fit
  (Element<VersionElement> firstElement, Context requested,
   List<VersionElement> nearFits, boolean canThrow)
    throws IntenseException
  {
    Element<VersionElement> element = firstElement;
    // The best-fit context:
    VersionElement bestFitContext = null;
    Vector<VersionElement> secondBestFits = new Vector<VersionElement>();
    int secondBestFitsMarker;
    // A flag for determining if it is OK to return a value:
    boolean haveBestFit = true;

    // If there are no contexts in our space upon which to do a best-fit,
    // then just return a failed fit (null):
    if (element == null) {
      if (nearFits != null) {
        return null;
      } else {
        return bestReturn(requested, canThrow, null);
      }
    }
    // Now chew through all of the contexts, looking for a best fit:
    while (element != null) {
      VersionElement thisContext;

      if ((thisContext = element.getVersion()) == null) {
        throw new IntenseException("null Context in ContextDomain.fit()");
      }
      // Test to see if this context refines to the requested context:
      if (thisContext.refinesTo(requested)) {
        // If there is already a best-fit context, then test to see if it
        // refines to this context; otherwise, set the best-fit context to
        // this context, directly:
        if (bestFitContext != null) {
          // There is a best-fit context already; set the best-fit
          // context to this context:
          if (bestFitContext.refinesTo(thisContext)) {
            bestFitContext = thisContext;
            if (nearFits != null) {
              secondBestFits.add(thisContext);
            }
          }
          // Test to see if this context does not refine to the
          // best-fit; if so, we have to store this context as a
          // second-best fit:
          else if (!(thisContext.refinesTo(bestFitContext))) {
            secondBestFits.add(thisContext);
          }
        } else {
          // There is no best-fit context yet; just set the best-fit
          // context to this context, directly:
          bestFitContext = thisContext;
          if (nearFits != null) {
            secondBestFits.add(thisContext);
          }
        }
      }
      element = element.getNext();
    }
    // Now just loop through the second-best-fits array, to see if there are
    // any second-best-fits which don't refine to the best-fit:
    for (secondBestFitsMarker = 0;
         secondBestFitsMarker < secondBestFits.size();
         secondBestFitsMarker++) {
      VersionElement thisSecondBestFit =
        secondBestFits.get(secondBestFitsMarker);

      if (!(thisSecondBestFit.refinesTo(bestFitContext))) {
        haveBestFit = false;
      }
    }
    if (haveBestFit) {
      if (nearFits != null) {
        if (bestFitContext != null) {
          nearFits.add(bestFitContext);
        }
      }
      return bestReturn(requested, canThrow, bestFitContext);
    } else {
      if (nearFits != null) {
        // There was no unique best fit - we have to do (n^2) refinement
        // comparisons.
        for (secondBestFitsMarker = 0;
             secondBestFitsMarker < secondBestFits.size();
             secondBestFitsMarker++) {
          VersionElement thisSecondBestFit =
            secondBestFits.get(secondBestFitsMarker);
          boolean doesNotRefine = true;
          int marker = secondBestFitsMarker;

          while (++marker < secondBestFits.size()) {
            Context nextSecondBestFit = (Context)secondBestFits.get(marker);

            if (thisSecondBestFit.refinesTo(nextSecondBestFit)) {
              doesNotRefine = false;
              break;
            }
          }
          if (doesNotRefine) {
            nearFits.add(thisSecondBestFit);
          }
        }
      }
      return bestReturn(requested, canThrow, null);
    }
  }

  /**
   * Determine the best-fit context in a list of contexts, termed a "context
   * domain".  The best-fit context is that which is maximally refined, with
   * respect to the requested context.  Since refinement is a partial order on
   * the set of all contexts, it is possible that a subset of "near fits", none
   * of which refine to one another, may be found; in this case, there is no
   * best fit (try near fit, instead).
   *
   * @param firstElement The first element in the context domain.
   * @param requested The desired context.
   * @param canThrow If true, will case best() to throw an IntenseException on
   * a failed best fit.
   * @return The best fit context, or null if there was no best fit (and
   * canThrow was false).
   * @throws IntenseException if canThrow was true and there was no unique
   * best-fit context, or if any of the contexts in the given domain was null.
   */
  public VersionElement best
  (Element<VersionElement> firstElement, Context requested, boolean canThrow)
    throws IntenseException
  {
    return fit(firstElement, requested, null, canThrow);
  }

  /**
   * Determine the best-fit context in a list of contexts, as supplied by
   * this ContextDomain, as per (static) best(), above.
   *
   * @param requested The desired context.
   * @param canThrow If true, will case best() to throw an IntenseException on
   * a failed best fit.
   * @return The best fit context, or null if there was no best fit (and
   * canThrow was false).
   * @throws IntenseException if canThrow was true and there was no unique
   * best-fit context, or if any of the contexts in the given domain was null.
   */
  public VersionElement best
  (Context requested, boolean canThrow)
    throws IntenseException
  {
    return best(getFirstElement(), requested, canThrow);
  }

  /**
   * Determine the best-fit context in a list of contexts, as supplied by
   * this ContextDomain, as per (static) best(), above.
   *
   * @param requested The desired context.
   * @return The best fit context.
   * @throws IntenseException if the best fit failed, or if any of the contexts
   * in the given domain was null.
   */
  public VersionElement best
  (Context requested)
    throws IntenseException
  {
    return best(getFirstElement(), requested, true);
  }

  /**
   * Determine the set of near-fit contexts over a context domain, for a given
   * requested context, using the same algorithm as per best().  If the
   * returned list contains exactly one element, this is the best-fit context.
   * If it contains no elements, then no element of the domain contained a
   * context which refined to the requested context.
   *
   * @param firstElement The first element in the context domain.
   * @param requested The desired context.
   * @return The set of near-fit contexts.
   * @throws IntenseException if any of the contexts in the given domain was
   * null.
   */
  public List<VersionElement> near
  (Element<VersionElement> firstElement, Context requested)
    throws IntenseException
  {
    LinkedList<VersionElement> returnValue = new LinkedList<VersionElement>();
    fit(firstElement, requested, returnValue, false);
    return returnValue;
  }

  /**
   * Identical to the first implementation of near(), above, but with the use
   * of a caller-supplied target near-fit contexts list.
   *
   * @param firstElement The first element in the context domain.
   * @param target The target near-fit list to populate (which is NOT cleared,
   * first - multiple calls with the same target will simply add near-fit lists
   * to the target, which is possibly useful).
   * @param requested The desired context.
   * @return The best-fit context, if there was one (i.e., target contains
   * exactly one element), or null (i.e., target contains no elements, or more
   * than one element).
   * @throws IntenseException if any of the contexts in the given domain was
   * null.
   */
  public VersionElement near
  (Element<VersionElement> firstElement, List<VersionElement> target,
   Context requested)
    throws IntenseException
  {
    return fit(firstElement, requested, target, false);
  }

  /**
   * Determine the set of near-fit contexts in a list of contexts, as supplied
   * by this ContextDomain, as per (static) near(), above.
   *
   * @param requested The desired context.
   * @return The set of near-fit contexts.
   * @throws IntenseException if any of the contexts in the given domain was
   * null.
   */
  public List<VersionElement> near
  (Context requested)
    throws IntenseException
  {
    return near(getFirstElement(), requested);
  }

  /**
   * Determine the set of near-fit contexts in a list of contexts, as supplied
   * by this ContextDomain, as per (static) near(), above.
   *
   * @param target The target near-fit list to populate (which is NOT cleared,
   * first - multiple calls with the same target will simply add near-fit lists
   * to the target, which is possibly useful).
   * @param requested The desired context.
   * @return The set of near-fit contexts.
   * @throws IntenseException if any of the contexts in the given domain was
   * null.
   */
  public VersionElement near
  (List<VersionElement> target, Context requested)
    throws IntenseException
  {
    return near(getFirstElement(), target, requested);
  }

  /**
   * Subclasses must define a means for inserting a Context (or ContextBinder,
   * etc) into a domain.
   *
   * @param context The Context to insert.
   * @param canReplace If true, will allow the replacement of a Context with
   * the same value (as per Context.compare()) as the inserted Context.
   * @return True if the context was inserted, false otherwise.
   */
  public abstract boolean insert
  (VersionElement context, boolean canReplace);

  /**
   * Insert a Context (or ContextBinder, etc) into this domain.
   *
   * @param context The Context to insert.
   * @return True if the context was inserted, false otherwise.
   */
  public boolean insert
  (VersionElement context)
  {
    return insert(context, true);
  }

  /**
   * Insert a Context (or ContextBinder, etc) into this domain.
   *
   * @param context The Context to insert.
   * @return True if the context was inserted, false otherwise.
   */
  public abstract void remove
  (Context context);

  /**
   * Clear this domain of all values.
   */
  public abstract void clear
  ();

}
