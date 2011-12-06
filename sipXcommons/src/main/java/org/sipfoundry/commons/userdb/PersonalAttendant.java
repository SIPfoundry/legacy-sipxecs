/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.commons.userdb;

import java.util.Map;

import org.apache.log4j.Logger;

/**
 * Holds the configuration data and performs the parts needed for 
 * Personal AutoAttendant part of voicemail
 * 
 */

public class PersonalAttendant {
    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    private String m_language; // Language to use for callers to this mailbox
    private String m_operator; // URL to dial when caller presses "0"
    private Map<String, String> m_menu; // Key is digit pressed, value is URL to dial
    private String m_validDigits;

    public PersonalAttendant(String language, String operator, Map<String, String> menu, String validDigits){
        m_language = language;
        m_operator = operator;
        m_menu = menu;
        m_validDigits = validDigits;
    }

    public String getLanguage() {
        return m_language;
    }
    
    public String getOperator() {
        return m_operator;
    }
    
    public String getMenuValue(String key) {
        return m_menu.get(key);
    }
    
    public String getValidDigits() {
        return m_validDigits;
    }

}
