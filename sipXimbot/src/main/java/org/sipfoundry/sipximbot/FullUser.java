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
    
}
