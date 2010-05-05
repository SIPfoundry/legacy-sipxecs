/**
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.commons.sipXecsService;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.sipfoundry.commons.log4j.SipFoundryLayout;

import java.io.*;
import java.util.NoSuchElementException;
import java.util.Properties;
import java.util.StringTokenizer;

/**
 * SipXecsService: superclass for common features of all sipXecs services
 *
 * This class provides for the common features of sipXecs service processes.
 * It sets up the proper signal handling and listens for change notifications from the supervisor.
 * It invokes change callbacks when config files change which subclasses can override to
 * make changes dynamically.
 * It provides a shutdown method which subclasses can override to perform a graceful shutdown.
 * Classes which extend this class should call setLogLevel as soon as possible to
 * set the logLevel to the configured value (in the constructor or very soon after).
 *
 * @author Carolyn Beeton
 */

public abstract class SipXecsService implements StdinListener, Runnable {
    static final Logger LOG = Logger.getLogger("org.sipfoundry");

    private BufferedReader stdIn;
    private String mServiceName;

   public SipXecsService(String serviceName) {
       mServiceName = serviceName;

       // Load the configuration
       String path = System.getProperty("log.dir") ;
       if (path == null)
       {
          path="." ;
       }
       // Configure log4j
       Properties props = new Properties() ;
       props.setProperty("log4j.rootLogger","warn, file") ;
       props.setProperty("log4j.logger.org.sipfoundry",
               SipFoundryLayout.mapSipFoundry2log4j("NOTICE").toString()) ;
       props.setProperty("log4j.appender.file", "org.sipfoundry.commons.log4j.SipFoundryAppender") ;
       props.setProperty("log4j.appender.file.File", path + "/" + mServiceName + ".log") ;
       props.setProperty("log4j.appender.file.layout","org.sipfoundry.commons.log4j.SipFoundryLayout") ;
       props.setProperty("log4j.appender.file.layout.facility", mServiceName) ;
       PropertyConfigurator.configure(props) ;
       LOG.info(mServiceName + " >>>>>>>>>>>>>>>> STARTED");

       // spawn thread to listen to stdin, and call gotInput when there is input
       stdIn = new BufferedReader(new InputStreamReader(System.in));
       Thread t = new Thread(this);
       t. start();
   }

   public void run() {
       String input;

       try {
           while ((input = stdIn.readLine()) != null) {
               gotInput(input);
           }
       } catch (IOException e) {
           LOG.error(mServiceName + " caught IOException", e);
           e.printStackTrace();
       }
   }

   /// Set the loglevel for sipfoundry loggers to the specified level
   // @param level String specifying a sipfoundry loglevel (DEBUG, INFO, FATAL, WARNING, ERROR, CRIT)
   public void setLogLevel(String level) {
       Properties props = new Properties() ;
       props.setProperty("log4j.logger.org.sipfoundry",
               SipFoundryLayout.mapSipFoundry2log4j(level).toString());
       PropertyConfigurator.configure(props) ;
       LOG.log(SipFoundryLayout.mapSipFoundry2log4j(level), "Log level set to " + level);
   }

   /// A config file has changed.
   //  Child processes can implement this to handle the config changes as they wish.
   //  Note that this method is called in the StdinListener thread, so the implementation must be threadsafe.
   public abstract void resourceChanged(String fileType, String configFile);

   /// The child process must exit.
   //  Child processes can override this to perform whatever shutdown is required.
   public void shutdown() {
       LOG.info("<<<<<<<<<<<<<<<< " + mServiceName + " stopped");
       System.exit(0);
   }

   /// Process input that has been received
   //  (StdinListener interface)
   public void gotInput(String stdinMsg) {
      StringTokenizer st = new StringTokenizer(stdinMsg);
      String command = "";
      if (st.hasMoreTokens()) {
          command = st.nextToken();
      }
      if (command.equals("CONFIG_CHANGED")) {
          try {
              String fileType = st.nextToken();
              String file = st.nextToken().replaceAll("'", "");
              resourceChanged(fileType, file);
          } catch (NoSuchElementException e) {
              LOG.error("SipXecsService::gotInput: " + stdinMsg + " bad format (ignored)");
          }
      } else if (command.equals("SHUTDOWN")) {
          shutdown();
      } else {
          LOG.warn("SipXecsService::gotInput: unrecognized command: " + command + "(" + stdinMsg);
      }

   }
}
