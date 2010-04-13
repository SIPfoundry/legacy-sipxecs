/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package sipxpage;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;
import java.util.Vector;

/**
 * Holds the configuration data neede for the Page Server.
 *
 * @author Woof!
 *
 */
public class Configuration
{

   class PageGroupConfig {
      String description ;    // A description of this group (just for logging)
      String user ;           // The sip user part that pages this group
      String beep ;           // The audio file to use as a beep
      String urls ;           // A comma seperated list of URLs to page
      int maximumDuration;    // The page timeout (in mS) (<=0 means no timeout)
   }

   String logLevel ;          // The desired logging level in SipFoundry format (not log4j!)
   String traceLevel ;         // The NIST SIP stack trace level (optional)
   String logFile ;           // The file to log into
   String ipAddress ;		   // The IP address (dotted quad string)
   int udpSipPort ;           // The SIP Listen port for UDP
   int tcpSipPort ;           // The SIP Listen port for TCP
   int tlsSipPort ;           // The SIP Listen port for TLS
   int startingRtpPort ;      // The starting RTP port range (4 per page group)

   Vector<PageGroupConfig> pageGroups ;

   Configuration()
   {
      properties() ;
   }

   void internal()
   {
      logLevel = "DEBUG" ;
      logFile = "./sipxpage.log" ;
      ipAddress = "10.1.1.151" ;
      udpSipPort = 5050 ;
      tcpSipPort = 5050 ;
      tlsSipPort = 5051 ;
      startingRtpPort = 4242 ;

      pageGroups = new Vector<PageGroupConfig>() ;
      PageGroupConfig p = new PageGroupConfig() ;
      p.description= "First Group" ;
      p.beep = "/home/woof/Downloads/PagerAudio/TadaTada.wav" ;
      p.user = "1" ;
      p.urls = "200@cdhcp151.pingtel.com" ;
      pageGroups.add(p) ;

      p = new PageGroupConfig() ;
      p.description = "Number Two" ;
      p.beep = "/home/woof/Downloads/PagerAudio/fanfare.wav" ;
      p.user = "42" ;
      p.urls = "200@cdhcp151.pingtel.com,203@cdhcp151.pingtel.com,204@cdhcp151.pingtel.com" ;
      pageGroups.add(p) ;

      // Create a master "page all" group from all the urls in the other groups
      // (with no duplicates)
      StringBuffer urls = new StringBuffer() ;
      Vector<String> set = new Vector<String>() ;
      for(PageGroupConfig pc : pageGroups)
      {
         for (String dest : pc.urls.split(","))
         {
            // Use the Vector to determine if the url
            // already exists in the list.
            if (set.contains(dest) == false)
            {
               set.add(dest) ;
               urls.append(dest) ;
               urls.append(",") ;
            }
         }
      }
      urls.deleteCharAt(urls.length()-1) ; // Remove trailing comma

      p = new PageGroupConfig() ;
      p.description = "Page All" ;
      p.beep = "/home/woof/Downloads/PagerAudio/attention.wav" ;
      p.user = "*" ;
      p.urls = urls.toString() ;
      pageGroups.add(p) ;
   }

   void properties()
   {
      String path = System.getProperty("conf.dir") ;
      if (path == null)
      {
         path="./etc" ;
      }
      String name = "sipxpage.properties" ;
      InputStream inStream ;
      Properties props = null ;
      try
      {
         inStream = new FileInputStream(path+"/"+name) ;
         props = new Properties() ;
         props.load(inStream) ;
      } catch (FileNotFoundException e)
      {
         e.printStackTrace() ;
         System.exit(1) ;
      } catch (IOException e)
      {
         e.printStackTrace();
         System.exit(1) ;
      }

      String prop = null ;
      try
      {
         logLevel = props.getProperty(prop="log.level") ;
         logFile = props.getProperty(prop="log.file") ;

         ipAddress = props.getProperty(prop="sip.address") ;
         udpSipPort = Integer.parseInt(props.getProperty(prop="sip.udpPort")) ;
         tcpSipPort = Integer.parseInt(props.getProperty(prop="sip.tcpPort")) ;
         tlsSipPort = Integer.parseInt(props.getProperty(prop="sip.tlsPort")) ;
         traceLevel = props.getProperty(prop="sip.trace") ;
         if (traceLevel == null)
         {
            traceLevel = "NONE" ;
         }

         startingRtpPort = Integer.parseInt(props.getProperty(prop="rtp.port"));

         pageGroups = new Vector<PageGroupConfig>() ;
         for(int i=1;;i++)
         {
            PageGroupConfig p = new PageGroupConfig() ;
            String prefix = String.format("page.group.%d.", i) ;
            String temp = null ;

            p.description= props.getProperty(prop=prefix+"description") ;
            if (p.description == null)
               break ;

            p.beep = props.getProperty(prop=prefix+"beep") ;
            p.user = props.getProperty(prop=prefix+"user") ;
            p.urls = props.getProperty(prop=prefix+"urls") ;
            temp = props.getProperty(prop=prefix+"timeout") ;
            if (temp != null)
            {
               p.maximumDuration = Integer.parseInt(temp) ;
            }

            pageGroups.add(p) ;
         }
      }
      catch (Exception e)
      {
         System.err.println("Problem understanding property "+prop) ;
         e.printStackTrace();
         System.exit(1) ;
      }
   }
}
