/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.voicemail;

import java.io.File;
import java.util.Vector;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * Holds the configuration data needed for VoiceMail.
 * 
 */
public class Configuration {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    private int m_initialTimeout; // initial digit timeout (mS)
    private int m_interDigitTimeout; // subsequent digit timeout (mS)
    private int m_extraDigitTimeout; // extra (wait for #) (mS)
    private int m_maximumDigits; // maximum extension length
    private int m_noInputCount; // give up after this many timeouts
    private int m_invalidResponseCount; // give up after this many bad entries
    private boolean m_transferOnFailures; // What to do on failure
    private String m_transferPrompt; // What to say
    private String m_transferUrl; // Where to go

    public int getInitialTimeout() {
        return m_initialTimeout;
    }

    public int getInterDigitTimeout() {
        return m_interDigitTimeout;
    }

    public int getExtraDigitTimeout() {
        return m_extraDigitTimeout;
    }

    public int getMaximumDigits() {
        return m_maximumDigits;
    }

    public int getNoInputCount() {
        return m_noInputCount;
    }

    public int getInvalidResponseCount() {
        return m_invalidResponseCount;
    }

    public boolean isTransferOnFailure() {
        return m_transferOnFailures;
    }

    public String getTransferPrompt() {
        return m_transferPrompt;
    }

    public String getTransferURL() {
        return m_transferUrl;
    }

    private static Configuration s_current;
    private static File s_configFile;
    private static long s_lastModified;

    /**
     * Private constructor for updatable singleton
     */
    private Configuration() {
        // Global defaults if otherwise not specified
        m_initialTimeout = 7000;
        m_interDigitTimeout = 3000;
        m_extraDigitTimeout = 3000;
        m_maximumDigits = 10;
        m_noInputCount = 2;
        m_invalidResponseCount = 2;
    }

    /**
     * Load new Configuration object if the underlying properties files have changed since the
     * last time.
     * 
     * @return
     */
    public static Configuration update(boolean load) {
        if (s_current == null || s_configFile.lastModified() != s_lastModified) {
            s_current = new Configuration();
            if (load) {
                s_current.loadXML();
            }
        }
        return s_current;
    }

    /**
     * Load the voicemail.xml file
     * NB: There is currently NO voicemail.xml file, this is just a placeholder
     */
    void loadXML() {
    	return ;

/*
    	LOG.info("Loading voicemail.xml configuration");
        String path = System.getProperty("conf.dir");
        if (path == null) {
            LOG.fatal("Cannot get System Property conf.dir!  Check jvm argument -Dconf.dir=") ;
            System.exit(1);
        }
        
        Document voiceMailDoc = null;
        
        try {
            s_configFile = new File(path + "/voicemail.xml");
            s_lastModified = s_configFile.lastModified();

            DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
            voiceMailDoc = builder.parse(s_configFile);
            
        } catch (Throwable t) {
            LOG.fatal("Something went wrong loading the voicemail.xml file.", t);
            System.exit(1);
        }
*/
    }


}
