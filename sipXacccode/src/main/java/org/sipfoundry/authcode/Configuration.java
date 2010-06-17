/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.authcode;

import java.io.File;
import java.util.HashMap;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.apache.log4j.Logger;
import org.sipfoundry.sipxacccode.ApplicationConfiguraton;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * Holds the configuration data needed for Authorization Codes.
 * 
 */
public class Configuration {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxauthcode");

    private static Configuration s_current;
    private static File s_configFile;
    private static long s_lastModified;

    private HashMap<String, AuthCodeConfig> m_authcodes;

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
        if (s_current == null || s_configFile.lastModified() != s_lastModified) {
            s_current = new Configuration();
            if (load) {
                s_current.loadXML();
            }
        }
        return s_current;

    } 

    public AuthCodeConfig getAuthCode(String id) {
        if (m_authcodes == null || id == null) {
            return null;
        }
        AuthCodeConfig acc = m_authcodes.get(id);
        return acc;

    }
    /**
     * Load the authcodes.xml file
     */
    void loadXML() {
        LOG.debug("Configuration::loadXML Loading authcodes.xml configuration");
        String path = System.getProperty("conf.dir");
        if (path == null) {
            LOG.fatal("Cannot get System Property conf.dir!  Check jvm argument -Dconf.dir=") ;
            System.exit(1);
        }

        Document authCodesDoc = null;

        try {
            s_configFile = new File(path + "/authcodes.xml");
            s_lastModified = s_configFile.lastModified();

            DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
            authCodesDoc = builder.parse(s_configFile);

        } catch (Throwable t) {
            LOG.fatal("Configuration::loadXML Something went wrong loading the authcodes.xml file.", t);
            System.exit(1);
        }

        String prop = null;
        try {
            prop = "unknown";
            m_authcodes = new HashMap<String, AuthCodeConfig>();

            // Walk authcode elements, building up the hash map.
            NodeList authcodes = authCodesDoc.getElementsByTagName(prop = "authcode");
            for (int acNum = 0; acNum < authcodes.getLength(); acNum++) {
                Node authCodeNode = authcodes.item(acNum);
                AuthCodeConfig c = new AuthCodeConfig() ;
                String id = authCodeNode.getAttributes().getNamedItem("code").getNodeValue();
                c.setAuthCode(id);
                for(Node next = authCodeNode.getFirstChild(); next != null; next = next.getNextSibling()) {
                    if (next.getNodeType() == Node.ELEMENT_NODE) {
                        String name = next.getNodeName();
                        if (name.equals(prop = "authname")) {
                            c.setAuthName(next.getTextContent().trim());
                        } else if (name.equals(prop = "authpassword")) {
                            c.setAuthPassword(next.getTextContent().trim());
                        } 
                    }
                }
                m_authcodes.put(c.getAuthCode(), c);
            }
        } catch (Exception e) {
            LOG.fatal("Configuration::loadXML Problem understanding document "+prop, e);
            System.exit(1);
        }
    }



}
