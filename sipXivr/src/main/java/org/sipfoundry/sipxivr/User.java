/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import java.util.Vector;
import org.apache.commons.codec.digest.DigestUtils;
import org.apache.log4j.Logger;

public class User {
    private String m_identity;
    private String m_userName;
    private String m_displayName;
    private String m_uri;
    private String m_pintoken;
    private boolean m_inDirectory;
    private Vector<String> m_dialPatterns;
    private Vector<String> m_aliases;

    public String getIdentity() {
        return m_identity;
    }

    public void setIdentity(String identity) {
        m_identity = identity;
    }

    public String getUserName() {
        return m_userName;
    }

    public void setUserName(String userName) {
        m_userName = userName;
    }

    public String getDisplayName() {
        return m_displayName;
    }

    public void setDisplayName(String displayName) {
        m_displayName = displayName;
    }

    public String getUri() {
        return m_uri;
    }

    public void setUri(String uri) {
        m_uri = uri;
    }

    public String getPintoken() {
        return m_pintoken;
    }

    public void setPintoken(String pintoken) {
        m_pintoken = pintoken;
    }

    public boolean isInDirectory() {
        return m_inDirectory;
    }

    public void setInDirectory(boolean inDirectory) {
        m_inDirectory = inDirectory;
    }

    public Vector<String> getDialPatterns() {
        return m_dialPatterns;
    }

    public void setDialPatterns(Vector<String> dialPatterns) {
        this.m_dialPatterns = dialPatterns;
    }
    
    public Vector<String> getAliases() {
    	return m_aliases;
    }
    
    public void setAliases(Vector<String> aliases) {
    	this.m_aliases = aliases;
    }

    public boolean isPinCorrect(String pin, String realm) {
        // pintoken is MD5 Hash of:
        //   userName:realm:pin
        String token = m_userName+":"+realm+":"+pin;
        String hash = DigestUtils.md5Hex(token);
        return hash.equals(m_pintoken);
    }
}
