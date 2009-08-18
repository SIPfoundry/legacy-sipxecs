package org.sipfoundry.sipcallwatcher;

import java.io.File;
import java.io.FileInputStream;
import java.util.Properties;

import javax.sip.ListeningPoint;
import javax.sip.SipProvider;

import org.apache.log4j.Logger;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.PatternLayout;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.openfire.config.ConfigurationParser;
import org.sipfoundry.openfire.config.WatcherConfig;


public class CallWatcher 
{
    // //////////////// //
    // static variables //
    // //////////////// //	
	private static SipFoundryAppender logAppender;
	private static WatcherConfig watcherConfig;
	private static SipProvider sipProvider;
	private static String configurationPath = "/etc/sipxpbx/";
    private static String configurationFile;
    private static String logFile;
    private static ListeningPoint udpListeningPoint;
    private static ListeningPoint tcpListeningPoint;
    private static Subscriber subscriber;
    
    /**
     * @return the sipProvider
     */
    static SipProvider getSipProvider() 
    {
        return sipProvider;
    }

    public static WatcherConfig getConfig() 
    {
	    return watcherConfig;
	}
	
    static void parseConfigurationFile() 
    {   
        configurationFile = CallWatcher.configurationPath + "/sipxopenfire.xml";

        if (!new File(CallWatcher.configurationFile).exists()) {
            System.err.println( String.format("Configuration %s file not found -- exiting",
                    CallWatcher.configurationFile) );
            System.exit(-1);
        }
        ConfigurationParser parser = new ConfigurationParser();
        watcherConfig  = parser.parse( "file://" + configurationFile );
    }
    
    static void initializeJainSip() throws Exception {
        ProtocolObjects.init();
        udpListeningPoint = ProtocolObjects.getSipStack().createListeningPoint(watcherConfig.getWatcherAddress(), watcherConfig.getWatcherPort(), "udp");
        tcpListeningPoint = ProtocolObjects.getSipStack().createListeningPoint(watcherConfig.getWatcherAddress(), watcherConfig.getWatcherPort(), "tcp");
        sipProvider = ProtocolObjects.getSipStack().createSipProvider(udpListeningPoint);
        subscriber = new Subscriber( sipProvider,  watcherConfig );
        sipProvider.addSipListener( subscriber );      
        ProtocolObjects.start();
    }
	
    static void initializeLogging() throws Exception {
        try {
            String javaClassPaths = System.getProperty("java.class.path");
            String openfireHome = System.getProperty("openfireHome");
            StringBuilder sb = new StringBuilder(javaClassPaths).append(":" + openfireHome+ "/lib/sipxcommons.jar");
            System.setProperty("java.class.path",sb.toString());
            String log4jPropertiesFile = configurationPath + "/log4j.properties";

            if (new File(log4jPropertiesFile).exists()) 
            {
                /*
                 * Override the file configuration setting.
                 */
                Properties props = new Properties();
                props.load(new FileInputStream(log4jPropertiesFile));
                String level = props.getProperty("log4j.category.org.sipfoundry.sipxcallwatcher");
                if (level != null) 
                {
                	watcherConfig.setLogLevel(level);
                }
            }
            logAppender = new SipFoundryAppender(new SipFoundryLayout(), logFile );
            Logger applicationLogger = Logger.getLogger(CallWatcher.class.getPackage().getName());

            /*
             * Set the log level.
             */
            if (watcherConfig.getLogLevel().equals("TRACE")) {
                applicationLogger.setLevel(org.apache.log4j.Level.DEBUG);
            } else {
                applicationLogger.setLevel(org.apache.log4j.Level.toLevel(watcherConfig.getLogLevel()));
            }

            applicationLogger.addAppender(logAppender);
            if( System.getProperty( "output.console" ) != null )
            {
            	applicationLogger.addAppender( new ConsoleAppender( new PatternLayout() ) );
            }
        } catch (Exception ex) 
        {
            throw new Exception(ex);
        }
    }
    
    /**
     * Entry point to get things going from the openfire plugin.
     * 
     * @throws Exception
     */
    
    public static void init() throws Exception {
        System.out.println("path is " + CallWatcher.configurationPath );
        CallWatcher.parseConfigurationFile();
        logFile = watcherConfig.getLogDirectory() + "/sipxcallwatcher.log";
        
        //Initialize the logging subsystem
        initializeLogging();
        //Create Listening Point
        initializeJainSip();
        subscriber.start();
    }

    /**
     * Entry point for initialization when it is called from a plugin.
     * Logging initialization is done in the plugin so we dont call it here.
     * NOTE : the main init method should be removed later.
     * 
     * @throws Exception
     */
    public static void pluginInit() throws Exception  {
        //Create Listening Point
        initializeJainSip();
        subscriber.start();
    }
    
    
    public static void setConfigurationPath(String configurationPath) {
        CallWatcher.configurationPath = configurationPath;     
    }  
    


    /**
     * @return the logAppender
     */
    public static SipFoundryAppender getLogAppender() {
        return logAppender;
    }
    
    
    

    public static void setLogAppender(SipFoundryAppender logAppender) {
       CallWatcher.logAppender = logAppender;
        
    }

    public static void setWatcherConfig(WatcherConfig watcherConfig) {
       
        CallWatcher.watcherConfig = watcherConfig;      
    }
    
    /**
     * @return the subscriber
     */
    public static Subscriber getSubscriber() {
        return subscriber;
    }
    
    
    public static void main(String[] args) throws Exception
    {
        CallWatcher.configurationPath = System.getProperty("conf.dir","/etc/sipxpbx");
        init();
    }
}
