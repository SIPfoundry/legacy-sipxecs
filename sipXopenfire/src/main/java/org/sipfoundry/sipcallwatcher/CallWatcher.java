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
	private static String configurationPath = "/etc/sipxpbx/";
    private static String configurationFile;
    private static SipStackBean sipStackBean;
    
  

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
        sipStackBean = new SipStackBean();
        sipStackBean.init();
        sipStackBean.getSipStack().start();
    }
	
    public static SipStackBean getSipStackBean() {
        return sipStackBean;
    }
  
    
    /**
     * Entry point to get things going from the openfire plugin.
     * 
     * @throws Exception
     */
    
    public static void init() throws Exception {
        System.out.println("path is " + CallWatcher.configurationPath );
        CallWatcher.parseConfigurationFile();   
         //Create Listening Point
        initializeJainSip();
        Thread.sleep(100);
        sipStackBean.getSubscriber().start();
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
        Thread.sleep(100);
        sipStackBean.getSubscriber().start();
       
    }
    
    
    public static void setConfigurationPath(String configurationPath) {
        CallWatcher.configurationPath = configurationPath;     
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
        return sipStackBean.getSubscriber();
    }
    
    
    public static void main(String[] args) throws Exception
    {
        CallWatcher.configurationPath = System.getProperty("conf.dir","/etc/sipxpbx");
        init();
    }
}
