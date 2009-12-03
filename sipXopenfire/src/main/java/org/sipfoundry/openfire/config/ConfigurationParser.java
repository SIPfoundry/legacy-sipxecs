package org.sipfoundry.openfire.config;

import org.apache.commons.digester.Digester;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.SimpleLayout;
import org.sipfoundry.openfire.plugin.presence.SipXOpenfirePluginException;
import org.xml.sax.InputSource;

public class ConfigurationParser {
	public static final String WATCHER_CONFIG = "sipxopenfire-config";
	public static final String S2S_INFO = "server-to-server";
	public static final String S2S_ALLOWED_SERVERS = "allowed-servers";
	public static final String S2S_DISALLOWED_SERVERS = "disallowed-servers";
	public static String s2sTag = String.format("%s/%s", WATCHER_CONFIG, S2S_INFO);
	public static String s2sAllowedServersTag = String.format("%s/%s", s2sTag, S2S_ALLOWED_SERVERS);
	public static String s2sDisallowedServersTag = String.format("%s/%s", s2sTag, S2S_DISALLOWED_SERVERS);
    
    private static String currentTag = null;
    private static Digester digester;

    static {
        Logger logger = Logger.getLogger(Digester.class);
        logger.addAppender(new ConsoleAppender(new SimpleLayout()));
        logger.setLevel(Level.OFF);
        logger = Logger.getLogger("org.apache.commons.beanutils");
        logger.addAppender(new ConsoleAppender(new SimpleLayout()));
        logger.setLevel(Level.OFF);
    }

    private static void addCallMethodString(String elementName,
			String methodName) {
		digester.addCallMethod(String.format("%s/%s", currentTag, elementName),
				methodName, 0);
	}

	private static void addCallMethodInt(String elementName, String methodName) {
		digester.addCallMethod(String.format("%s/%s", currentTag, elementName),
				methodName, 0, new Class[] { Integer.class });
	}
    
    /*
    * Add the digester rules.
    * 
    * @param digester
    */
   private static void addRules(Digester digester) throws Exception {

	   digester.setUseContextClassLoader(true);
       digester.addObjectCreate(WATCHER_CONFIG, WatcherConfig.class.getName());
       
       digester.addObjectCreate(s2sTag, XmppS2sInfo.class.getName());
       digester.addSetNext(s2sTag, "addS2sInfo");
       
       digester.addObjectCreate(s2sAllowedServersTag, XmppS2sPolicy.class.getName());
       digester.addSetNext(s2sAllowedServersTag, "addS2sAllowedPolicy");
       
       digester.addObjectCreate(s2sDisallowedServersTag, XmppS2sPolicy.class.getName());
       digester.addSetNext(s2sDisallowedServersTag, "addS2sDisallowedPolicy");

       currentTag = WATCHER_CONFIG;
       addCallMethodString("password", "setPassword");
       addCallMethodString("sipx-proxy-domain", "setProxyDomain");
       addCallMethodInt("sipx-proxy-port", "setProxyPort");
       addCallMethodString("resource-list", "setResourceList");
       addCallMethodString("user-name", "setUserName");
       addCallMethodString("password", "setPassword");
       addCallMethodString("watcher-address", "setWatcherAddress");
       addCallMethodInt("watcher-port", "setWatcherPort");
       addCallMethodString("log-level", "setLogLevel");
       addCallMethodString("log-directory", "setLogDirectory");
       addCallMethodInt("openfire-xml-rpc-port", "setOpenfireXmlRpcPort");
       addCallMethodString("openfire-host", "setOpenfireHost");
       addCallMethodString("sipxrest-ip-address", "setSipXrestIpAddress");
       addCallMethodInt("sipxrest-https-port", "setSipXrestHttpsPort");
       addCallMethodInt("sipxrest-external-http-port", "setSipXrestHttpPort");
       addCallMethodString("IM-message-logging", "setImMessageLogging");
       addCallMethodString("IM-message-logging-directory", "setImMessageLoggingDirectory");

       currentTag = s2sTag;
       addCallMethodString("enabled", "setS2sServerActive");
       addCallMethodInt("port", "setS2sRemotePort");
       addCallMethodString("disconnect-on-idle", "setS2sDisconnectOnIdle");
       addCallMethodInt("idle-timeout", "setS2sSessionIdleTimeInMinutes");
       addCallMethodString("any-can-connect", "setS2sAnyCanConnect");

       currentTag = s2sAllowedServersTag;
       addCallMethodString("host", "setXmppDomainName");
       addCallMethodInt("port", "setXmppServerPort");

       currentTag = s2sDisallowedServersTag;
       addCallMethodString("host", "setXmppDomainName");
       addCallMethodInt("port", "setXmppServerPort");
    }
   
   public WatcherConfig parse(String url) {

       digester = new Digester();

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
