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

/**
 * Holds the configuration data needed for sipXivr.
 * 
 */
public class Configuration {

    private String m_logLevel; // The desired logging level in SipFoundry format (not log4j!)
    private String m_logFile; // The file to log into
    private int m_eventSocketPort; // The Event Socket Listen port
    private String m_mailstoreDirectory; // File path to the mailstore
    private String m_organizationPrefs; // File path to organizationprefs.xml
    private String m_scriptsDirectory; // File path to the AA scripts (for schedule access)
    private String m_docDirectory; // File path to DOC Directory (usually /usr/share/www/doc)
    private String m_operatorAddr; // Address of 'operator'
    private String m_sipxchangeDomainName; // The domain name of this system (from config.defs?)
    private String m_voicemailUrl; // URL for voicemail access

    private static Configuration s_current;
    private static File s_propertiesFile;
    private static long s_lastModified;
    
    private Configuration() {
    }
    
    /**
     * Load new Configuration object if the underlying properties files have changed since the last
     * time
     * 
     * @return
     */
    public static Configuration update(boolean load) {
        if (s_current == null || s_propertiesFile.lastModified() != s_lastModified) {
            s_current = new Configuration();
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
            m_mailstoreDirectory = props.getProperty(prop = "ivr.mailstoreDirectory");
            m_organizationPrefs = props.getProperty(prop = "ivr.organizationPrefs");
            m_scriptsDirectory = props.getProperty(prop = "ivr.scriptsDirectory");
            m_docDirectory = props.getProperty(prop = "ivr.docDirectory") ;
            m_operatorAddr = props.getProperty(prop = "ivr.operatorAddr");
            m_sipxchangeDomainName = props.getProperty(prop = "ivr.sipxchangeDomainName");
            m_voicemailUrl = props.getProperty(prop = "ivr.voicemailUrl");
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

    public String getMailstoreDirectory() {
        return m_mailstoreDirectory;
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

    public String getOperatorAddr() {
        return m_operatorAddr;
    }

    public String getSipxchangeDomainName() {
        return m_sipxchangeDomainName;
    }

    public String getVoicemailUrl() {
        return m_voicemailUrl;
    }

    public void setVoicemailUrl(String voicemailUrl) {
        m_voicemailUrl = voicemailUrl;
    }
}
