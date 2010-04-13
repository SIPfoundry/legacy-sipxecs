/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxrecording;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.freeswitch.FreeSwitchConfigurationInterface;

/**
 * Holds the configuration data needed for sipXrecording.
 * 
 */
public class RecordingConfiguration implements FreeSwitchConfigurationInterface {

    private static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxrecording");
    private String m_logLevel; // The desired logging level in SipFoundry format (not log4j!)
    private String m_logFile; // The file to log into
    private String m_docDirectory; // File path to DOC Directory (usually /usr/share/www/doc)
    private String m_sipxchangeDomainName; // The domain name of this system
    private String m_realm;
    private static final int PORT_NONE = -1;
    
    private static RecordingConfiguration s_current;
    private static File s_propertiesFile;
    private static long s_lastModified;
    
    private RecordingConfiguration() {
    }
    
    public static RecordingConfiguration get() {
        return update(true);
    }
    
    public static RecordingConfiguration getTest() {
        return update(false);
    }
    
    /**
     * Load new Configuration object if the underlying properties files have changed since the last
     * time
     * 
     * @return
     */
    private static RecordingConfiguration update(boolean load) {
        if (s_current == null || s_propertiesFile != null && s_propertiesFile.lastModified() != s_lastModified) {
            s_current = new RecordingConfiguration();
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
        
        String name = "sipxrecording.properties";
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
            m_docDirectory = props.getProperty(prop = "recording.docDirectory") ;
            m_sipxchangeDomainName = props.getProperty(prop = "recording.sipxchangeDomainName");
            m_realm = props.getProperty(prop ="recording.realm");
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
        return PORT_NONE;
    }

    public String getDocDirectory() {
        return m_docDirectory;
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

	@Override
	public Logger getLogger() {
		return LOG;
	}
}
