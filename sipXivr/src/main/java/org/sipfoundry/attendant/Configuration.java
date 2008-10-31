/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.attendant;

import java.io.File;
import java.util.Vector;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.apache.log4j.Logger;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

/**
 * Holds the configuration data needed for the AutoAttendant.
 * 
 */
public class Configuration {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    public class AttendantConfig {
    	private String m_id ; // The ID of this attendant
        private String m_name; // The name of this attendant
        private String m_prompt; // The top level prompt
        private Vector<AttendantMenuItem> m_menuItems;
        private int m_initialTimeout; // initial digit timeout (mS)
        private int m_interDigitTimeout; // subsequent digit timeout (mS)
        private int m_extraDigitTimeout; // extra (wait for #) (mS)
        private int m_maximumDigits; // maximum extension length
        private int m_noInputCount; // give up after this many timeouts
        private int m_invalidResponseCount; // give up after this many bad entries
        private boolean m_transferOnFailures; // What to do on failure
        private String m_transferPrompt; // What to say
        private String m_transferUrl; // Where to go

        public AttendantConfig() {
            // Global defaults if otherwise not specified
            m_initialTimeout = 7000;
            m_interDigitTimeout = 3000;
            m_extraDigitTimeout = 3000;
            m_maximumDigits = 10;
            m_noInputCount = 2;
            m_invalidResponseCount = 2;
        }
        
        public String getId() {
        	return m_id;
        }
        
        public String getName() {
            return m_name;
        }

        public String getPrompt() {
            return m_prompt;
        }

        public Vector<AttendantMenuItem> getMenuItems() {
            return m_menuItems;
        }

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
    }

    private static Configuration s_current;
    private static File s_configFile;
    private static long s_lastModified;

    private Vector<AttendantConfig> m_attendants;
    private Vector<Schedule> m_schedules;
    
    public Schedule getSchedule(String id) {
        if (m_schedules == null || id == null) {
            return null;
        }

        for (Schedule s : m_schedules) {
            if (s.getId().contentEquals(id)) {
                return s;
            }
        }
        return null;
    }

    /**
     * Private constructor for updatable singleton
     */
    private Configuration() {
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

    public AttendantConfig getAttendant(String id) {
        if (m_attendants == null || id == null) {
            return null;
        }

        for (AttendantConfig config : m_attendants) {
            if (config.m_id.contentEquals(id)) {
                return config;
            }
        }
        return null;
    }

    /**
     * Load the autoattendants.xml file
     */
    void loadXML() {
    	LOG.info("Loading autoattendants.xml configuration");
        String path = System.getProperty("conf.dir");
        if (path == null) {
            LOG.fatal("Cannot get System Property conf.dir!  Check jvm argument -Dconf.dir=") ;
            System.exit(1);
        }
        
        Document autoAttendantsDoc = null;
        
        try {
            s_configFile = new File(path + "/autoattendants.xml");
            s_lastModified = s_configFile.lastModified();

            DocumentBuilder builder = DocumentBuilderFactory.newInstance().newDocumentBuilder();
            autoAttendantsDoc = builder.parse(s_configFile);
            
        } catch (Throwable t) {
            LOG.fatal("Something went wrong loading the autoattendants.xml file.", t);
            System.exit(1);
        }

        String prop = null;
        try {
            prop = "unknown";
            m_attendants = new Vector<AttendantConfig>();

            // Walk autoattendant elements, building up m_attendants as we go
            NodeList autoattendants = autoAttendantsDoc.getElementsByTagName(prop = "autoattendant");
            for (int aaNum = 0; aaNum < autoattendants.getLength(); aaNum++) {
                Node attendantNode = autoattendants.item(aaNum);
                AttendantConfig c = new AttendantConfig() ;
                String id = attendantNode.getAttributes().getNamedItem("id").getNodeValue();
                c.m_id = id;
                for(Node next = attendantNode.getFirstChild(); next != null; next = next.getNextSibling()) {
                    if (next.getNodeType() == Node.ELEMENT_NODE) {
                        String name = next.getNodeName();
                        if (name.equals(prop = "name")) {
                            c.m_name = next.getTextContent().trim();
                        } else if (name.equals(prop = "prompt")) {
                            c.m_prompt = next.getTextContent().trim();
                        } else if (name.equals(prop = "menuItems")) {
                            parseMenuItems(next, c) ;
                        } else if (name.equals(prop = "dtmf")) {
                            parseDtmf(next, c) ;
                        }  else if (name.equals(prop = "invalidResponse")) {
                            parseInvalidResponse(next, c) ;
                        }
                    }
                }
                m_attendants.add(c);
            }

            // Walk schedule elements 
            m_schedules = new Vector<Schedule>();
            NodeList schedules = autoAttendantsDoc.getElementsByTagName(prop = "schedule");
            for (int schedNum = 0; schedNum < schedules.getLength(); schedNum++) {
            	Node scheduleNode = schedules.item(schedNum);
            	Schedule s = new Schedule();
            	s.loadSchedule(scheduleNode) ;
            	m_schedules.add(s) ;
            }

        } catch (Exception e) {
            LOG.fatal("Problem understanding document "+prop, e);
            System.exit(1);
        }
    }

    void parseMenuItems(Node menuItemsNode, AttendantConfig c) {
        c.m_menuItems = new Vector<AttendantMenuItem>();
        for(Node next = menuItemsNode.getFirstChild(); next != null; next = next.getNextSibling()) {
            if (next.getNodeType() == Node.ELEMENT_NODE) {
                if (next.getNodeName().equals("menuItem")) {
                    parseMenuItem(next, c);
                }
            }
        }
    }
    
    void parseInvalidResponse(Node menuItemNode, AttendantConfig c) {
        for(Node next = menuItemNode.getFirstChild(); next != null; next = next.getNextSibling()) {
            if (next.getNodeType() == Node.ELEMENT_NODE) {
                String name = next.getNodeName();
                if (name.equals("noInputCount")) {
                    c.m_noInputCount = Integer.parseInt(next.getTextContent().trim());
                } else if (name.equals("invalidResponseCount")) {
                    c.m_invalidResponseCount = Integer.parseInt(next.getTextContent().trim());
                } else if (name.equals("transferOnFailures")) {
                    c.m_transferOnFailures = Boolean.parseBoolean(next.getTextContent().trim());
                } else if (name.equals("transferUrl")) {
                    c.m_transferUrl =next.getTextContent().trim();
                } else if (name.equals("transferPrompt")) {
                    c.m_transferPrompt = next.getTextContent().trim();
                }
            }
        }
    }

    void parseDtmf(Node dtmfNode, AttendantConfig c) {
        for(Node next = dtmfNode.getFirstChild(); next != null; next = next.getNextSibling()) {
            if (next.getNodeType() == Node.ELEMENT_NODE) {
                String name = next.getNodeName();
                if (name.equals("interDigitTimeout")) {
                    c.m_interDigitTimeout = Integer.parseInt(next.getTextContent().trim());
                    if (c.m_interDigitTimeout < 100) {
                        LOG.warn("interDigitTimeout is too short.  Assuming x1000");
                        c.m_interDigitTimeout *= 1000; // Convert from seconds to mS
                    }
                } else if (name.equals("maximumDigits")) {
                    c.m_maximumDigits = Integer.parseInt(next.getTextContent().trim());
                } else if (name.equals("extraDigitTimeout")) {
                    c.m_extraDigitTimeout = Integer.parseInt(next.getTextContent().trim());
                    if (c.m_extraDigitTimeout < 100) {
                        LOG.warn("extraDigitTimeout is too short.  Assuming x1000");
                        c.m_extraDigitTimeout *= 1000; // Convert from seconds to mS
                    }
                } else if (name.equals("initialTimeout")) {
                    c.m_initialTimeout = Integer.parseInt(next.getTextContent().trim());
                    if (c.m_initialTimeout < 100) {
                        LOG.warn("initialTimeout is too short.  Assuming x1000");
                        c.m_initialTimeout *= 1000; // Convert from seconds to mS
                    }
                }
            }
        }
    }

    void parseMenuItem(Node menuItemNode, AttendantConfig c) {
        String dialPad = "";
        String action = "";
        String extension = "";
        String parameter = "" ;
        
        for(Node next = menuItemNode.getFirstChild(); next != null; next = next.getNextSibling()) {
            if (next.getNodeType() == Node.ELEMENT_NODE) {
                String name = next.getNodeName();
                if (name.equals("dialPad")) {
                    dialPad = next.getTextContent().trim();
                } else if (name.equals("action")) {
                    action = next.getTextContent().trim();
                } else if (name.equals("extension")) {
                    extension = next.getTextContent().trim();
                } else if (name.equals("parameter")) {
                    parameter = next.getTextContent().trim();
                }
            }
        }
        c.m_menuItems.add(new AttendantMenuItem(dialPad, Actions.valueOf(action), parameter, extension));
    }
    
    void internal() {
        m_attendants = new Vector<AttendantConfig>();
        m_schedules = new Vector<Schedule>();
        AttendantConfig c = new AttendantConfig();
        c.m_name = "operator";
        c.m_menuItems = new Vector<AttendantMenuItem>();
        c.m_menuItems.add(new AttendantMenuItem("#", Actions.voicemail_access, "", ""));
        c.m_menuItems.add(new AttendantMenuItem("*", Actions.repeat_prompt, "", ""));
        c.m_menuItems.add(new AttendantMenuItem("0", Actions.operator, "", ""));
        c.m_menuItems.add(new AttendantMenuItem("1", Actions.transfer_out, "",
                "sip:8200@cdhcp151.pingtel.com"));
        c.m_menuItems.add(new AttendantMenuItem("2", Actions.voicemail_deposit, "", "200"));
        c.m_menuItems.add(new AttendantMenuItem("3", Actions.disconnect, "", ""));
        c.m_menuItems.add(new AttendantMenuItem("4", Actions.transfer_to_another_aa_menu,
                "vacation", ""));
        c.m_menuItems.add(new AttendantMenuItem("9", Actions.dial_by_name, "", ""));
        c.m_initialTimeout = 7000;
        c.m_interDigitTimeout = 3000;
        c.m_extraDigitTimeout = 3000;
        c.m_maximumDigits = 10;
        c.m_noInputCount = 2;
        c.m_invalidResponseCount = 2;
        c.m_transferOnFailures = false;
        c.m_transferPrompt = null;
        c.m_transferUrl = null;
        m_attendants.add(c);

        c = new AttendantConfig();
        c.m_name = "afterhour";
        c.m_prompt = "/usr/local/ecs/main/var/sipxdata/mediaserver/data/prompts/afterhours.wav";
        c.m_menuItems = new Vector<AttendantMenuItem>();
        c.m_menuItems.add(new AttendantMenuItem("#", Actions.voicemail_access, "", ""));
        c.m_menuItems.add(new AttendantMenuItem("*", Actions.repeat_prompt, "", ""));
        c.m_menuItems.add(new AttendantMenuItem("9", Actions.dial_by_name, "", ""));
        c.m_initialTimeout = 7000;
        c.m_interDigitTimeout = 3000;
        c.m_extraDigitTimeout = 3000;
        c.m_maximumDigits = 10;
        c.m_noInputCount = 2;
        c.m_invalidResponseCount = 2;
        c.m_transferOnFailures = false;
        c.m_transferPrompt = null;
        c.m_transferUrl = null;
        m_attendants.add(c);

        c = new AttendantConfig();
        c.m_name = "vacation";
        c.m_prompt = "/usr/local/ecs/main/var/sipxdata/mediaserver/data/prompts/afterhours.wav";
        c.m_menuItems = new Vector<AttendantMenuItem>();
        c.m_menuItems.add(new AttendantMenuItem("#", Actions.voicemail_access, "", ""));
        c.m_menuItems.add(new AttendantMenuItem("*", Actions.repeat_prompt, "", ""));
        c.m_menuItems.add(new AttendantMenuItem("9", Actions.dial_by_name, "", ""));
        c.m_initialTimeout = 7000;
        c.m_interDigitTimeout = 3000;
        c.m_extraDigitTimeout = 3000;
        c.m_maximumDigits = 10;
        c.m_noInputCount = 2;
        c.m_invalidResponseCount = 2;
        c.m_transferOnFailures = false;
        c.m_transferPrompt = null;
        c.m_transferUrl = null;
        m_attendants.add(c);

    }

}
