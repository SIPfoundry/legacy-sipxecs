/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.FreeSwitchConfigurationInterface;

/**
 * Holds the configuration data needed for sipXivr.
 * 
 */
public class IvrConfiguration implements FreeSwitchConfigurationInterface {

	private static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");
    private String m_logLevel; // The desired logging level in SipFoundry format (not log4j!)
    private String m_logFile; // The file to log into
    private int m_eventSocketPort; // The Event Socket Listen port
    private String m_dataDirectory; // File path to the media server data directory
    private String m_mailstoreDirectory; // File path to the mailstore
    private String m_promptsDirectory; // File path to the AA prompts
    private String m_organizationPrefs; // File path to organizationprefs.xml
    private String m_scriptsDirectory; // File path to the AA scripts (for schedule access)
    private String m_docDirectory; // File path to DOC Directory (usually /usr/share/www/doc)
    private String m_operatorAddr; // Address of 'operator'
    private String m_sipxchangeDomainName; // The domain name of this system
    private String m_realm;
    private String m_mwiUrl; // The url of the Status Server we send MWI requests to
    private String m_configUrl; // The url of the Config Server for PIN change requests
    private int m_httpsPort; // The port on which we listen for HTTPS services
    private String m_3pccSecureUrl; // The url of the third party call controller via HTTPS
    private String m_sendIMUrl;
    private String m_openfireHost; // The host name where the Openfire service runs
    private int m_openfireXmlRpcPort; // The port number to use for XML-RPC Openfire requests
    
    private boolean m_CPUIisPrimary;
    private String  m_nameDialPrefix;
    
    private static IvrConfiguration s_current;
    private static File s_propertiesFile;
    private static long s_lastModified;
    
    private IvrConfiguration() {
    }
    
    public static IvrConfiguration get() {
        return update(true);
    }
    
    public static IvrConfiguration getTest() {
        return update(false);
    }
    
    /**
     * Load new Configuration object if the underlying properties files have changed since the last
     * time
     * 
     * @return
     */
    private static IvrConfiguration update(boolean load) {
        if (s_current == null || s_propertiesFile != null && s_propertiesFile.lastModified() != s_lastModified) {
            s_current = new IvrConfiguration();
            if (load) {
                s_current.properties();
            }
        }
        return s_current;
    }
    
    void properties() {
        String path = System.getProperty("conf.dir");
        if (path == null) {
            System.err.println("Cannot get System Property conf.dir!  Check jvm argument -Dconf.dir=") ;
            System.exit(1);
        }
        
        // Setup SSL properties so we can talk to HTTPS servers
        String keyStore = System.getProperty("javax.net.ssl.keyStore");
        if (keyStore == null) {
            // Take an educated guess as to where it should be
            keyStore = path+"/ssl/ssl.keystore";
            System.setProperty("javax.net.ssl.keyStore", keyStore);
            System.setProperty("javax.net.ssl.keyStorePassword", "changeit"); // Real security!
 //           System.setProperty("java.protocol.handler.pkgs", "com.sun.net.ssl.internal.www.protocol");
 //           System.setProperty("javax.net.debug", "ssl");
        }
        String trustStore = System.getProperty("javax.net.ssl.trustStore");
        if (trustStore == null) {
            // Take an educated guess as to where it should be
            trustStore = path+"/ssl/authorities.jks";
            System.setProperty("javax.net.ssl.trustStore", trustStore);
            System.setProperty("javax.net.ssl.trustStoreType", "JKS");
            System.setProperty("javax.net.ssl.trustStorePassword", "changeit"); // Real security!
        }
        
        String name = "sipxivr.properties";
        FileInputStream inStream;
        Properties props = null;
        try {
        	s_propertiesFile = new File(path + "/" + name);
            s_lastModified = s_propertiesFile.lastModified();
            inStream = new FileInputStream(s_propertiesFile);
            props = new Properties();
            props.load(inStream);
            inStream.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace(System.err);
            System.exit(1);
        } catch (IOException e) {
            e.printStackTrace(System.err);
            System.exit(1);
        }

        String prop = null;
        try {
            m_logLevel = props.getProperty(prop = "log.level");
            m_logFile = props.getProperty(prop = "log.file");
            m_eventSocketPort = Integer.parseInt(props
                    .getProperty(prop = "freeswitch.eventSocketPort"));
            m_dataDirectory = props.getProperty(prop = "ivr.dataDirectory");
            m_mailstoreDirectory = props.getProperty(prop = "ivr.mailstoreDirectory");
            m_promptsDirectory = props.getProperty(prop = "ivr.promptsDirectory");
            m_organizationPrefs = props.getProperty(prop = "ivr.organizationPrefs");
            m_scriptsDirectory = props.getProperty(prop = "ivr.scriptsDirectory");
            m_docDirectory = props.getProperty(prop = "ivr.docDirectory") ;
            m_operatorAddr = props.getProperty(prop = "ivr.operatorAddr");
            m_sipxchangeDomainName = props.getProperty(prop = "ivr.sipxchangeDomainName");
            m_realm = props.getProperty(prop ="ivr.realm");
            m_mwiUrl = props.getProperty(prop = "ivr.mwiUrl");
            m_httpsPort = Integer.parseInt(props.getProperty(prop = "ivr.httpsPort"));
            m_configUrl = props.getProperty(prop = "ivr.configUrl");
            m_sendIMUrl = props.getProperty("ivr.sendIMUrl");
            
            String defaultTUI = props.getProperty("ivr.defaultTui");
            if(defaultTUI == null) {
                m_CPUIisPrimary = false;
            } else {
                m_CPUIisPrimary = defaultTUI.equals("cpui");
            }
             
            String nameDialPrefix = props.getProperty("ivr.nameDialPrefix");
            if(nameDialPrefix == null) {
                m_nameDialPrefix = "11";
            } else {
                m_nameDialPrefix = nameDialPrefix;
            }            
            
            // Make up the 3pcc REST server URL for now until it is provided by sipxconfig
            // For now assume that the REST service resides on the same system as this service
            m_3pccSecureUrl = "https://" + m_sipxchangeDomainName + ":6666" + "/callcontroller/";
            
            // Make up the Openfire server hostname and port numbers for now until
            // it is provided by sipxconfig. Assume that the openfire service
            // resides on the same system as this service
            m_openfireHost = m_sipxchangeDomainName;
            m_openfireXmlRpcPort = 9094;
        } catch (Exception e) {
            System.err.println("Problem understanding property " + prop);
            e.printStackTrace(System.err);
            System.exit(1);
        }
    }

    public String getLogLevel() {
        return m_logLevel;
    }

    public String getLogFile() {
        return m_logFile;
    }

    public int getEventSocketPort() {
        return m_eventSocketPort;
    }

    public String getDataDirectory() {
        return m_dataDirectory;
    }
    
    public String getMailstoreDirectory() {
        return m_mailstoreDirectory;
    }

    public String getPromptsDirectory() {
        return m_promptsDirectory;
    }
    
    public String getOrganizationPrefs() {
        return m_organizationPrefs;
    }

    public String getScriptsDirectory() {
        return m_scriptsDirectory;
    }

    public String getDocDirectory() {
        return m_docDirectory;
    }
    
    public String getSendIMUrl() {
        return m_sendIMUrl;
    }

    public String getOperatorAddr() {
        return m_operatorAddr;
    }

    public String getSipxchangeDomainName() {
        return m_sipxchangeDomainName;
    }

    public String getRealm() {
        return m_realm;
    }
    
    public void setRealm(String realm) {
        m_realm = realm;
    }
    
    public String getMwiUrl() {
        return m_mwiUrl;
    }
    
    public void setMwiUrl(String mwiUrl) {
        m_mwiUrl = mwiUrl;
    }

    public String getConfigUrl() {
        return m_configUrl;
    }
    
    public void setConfigUrl(String configUrl) {
        m_configUrl = configUrl;
    }
    
    public int getHttpsPort() {
        return m_httpsPort;
    }
    
    public void setHttpsPort(int httpsPort) {
        m_httpsPort = httpsPort;
    }

    public String get3pccSecureUrl() {
        return m_3pccSecureUrl;
    }
    
    public void set3pccSecureUrl(String url) {
    	m_3pccSecureUrl = url;
    }

    public String getOpenfireHost() {
        return m_openfireHost;
    }
    
    public void setOpenfireHost(String host) {
    	m_openfireHost = host;
    }

    public int getOpenfireXmlRpcPort() {
        return m_openfireXmlRpcPort;
    }
    
    public void setOpenfireXmlRpcPort(int port) {
        m_openfireXmlRpcPort = port;
    }
    
    public boolean isCPUIPrimary() {
        return m_CPUIisPrimary;
    }
    
    public String getCPUINameDialingPrefix() {
        return m_nameDialPrefix;
    }
    
	@Override
	public Logger getLogger() {
		return LOG;
	}
}
