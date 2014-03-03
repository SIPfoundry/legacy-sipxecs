/*
 *
 *
 * Copyright (C) 2008-2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
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
    private String m_specialId; // The ID of the special attendant (if any)

    private File s_configFile;
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

    public AttendantConfig getAttendant(String id) {
        if (m_attendants == null || id == null) {
            return null;
        }

        for (AttendantConfig config : m_attendants) {
            if (config.getId().contentEquals(id)) {
                return config;
            }
        }
        return null;
    }

    /**
     * Load new Configuration object if the underlying properties files have changed since the
     * last time.
     * 
     * @return
     */
    public void update() {
        if (s_configFile.lastModified() != s_lastModified) {
            loadXML();
        }
    }

    /**
     * Returns the id of the Special Autoattendant, or null if there is none specified.
     * @return
     */
    public String getSpecialAttendantId() {
        return m_specialId;
    }
    
    /**
     * Load the autoattendants.xml file
     */
    void loadXML() {
    	LOG.info("Configuration::loadXML Loading autoattendants.xml configuration");
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
            LOG.fatal("Configuration::loadXML Something went wrong loading the autoattendants.xml file.", t);
            return;
        }

        String prop = null;
        try {
            prop = "unknown";
            m_specialId = null;
            m_attendants = new Vector<AttendantConfig>();

            // Walk autoattendant elements, building up m_attendants as we go
            NodeList autoattendants = autoAttendantsDoc.getElementsByTagName(prop = "autoattendant");
            for (int aaNum = 0; aaNum < autoattendants.getLength(); aaNum++) {
                Node attendantNode = autoattendants.item(aaNum);
                AttendantConfig c = new AttendantConfig() ;
                String id = attendantNode.getAttributes().getNamedItem("id").getNodeValue();
                Node specialNode = attendantNode.getAttributes().getNamedItem("special");
                if (specialNode != null && Boolean.parseBoolean(specialNode.getNodeValue())) {
                    m_specialId = id;
                }
                c.setId(id);
                for(Node next = attendantNode.getFirstChild(); next != null; next = next.getNextSibling()) {
                    if (next.getNodeType() == Node.ELEMENT_NODE) {
                        String name = next.getNodeName();
                        if (name.equals(prop = "name")) {
                            c.setName(next.getTextContent().trim());
                        } else if (name.equals(prop = "prompt")) {
                            c.setPrompt(next.getTextContent().trim());
                        } else if (name.equals(prop = "menuItems")) {
                            parseMenuItems(next, c) ;
                        } else if (name.equals(prop = "dtmf")) {
                            parseDtmf(next, c) ;
                        }  else if (name.equals(prop = "invalidResponse")) {
                            parseInvalidResponse(next, c) ;
                        } else if (name.equals(prop = "onTransfer")) {
                            parseOnTransfer(next, c) ;
                        } else if (name.equals(prop = "lang")) {
                            c.setLang(next.getTextContent().trim());
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
            LOG.fatal("Configuration::loadXML Problem understanding document "+prop, e);
            System.exit(1);
        }
    }

    void parseMenuItems(Node menuItemsNode, AttendantConfig c) {
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
                    c.setNoInputCount(Integer.parseInt(next.getTextContent().trim()));
                } else if (name.equals("invalidResponseCount")) {
                    c.setInvalidResponseCount(Integer.parseInt(next.getTextContent().trim()));
                } else if (name.equals("transferOnFailures")) {
                    c.setTransferOnFailures(Boolean.parseBoolean(next.getTextContent().trim()));
                } else if (name.equals("transferUrl")) {
                    c.setTransferUrl(next.getTextContent().trim());
                } else if (name.equals("transferPrompt")) {
                    c.setTransferPrompt(next.getTextContent().trim());
                }
            }
        }
    }

    void parseDtmf(Node dtmfNode, AttendantConfig c) {
        for(Node next = dtmfNode.getFirstChild(); next != null; next = next.getNextSibling()) {
            if (next.getNodeType() == Node.ELEMENT_NODE) {
                String name = next.getNodeName();
                if (name.equals("interDigitTimeout")) {
                    int value = Integer.parseInt(next.getTextContent().trim());
                    if (value < 100) {
                        LOG.warn("Configuration::parseDtmf interDigitTimeout is too short.  Assuming x1000");
                        value *= 1000; // Convert from seconds to mS
                    }
                    c.setInterDigitTimeout(value);
                } else if (name.equals("maximumDigits")) {
                    c.setMaximumDigits(Integer.parseInt(next.getTextContent().trim()));
                } else if (name.equals("extraDigitTimeout")) {
                    int value = Integer.parseInt(next.getTextContent().trim());
                    if (value < 100) {
                        LOG.warn("Configuration::parseDtmf extraDigitTimeout is too short.  Assuming x1000");
                        value *= 1000; // Convert from seconds to mS
                    }
                    c.setExtraDigitTimeout(value);
                } else if (name.equals("initialTimeout")) {
                    int value = Integer.parseInt(next.getTextContent().trim());
                    if (value < 100) {
                        LOG.warn("Configuration::parseDtmf initialTimeout is too short.  Assuming x1000");
                        value *= 1000; // Convert from seconds to mS
                    }
                    c.setInitialTimeout(value);
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
        c.addMenuItem(new AttendantMenuItem(dialPad, Actions.valueOf(action), parameter, extension));
    }

    void parseOnTransfer(Node menuItemNode, AttendantConfig c) {
        for (Node next = menuItemNode.getFirstChild(); next != null; next = next.getNextSibling()) {
            if (next.getNodeType() == Node.ELEMENT_NODE) {
                String name = next.getNodeName();
                if (name.equals("play-prompt")) {
                    c.setPlayPrompt(Boolean.parseBoolean(next.getTextContent().trim()));
                }
            }
        }
    }
}
