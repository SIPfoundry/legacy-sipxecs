/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

import java.util.PriorityQueue;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

import org.apache.log4j.Logger;

/**
 *
 * A simple timer service.
 * The listener is sent an event when the timer expires.
 *
 *
 * @author Woof!
 *
 */
public class Timers implements Runnable
{
   static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxpage") ;
   static Timers me  = null ;
   int rhythm ;            // Time (in mS) between beats
   long beatCount = System.currentTimeMillis() ;

   ScheduledExecutorService service ;
   PriorityQueue<Timer> timerQueue ;

   /**
    * @param rhythm  millisecond tick rate.  Timer resolution is at this rate.
    */
   private Timers(int rhythm)
   {
      this.rhythm = rhythm ;

      LOG.debug(String.format("Timers::Timers(%d)", rhythm)) ;
      timerQueue = new PriorityQueue<Timer>() ;

      if (rhythm > 0)
      {
         service = new ScheduledThreadPoolExecutor(1) ;
         service.scheduleAtFixedRate(this, 0, rhythm, TimeUnit.MILLISECONDS);
      }
      else
      {
         // For testing, send in a negative value for rhythm disables the
         // fixedRate calling thread.  This allows the unitTest to call beat()
         // directly without interfearence from real time.
      }
   }

   /**
    * @param timerName The name of the timer (one per legListener)
    * @param mS in how many mS to fire the timer
    * @param legListener the listener that gets the event
    */
   private void add(String timerName, int mS, LegListener legListener)
   {
      synchronized (timerQueue)
      {
         LOG.debug(String.format("Timers::add(%s, %d) %d", timerName, mS, mS+beatCount)) ;
         for (Timer inQueue : timerQueue)
         {
            if (inQueue.equals(timerName, legListener))
            {
               // Remove any timer that already exists
               // (essentually bump up the beatCount)
               LOG.debug(String.format("Timers::add(%s=%s) duplicate", timerName, inQueue.getName())) ;
               timerQueue.remove(inQueue) ;
            }
            else
            {
               LOG.debug(String.format("Timers::add(%s != %s)", timerName, inQueue.getName())) ;
            }
         }
         Timer t = new Timer(timerName, mS+beatCount, legListener) ;
         timerQueue.add(t) ;
      }
   }

   /**
    * @param timerName remove this name from the timer list for legListener
    * @param legListener the listener that gets the event
    */
   private void remove(String timerName, LegListener legListener)
   {
      synchronized (timerQueue)
      {
         LOG.debug(String.format("Timers::remove(%s, %s)",
               timerName, legListener.toString())) ;
         for (Timer inQueue : timerQueue)
         {
            if (inQueue.equals(timerName, legListener))
            {
               // Remove any timer that already exists
               timerQueue.remove(inQueue) ;
               // Notify the listener it's gone
               trigger("timer: status=removed name="+timerName, legListener) ;
            }
         }
      }
   }

   /**
    * Remove all timers on this legListener
    *
    * @param legListener
    */
   private void remove(LegListener legListener)
   {
      synchronized (timerQueue)
      {
         LOG.debug(String.format("Timers::remove(%s)", legListener.toString())) ;
         for (Timer inQueue : timerQueue)
         {
            if (inQueue.getLegListener() == legListener)
            {
               String timerName = inQueue.getName() ;
               // Remove any timer that already exists
               timerQueue.remove(inQueue) ;
               // Notify the listener it's gone
               trigger("timer: status=removed name="+timerName, legListener) ;
            }
         }
      }

   }


   /**
    * Beat (as in musical beat)
    * Called periodically based on {@link rhythm rhythm}.
    */
   void beat()
   {
      synchronized (timerQueue)
      {
         if (rhythm < 0)
         {
            // Negative rhythm is for testing sans a real clock
            beatCount += -rhythm ;
         }
         else
         {
            beatCount = System.currentTimeMillis() ;
         }

         if (timerQueue.size() > 0)
         {
            // Remove and fire all timers with beatCount <= current beatCount
            for (Timer t = timerQueue.peek(); t != null && t.getBeatCount() <= beatCount; t = timerQueue.peek())
            {
               t = timerQueue.remove() ;
               LOG.debug(String.format("Timers::beat %d: %s fired (%d mS off)", beatCount, t.getName(), beatCount - t.getBeatCount())) ;
               // Notify the listener it's fired
               trigger("timer: status=fired name="+t.getName(), t.getLegListener()) ;
            }
         }
      }
   }

   void trigger(String event, LegListener legListener)
   {
      LOG.debug(String.format("Timers::trigger event(%s)", event)) ;

      legListener.onEvent(new LegEvent(null, event)) ;
   }

   /**
    * A {@link java.thread.run run} method to call beat in a thread.
    */
   public void run()
   {
      beat() ;
   }

   public static synchronized Timers start(int rhythm)
   {
      /*
       * Create a singleton.
       */
      if (me == null)
      {
         me = new Timers(rhythm) ;
      }
      return me ;
   }

   public static synchronized void stop()
   {
      if (me != null)
      {
         if (me.service != null)
         {
            me.service.shutdown() ;
         }
         me = null ;
      }
   }

   public static synchronized Timers getTimers()
   {
      return me ;
   }

   /**
    * @param timerName The name of the timer (one per legListener)
    * @param mS in how many mS to fire the timer
    * @param legListener the listener that gets the event
    */
   public static synchronized void addTimer(String timerName, int mS, LegListener legListener)
   {
      if (me != null)
      {
         me.add(timerName, mS, legListener) ;
      }
   }

   /**
    * @param timerName remove this name from the timer list for legListener
    * @param legListener the listener that gets the event
    */
   public static synchronized void removeTimer(String timerName, LegListener legListener)
   {
      if (me != null)
      {
         me.remove(timerName, legListener) ;
      }
   }

   /**
    * Remove all timers on this legListener
    */
   public static void removeTimer(LegListener legListener)
   {
      if (me != null)
      {
         me.remove(legListener) ;
      }
   }
}
