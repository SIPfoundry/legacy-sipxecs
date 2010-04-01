/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipximbot;

import org.sipfoundry.commons.userdb.User;

public class FullUser extends User {

    private String  m_cellNumber;
    private String  m_homeNumber;
    private String  m_jid;
    private String  m_altjid;
    private String  m_confName;
    private String  m_confNum;
    private String  m_confPin;
    private boolean m_sendConfEntryIM;
    private boolean m_sendConfExitIM;
    private boolean m_sendVMEntryIM;
    private boolean m_sendVMExitIM;
    
    public String getCellNum() {
        return m_cellNumber;
    }
    
    public FullUser(User u) {
        this.setUserName(u.getUserName());
        this.setDisplayName(u.getDisplayName());
        this.setAliases(u.getAliases());
        this.setUri(u.getUri());
        this.setIdentity(u.getIdentity());
    }

    public void setCellNum(String cellNum) {
        m_cellNumber = cellNum;
    }
    
    public String getHomeNum() {
        return m_homeNumber;
    }

    public void setHomeNum(String homeNum) {
        m_homeNumber = homeNum;
    }
    
    public String getjid() {
        return m_jid;
    }

    public void setjid(String jid) {
        m_jid = jid;
    }
    
    public String getAltjid() {
        return m_altjid;
    }

    public void setAltjid(String jid) {
        m_altjid = jid;
    }
    
    public void setConfName(String confName) {
        m_confName = confName;
    }    
    
    public String getConfName() {
        return m_confName;
    }    
    
    public void setConfNum(String confNum) {
        m_confNum = confNum;
    }    
    
    public String getConfNum() {
        return m_confNum;
    }    
    
    public void setConfPin(String confPin) {
        m_confPin = confPin;
    }    
    
    public String getConfPin() {
        return m_confPin;
    }

    public void setConfEntryIM(String value) {
        m_sendConfEntryIM = value.equals("1") || value.equals("true");  
    } 
    
    public boolean getConfEntryIM() {
        return m_sendConfEntryIM;
    } 
    
    public void setConfExitIM(String value) {
        m_sendConfExitIM = value.equals("1") || value.equals("true");  
    }   
    
    public boolean getConfExitIM() {
        return m_sendConfExitIM;
    } 
    
    public void setVMEntryIM(String value) {
        m_sendVMEntryIM = value.equals("1") || value.equals("true");  
    }    
    
    public boolean getVMEntryIM() {
        return m_sendVMEntryIM;
    } 
    
    public void setVMExitIM(String value) {
        m_sendVMExitIM = value.equals("1") || value.equals("true");  
    }    
    
    public boolean getVMExitIM() {
        return m_sendVMExitIM;
    } 
    
}
