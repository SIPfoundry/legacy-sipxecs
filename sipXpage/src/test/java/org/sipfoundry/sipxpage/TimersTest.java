/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

import java.util.Properties;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.TimeUnit;

import junit.framework.TestCase;

import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

public class TimersTest extends TestCase
{
   ArrayBlockingQueue<LegEvent> events ;
   Timers t ;
   LegListener legListener ;
   static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxpage") ;


   public class testListener implements LegListener
   {
      TimersTest framework ;

      testListener(TimersTest framework)
      {
         this.framework = framework ;
         framework.events = new ArrayBlockingQueue<LegEvent>(10) ;
         framework.events.clear() ;
      }

      public boolean onEvent(LegEvent event)
      {
         // Append this event to the framework's queue
         framework.events.add(event) ;
         return true ;
      }

   }

   public void setUp()
   {
       Properties props = new Properties();
       props.setProperty("log4j.rootLogger", "debug, cons");
       props.setProperty("log4j.appender.cons", "org.apache.log4j.ConsoleAppender");
       props.setProperty("log4j.appender.cons.layout", "org.sipfoundry.commons.log4j.SipFoundryLayout");
       props.setProperty("log4j.appender.cons.layout.facility", "sipXpage");

       PropertyConfigurator.configure(props);

      LOG.setLevel(Level.ALL) ;
      t = Timers.start(-1) ;
      legListener = new testListener(this) ;
   }

   public void tearDown()
   {
      Timers.stop() ;
   }

   public void testAddTimer1()
   {
      Timers.addTimer("test1", 1, legListener) ;

      // There had best be no events already in the test queue
      assertEquals(0, events.size()) ;

      // Tick once
      t.beat() ;

      // There should now be one event in the test queue
      assertEquals(1, events.size()) ;
      assertEquals("timer: status=fired name=test1", events.poll().description) ;
   }

   public void testAddTimer2()
   {
      Timers.addTimer("test1", 1, legListener) ;
      Timers.addTimer("test2", 2, legListener) ;
      Timers.addTimer("test3", 2, legListener) ;

      // There had best be no events already in the test queue
      assertEquals(0, events.size()) ;

      // Tick once
      t.beat() ;

      // There should now be one event in the test queue
      assertEquals(1, events.size()) ;
      assertEquals("timer: status=fired name=test1", events.poll().description) ;

      // Tick again
      t.beat() ;

      // There should now be two events in the test queue
      assertEquals(2, events.size()) ;
      String e1 = events.poll().description ;
      // Order of firing equal length timers is undetermined.
      if (e1.equals("timer: status=fired name=test2"))
      {
         assertEquals("timer: status=fired name=test3", events.poll().description) ;
      }
      else
      {
         assertEquals("timer: status=fired name=test2", events.poll().description) ;
      }

      // Tick again
      t.beat() ;

      // There should still be no events in the test queue
      assertEquals(0, events.size()) ;
   }

   public void testModifyTimer1()
   {
      Timers.addTimer("test1", 2, legListener) ;

      // There had best be no events already in the test queue
      assertEquals(0, events.size()) ;

      // Tick once
      t.beat() ;

      // There should still be no events already in the test queue
      assertEquals(0, events.size()) ;

      // Should modify the "test2" timer to fire in two more ticks, not one
      Timers.addTimer("test1", 2, legListener) ;

      // Tick once
      t.beat() ;

      // There should now be no events in the test queue
      assertEquals(0, events.size()) ;

      // Tick again
      t.beat() ;

      // There should now be one event in the test queue
      assertEquals(1, events.size()) ;
      assertEquals("timer: status=fired name=test1", events.poll().description) ;

   }

   public void testModifyTimer2()
   {
      Timers.addTimer("testMod1", 2, legListener) ;
      Timers.addTimer("testMod2", 2, legListener) ;

      // There had best be no events already in the test queue
      assertEquals(0, events.size()) ;
      // And two timers in the timerQueue
      assertEquals(2, t.timerQueue.size()) ;
      for (Timer inQueue : t.timerQueue)
      {
         LOG.debug(String.format("timer name=%s beats=%d", inQueue.getName(), inQueue.getBeatCount())) ;
      }

      // Tick once
      t.beat() ;

      // There should still be no events already in the test queue
      assertEquals(0, events.size()) ;

      // Should modify the "testMod2" timer to fire in two more ticks, not one
      Timers.addTimer("testMod2", 2, legListener) ;

      // And still only two timers in the timerQueue
      assertEquals(2, t.timerQueue.size()) ;

      for (Timer inQueue : t.timerQueue)
      {
         LOG.debug(String.format("timer name=%s beats=%d", inQueue.getName(), inQueue.getBeatCount())) ;
      }

      // Tick once
      t.beat() ;

      // There should now be one event in the test queue
      assertEquals(1, events.size()) ;
      assertEquals("timer: status=fired name=testMod1", events.poll().description) ;

      // Tick again
      t.beat() ;

      // There should now be one events in the test queue
      assertEquals(1, events.size()) ;
      assertEquals("timer: status=fired name=testMod2", events.poll().description) ;
   }

   public void testRemoveTimer()
   {
      Timers.addTimer("testRemove1", 60, legListener) ;
      Timers.addTimer("testRemove2", 40, legListener) ;
      Timers.addTimer("testRemove3", 20, legListener) ;

      // Should be 3 timers in the timerQueue
      assertEquals(3, t.timerQueue.size()) ;

      Timers.removeTimer("testRemove2", legListener) ;

      // Should be 2 timers in the timerQueue
      assertEquals(2, t.timerQueue.size()) ;

      // There should now be one event in the test queue
      assertEquals(1, events.size()) ;
      assertEquals("timer: status=removed name=testRemove2", events.poll().description) ;
   }

   public void testRealTime()
   {
      Timers.stop() ; // Stop the one created by setUp

      Timers.start(10) ; // Start a real one with 10 mS ticks ;

      // There had best be no events already in the test queue
      assertEquals(0, events.size()) ;
      long start = System.currentTimeMillis() ;
      Timers.addTimer("100 mS", 100, legListener) ;
      Timers.addTimer("200 mS", 200, legListener) ;

      try
      {
         // Wait up to 1 second for the first event
         LegEvent e = events.poll(1000, TimeUnit.MILLISECONDS);

         long first = System.currentTimeMillis() ;
         assertNotNull(e) ;
         assertEquals("timer: status=fired name=100 mS", e.description) ;
         LOG.debug("Time delta "+(first-start)+" mS");
//         assertTrue((first-start) > 80) ;

         // Wait up to 1 second for the second event
         e = events.poll(1000, TimeUnit.MILLISECONDS) ;

         long second = System.currentTimeMillis() ;
         assertNotNull(e) ;
         assertEquals("timer: status=fired name=200 mS", e.description) ;
         LOG.debug("Time delta "+(second-start)+" mS");
//         assertTrue((second-start) > 180) ;
      } catch (InterruptedException e1)
      {
         fail(e1.getStackTrace().toString()) ;
      }

   }

}
