package org.sipfoundry.openfire.config;

import org.apache.commons.digester.Digester;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePluginException;
import org.xml.sax.InputSource;

public class ConfigurationParser {
    private static final String WATCHER_CONFIG = "sipxopenfire-config";
    
    static {
        Logger logger = Logger.getLogger(Digester.class);
        logger.addAppender(new ConsoleAppender(new SimpleLayout()));
        logger.setLevel(Level.OFF);
        logger = Logger.getLogger("org.apache.commons.beanutils");
        logger.addAppender(new ConsoleAppender(new SimpleLayout()));
        logger.setLevel(Level.OFF);
    }
    /*
    * Add the digester rules.
    * 
    * @param digester
    */
   private static void addRules(Digester digester) throws Exception {
     
       digester.setUseContextClassLoader(true);
       digester.addObjectCreate(WATCHER_CONFIG, WatcherConfig.class.getName());
       digester.addCallMethod(String.format("%s/%s", WATCHER_CONFIG, "sipx-proxy-domain"),
               "setProxyDomain", 0);

       digester.addCallMethod(String.format("%s/%s", WATCHER_CONFIG, "sipx-proxy-port"),
               "setProxyPort", 0, new Class[] {
                   Integer.class
               });

       digester.addCallMethod(String.format("%s/%s", WATCHER_CONFIG, "resource-list"),
               "setResourceList", 0);
       
       digester.addCallMethod(String.format("%s/%s", WATCHER_CONFIG, "user-name"),
               "setUserName", 0);
       digester.addCallMethod(String.format("%s/%s", WATCHER_CONFIG, "password"),
               "setPassword", 0);
       digester.addCallMethod(String.format("%s/%s", WATCHER_CONFIG, "watcher-address"),
               "setWatcherAddress", 0);
       digester.addCallMethod(String.format("%s/%s", WATCHER_CONFIG, "watcher-port"),
               "setWatcherPort", 0, new Class[] {
                   Integer.class
               });
       digester.addCallMethod(String.format("%s/%s", WATCHER_CONFIG, "log-level"),
               "setLogLevel", 0);
      
       digester.addCallMethod(String.format("%s/%s", WATCHER_CONFIG, "log-directory"),
               "setLogDirectory", 0);
       
       digester.addCallMethod(String.format("%s/%s", WATCHER_CONFIG, "openfire-xml-rpc-port"),
               "setOpenfireXmlRpcPort", 0, new Class[] {
                   Integer.class
               });
       digester.addCallMethod(String.format("%s/%s", WATCHER_CONFIG, "openfire-host"),
               "setOpenfireHost", 0);
      
      
    }
   
   public WatcherConfig parse(String url) {
       // Create a Digester instance
       Digester digester = new Digester();

       //digester.setSchema("file:schema/sipxcallwatcher.xsd");

       
       try {

           addRules(digester);
       
           InputSource inputSource = new InputSource(url);
           digester.parse(inputSource);
           WatcherConfig watcherConfig = (WatcherConfig) digester.getRoot();
           return watcherConfig;
       } catch (Exception ex) {
           
           System.err.println(ex);
           throw new SipXOpenfirePluginException(ex);
       }
      
   }

   
    
}
