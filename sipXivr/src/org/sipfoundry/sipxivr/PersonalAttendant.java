/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxivr;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.HashMap;
import java.util.Properties;

import org.apache.log4j.Logger;

/**
 * Holds the configuration data and performs the parts needed for 
 * Personal AutoAttendant part of voicemail
 * 
 */

public class PersonalAttendant {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    private Mailbox m_mailbox;
    
    private String m_language; // Language to use for callers to this mailbox
    private String m_operator; // URL to dial when caller presses "0"
    private HashMap<String, String> m_menu; // Key is digit pressed, value is URL to dial
    private StringBuilder m_validDigits;

    public PersonalAttendant(Mailbox mailbox){
        m_mailbox = mailbox ;
        m_menu = new HashMap<String, String>();
        m_validDigits = new StringBuilder(10);
        loadProperties();
    }
    
    void loadProperties() {
        String path = m_mailbox.getUserDirectory();
        if (path == null) {
            LOG.warn("PersonalAttendant::loadProperties User directory for mailbox is null");
            return ;
        }
        String name = path + "/PersonalAttendant.properties";
        FileInputStream inStream;
        Properties props = null;
        try {
            File propertiesFile = new File(name);
            inStream = new FileInputStream(propertiesFile);
            props = new Properties();
            props.load(inStream);
            inStream.close();
        } catch (FileNotFoundException e) {
            LOG.warn("PersonalAttendant::loadProperties File not found: "+name);
            return ;
        } catch (IOException e) {
            LOG.error("PersonalAttendant::loadProperties File cannot be read: "+name, e);
            throw new RuntimeException(e);
        }

        String prop = null;
        try {
            m_language = props.getProperty(prop="pa.language");
            if (m_language != null && m_language.length() == 0) {
                m_language = null;
            }
            m_operator = props.getProperty(prop="pa.operator");
            if (m_operator != null && m_operator.length() == 0) {
                m_operator = null;
            }
            for(int i=1; i<10; i++) {
                prop = String.format("pa.menu.%d.key", i);
                String key = props.getProperty(prop);
                if (key == null) {
                    break ;
                }
                prop = String.format("pa.menu.%d.uri", i);
                String value = props.getProperty(prop);
                if (value == null) {
                    break ;
                }
                m_menu.put(key, value);
                m_validDigits.append(key);
            }
        } catch (Exception e) {
            LOG.error("PersonalAttendant::loadProperties problem understanding property: "+prop);
            throw new RuntimeException(e);
        }
    }

    public String getLanguage() {
        return m_language;
    }
    
    public String getOperator() {
        return m_operator;
    }
    
    public void setOperator(String operator) {
        
        m_operator = operator;
    }
    
    public String getMenuValue(String key) {
        return m_menu.get(key);
    }
    
    public String getValidDigits() {
        return m_validDigits.toString();
    }
}
