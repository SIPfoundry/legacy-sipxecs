package org.sipfoundry.sipcallwatcher;

import java.io.File;
import java.io.FileInputStream;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;
import org.sipfoundry.commons.log4j.SipFoundryAppender;
import org.sipfoundry.commons.log4j.SipFoundryLayout;
import org.sipfoundry.openfire.config.ConfigurationParser;
import org.sipfoundry.openfire.config.WatcherConfig;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePluginException;


public class CallWatcher 
{
    // //////////////// //
    // static variables //
    // //////////////// //	
	private static WatcherConfig watcherConfig;
    private static String logFile;
	private static String configurationPath;
    private static String configurationFile;
    private static SipStackBean sipStackBean;
    
  

    public static WatcherConfig getConfig() 
    {
	    return watcherConfig;
	}
	
    static void parseConfigurationFile() 
    {   
        configurationFile = CallWatcher.getConfigurationPath() + "/sipxopenfire.xml";

        if (!new File(CallWatcher.configurationFile).exists()) {
            System.err.println( String.format("Configuration %s file not found -- exiting",
                    CallWatcher.configurationFile) );
            System.exit(-1);
        }
        ConfigurationParser parser = new ConfigurationParser();
        watcherConfig  = parser.parse( "file://" + configurationFile );
        logFile = watcherConfig.getLogDirectory() + "/sipxopenfire.log";
    }
    
    
    static void initializeJainSip() throws Exception {
        sipStackBean = new SipStackBean();
        sipStackBean.init();
        sipStackBean.getSipStack().start();
    }
    
    static void destroyJainSip(){
        sipStackBean.getSipStack().stop();
        sipStackBean.destroy();
        CallWatcher.sipStackBean = null;
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
        CallWatcher.parseConfigurationFile();   
        initializeLogging();
        //Create Listening Point
        initializeJainSip();
        sipStackBean.getSubscriber().start();
    }

    static void initializeLogging() {
        try {
            String javaClassPaths = System.getProperty("java.class.path");
            String openfireHome = System.getProperty("openfire.home");
            StringBuilder sb = new StringBuilder(javaClassPaths).append(":" + openfireHome
                    + "/lib/sipxcommons.jar");
            System.setProperty("java.class.path", sb.toString());

            // Configure log4j
            Properties props = new Properties();
            props.setProperty("log4j.rootLogger", "warn, file");
            props.setProperty("log4j.logger.org.sipfoundry.sipcallwatcher",
                    SipFoundryLayout.mapSipFoundry2log4j(watcherConfig.getLogLevel()).toString());
            props.setProperty("log4j.appender.file", SipFoundryAppender.class.getName());
            props.setProperty("log4j.appender.file.File", logFile);
            props.setProperty("log4j.appender.file.layout", SipFoundryLayout.class.getName());
            props.setProperty("log4j.appender.file.layout.facility", "JAVA");
            String log4jProps = getConfigurationPath() + "/log4j.properties";
            PropertyConfigurator.configure(props);

        } catch (Exception ex) {
        }
    }
        
    public static void destroy(){
        sipStackBean.getSubscriber().stop();
        destroyJainSip();        
    }
    
    public static String getConfigurationPath() {
        if( CallWatcher.configurationPath == null ){     
            try {
                if (new File("/tmp/sipx.properties").exists()) {
                    System.getProperties()
                            .load(new FileInputStream(new File("/tmp/sipx.properties")));
                }
            } catch (Exception ex) {
            }
            CallWatcher.configurationPath = System.getProperty("conf.dir", "/etc/sipxpbx");
        }
        return CallWatcher.configurationPath;
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
        init();
    }
}
