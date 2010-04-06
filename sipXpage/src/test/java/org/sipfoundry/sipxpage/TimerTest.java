/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

import junit.framework.TestCase;

public class TimerTest extends TestCase
{

   public class testListener implements LegListener
   {


      public boolean onEvent(LegEvent event)
      {
         return false;
      }
   }

   public void testTimer()
   {
      LegListener legListener = new testListener() ;

      Timer t = new Timer("testTimer", 42, legListener) ;
      assertEquals(42, t.getBeatCount()) ;
   }

   public void testCompareTo()
   {
      LegListener legListener = new testListener() ;

      Timer t1 = new Timer("testCompareTo1", 42, legListener) ;
      Timer t2 = new Timer("testCompareTo2", 52, legListener) ;
      Timer t3 = new Timer("testCompareTo2", 52, legListener) ;
      Timer t4 = new Timer("testCompareTo3", 52, legListener) ;
      assertTrue(t1.compareTo(t2) < 0) ;
      assertTrue(t2.compareTo(t2) == 0) ;
      assertTrue(t2.compareTo(t3) == 0) ;
      assertTrue(t3.compareTo(t1) > 0) ;
      assertTrue(t4.compareTo(t2) > 0) ;
   }

   public void testEqualsLegListener()
   {
      LegListener legListener1 = new testListener() ;
      LegListener legListener2 = new testListener() ;

      Timer t1 = new Timer("testCompareTo1", 42, legListener1) ;
      assertFalse(t1.equals("dog", legListener1)) ;
      assertFalse(t1.equals("testCompareTo1", legListener2)) ;
      assertTrue(t1.equals("testCompareTo1", legListener1)) ;
   }

}
