/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone;


/**
 * Information to describe an external line.
 */
public class LineInfo {
    private String m_registrationServer;
    private String m_registrationServerPort;
    private String m_userId;
    private String m_password;
    private String m_huntgroup;
    private String m_voiceMail;
    private String m_displayName;

    /** optional - some phones like numeric extensions */
    private String m_extension;

    public String getDisplayName() {
        return m_displayName;
    }
    public void setDisplayName(String displayName) {
        m_displayName = displayName;
    }
    public String getPassword() {
        return m_password;
    }
    public void setPassword(String password) {
        m_password = password;
    }
    public String getRegistrationServer() {
        return m_registrationServer;
    }
    public void setRegistrationServer(String registrationServer) {
        m_registrationServer = registrationServer;
    }
    public String getUserId() {
        return m_userId;
    }
    public void setUserId(String userId) {
        m_userId = userId;
    }
    public String getHuntgroup() {
        return m_huntgroup;
    }
    public void setHuntgroup(String huntgroup) {
        m_huntgroup = huntgroup;
    }
    public String getVoiceMail() {
        return m_voiceMail;
    }
    public void setVoiceMail(String voiceMail) {
        m_voiceMail = voiceMail;
    }
    public String getRegistrationServerPort() {
        return m_registrationServerPort;
    }
    public void setRegistrationServerPort(String registrationServerPort) {
        m_registrationServerPort = registrationServerPort;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }
}
