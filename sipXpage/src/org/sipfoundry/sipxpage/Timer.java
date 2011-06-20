/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;


/**
 * A Timer class that supports Comparable on beatCount
 *
 * @author Woof!
 *
 */
public class Timer implements Comparable<Timer>
{
   String name ;
   long beatCount ;
   LegListener legListener ;

   /**
    *
    * @param name The name of this timer
    * @param beatCount The beat count at which to fire
    * @param legListener The listener that gets the fired event
    */
   public Timer(String name, long beatCount, LegListener legListener)
   {
      this.name = name ;
      this.beatCount = beatCount ;
      this.legListener = legListener ;
   }

   public String getName()
   {
      return name ;
   }

   public long getBeatCount()
   {
      return beatCount ;
   }

   public LegListener getLegListener()
   {
      return legListener ;
   }

   /**
    * @param a Compare this timer's beatCount, name, and legListener to a's.
    * @return <0, 0, or >0
    *
    * Must compare all elements, as PriorityQueue.remove(Timer) will call
    * compareTo() to see if it equals Timer.  If just beatCount is used,
    * the wrong element may be removed.
    */
   public int compareTo(Timer a)
   {
      Long lA = beatCount ;
      int comp =  lA.compareTo(a.getBeatCount()) ;
      if (comp == 0)
      {
         comp = name.compareTo(a.getName()) ;
         if (comp == 0)
         {
            Integer iB = legListener.hashCode() ;
            comp = iB.compareTo(a.getLegListener().hashCode()) ;
         }
      }
      return comp ;
   }

   /**
    * Compare this timer based on name and legListener.  If both are the same, return true
    * @param name The name to compare
    * @param legListener The legListener to compare
    * @return true if both name and legListener are the same as this object's.
    */
   public boolean equals(String name, LegListener legListener)
   {
      return name.equals(this.name) && legListener.equals(this.legListener) ;
   }
}
