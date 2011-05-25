/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.voicemail;

import java.io.File;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxivr.ApplicationConfiguraton;

/**
 * Holds the configuration data needed for VoiceMail.
 * 
 */
public class Configuration extends ApplicationConfiguraton {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    private static Configuration s_current;
    private static File s_configFile;
    private static long s_lastModified;

    /**
     * Private constructor for updatable singleton
     */
    private Configuration() {
        super();
    }

    /**
     * Load new Configuration object if the underlying properties files have changed since the
     * last time.
     * 
     * @return
     */
    public static Configuration update(boolean load) {
/*        
        if (s_current == null || s_configFile.lastModified() != s_lastModified) {
            s_current = new Configuration();
            if (load) {
                s_current.loadXML();
            }
        }
*/
        if (s_current == null)
            s_current = new Configuration();
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
