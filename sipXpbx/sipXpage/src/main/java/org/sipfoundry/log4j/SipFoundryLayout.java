package log4j;

import java.text.SimpleDateFormat;
import java.util.TimeZone;

import org.apache.log4j.Layout;
import org.apache.log4j.Level;
import org.apache.log4j.Priority;
import org.apache.log4j.spi.LoggingEvent;

import util.Hostname;

/**
 * A log4j Layout class that matches the SipFoundry C++ OsSyslog format (within reason)
 * 
 * @author Woof!
 */
public class SipFoundryLayout extends Layout
{
   static Long lineNumber = 0L ;
   SimpleDateFormat dateFormat ;
   String hostName ;
   String facility ;
   public SipFoundryLayout()
   {
      super();
      dateFormat = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSSSSS'Z'") ;
      dateFormat.setTimeZone(TimeZone.getTimeZone("UTC")) ;
      hostName = Hostname.get() ;
      facility = "JAVA" ; // Can be set from the property log4j.appender.xxx.layout.facility
   }

   /**
    * Map the log4j levels to the SipFoundry text (ERROR is ERR)
    * @param l The level to map
    * @return The SipFoundry level text if available, otherwise the log4j text
    */
   private String mapLevel2SipFoundry(Level l)
   {
      switch (l.toInt())
      {
         case Priority.DEBUG_INT: return "DEBUG" ;
         case Priority.INFO_INT: return "INFO" ;
         case Priority.WARN_INT: return "WARNING" ;
         case Priority.ERROR_INT: return "ERR" ;
         default: return l.toString() ;
      }
   }
   
   /**
    * Map the SipFoundry text to the log4j Priority number
    * @param level
    * @return
    */
   public static Level mapSipFoundry2log4j(String level)
   {
      if (level.equalsIgnoreCase("DEBUG"))
         return Level.DEBUG ;
      if (level.equalsIgnoreCase("INFO"))
         return Level.INFO ;
      if (level.equalsIgnoreCase("NOTICE"))
         return Level.INFO ;
      if (level.equalsIgnoreCase("WARNING"))
         return Level.WARN ;
      if (level.equalsIgnoreCase("ERR"))
         return Level.ERROR ;
      if (level.equalsIgnoreCase("ERROR"))
         return Level.ERROR ;
      return Level.toLevel(level) ;
   }
   
   /**
    * Escape any CR or LF in the message with the \r \n escapes.
    * SipFoundry logging logs multiline messages (like a SIP PDU) on a single log entry
    * by escaping the CRs and LFs
    * 
    * @param msg The message to escape
    * @return The escaped message
    */
    String escapeCrlf(String msg)
   {
       if (msg == null)
       {
          return null ;
       }
       
/*    
 * This is too slow.  Use the non-regex version  
 * 
      StringBuffer sb = new StringBuffer(msg.length()+2) ;
      // TODO  remove final newline
      Matcher m = ctrl.matcher(msg) ;
      while(m.find())
      {
         m.appendReplacement(sb, "") ;
         String crlf = m.group() ;
         if (crlf.equals("\r"))
         {
            sb.append("\\r") ;   // CR becomes \r
         }
         else if (crlf.equals("\n"))
         {
            sb.append("\\n") ;   // LF becomes \n
         }
      }
      m.appendTail(sb) ;
      return sb.toString() ;
*/      
      int n = msg.length() ;
      
      // Ignore trailing CR LFs
      for(int i=n-1; i>0; i--)
      {
         char c = msg.charAt(i) ;
         if (c == '\r' || c == '\n')
         {
            n-- ;
            continue ;
         }
         break ;
      }
      
      // escape CR LFs
      StringBuffer sb = new StringBuffer(n+2) ;
      for(int i=0; i<n; i++)
      {
         char c = msg.charAt(i) ;
         if (c == '\r')
         {
            sb.append("\\r") ;
         }
         else if (c == '\n')
         {
            sb.append("\\n") ;
         }
         else 
         {
            sb.append(c) ;
         }
      }
      return sb.toString() ;
   }
   

   @Override
   public String format(LoggingEvent arg0)
   {
      String msg = escapeCrlf(arg0.getRenderedMessage()) ;
      String loggerNames[]  = arg0.getLoggerName().split("[.]") ;
      String loggerName = loggerNames[loggerNames.length-1] ;
      
/*
      // syslog2siptrace needs these facilities, and this
      // is a cheap hack to get them!
Actually, syslog2siptrace needs to know ip addrs and ports from
the messages, and that info ain't there at the moment.  So just
ignore this for now...
      String localFacility = facility ;
      if (msg.contains(">>>>>>>>"))
         localFacility = "OUTGOING" ;
      if (msg.contains("<<<<<<<<<"))
         localFacility = "INCOMING" ;
*/

      // lineNumber is static across all loggers, so must be mutex protected.
      // time should also increase monotonically, so hold the lock
      synchronized (lineNumber)
      {
         lineNumber++ ;
         String out1 = String.format("\"%s\":%d:%s:%s:%s:%s:%s:%s:\"%s\"%n", 
               dateFormat.format(System.currentTimeMillis()),
               lineNumber,                 // line number
               facility,                     // Facility
               mapLevel2SipFoundry(arg0.getLevel()),   // msg priority (DEBUG, WARN, etc.)
               hostName,                     // Name of this machine
               arg0.getThreadName(),         // Thread that called log
               "00000000",                   // Thread Id (not useful in Java)
               loggerName,                   // Name of the logger
               msg) ;                        // The message itself (w CRLF escaped)
         return out1 ;
      }
   }

   @Override
   public boolean ignoresThrowable()
   {
      return true;
   }

   @Override
   public void activateOptions()
   {
      // No options
      return ;
   }

   public void setFacility(String facility)
   {
      this.facility = facility ;
   }
   
   public String getFacility()
   {
      return facility ;
   }
}
