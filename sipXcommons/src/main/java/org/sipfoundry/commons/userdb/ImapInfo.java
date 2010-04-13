/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.userdb;

import org.apache.commons.codec.binary.Base64;

public class ImapInfo {
    private String m_host;
    private String m_port;
    private boolean useTLS;
    private String m_account;
    private String m_password;
    private boolean m_synchronize;
    
    public String getHost() {
        return m_host;
    }
    
    public void setHost(String host) {
        m_host = host;
    }
    
    public String getPort() {
        return m_port;
    }
    
    public void setPort(String port) {
        m_port = port;
    }
    
    public boolean isUseTLS() {
        return useTLS;
    }
    
    public void setUseTLS(boolean useTLS) {
        this.useTLS = useTLS;
    }
    
    public String getAccount() {
        return m_account;
    }
    
    public void setAccount(String account) {
        m_account = account;
    }
    
    public String getPassword() {
        return m_password;
    }
    
    public String getDecodedPassword() {
        byte[] decoded = Base64.decodeBase64(m_password.getBytes());
        String decodedStr = new String(decoded);

        return decodedStr;
    }
    
    public void setPassword(String password) {
        m_password = password;
    }

    public boolean isSynchronize() {
        return m_synchronize;
    }

    public void setSynchronize(boolean synchronize) {
        m_synchronize = synchronize;
    }
}
