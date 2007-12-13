/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.common;

public class SpecialUser extends  BeanWithId {
    
    public enum SpecialUserType {
        PARK_SERVER("~~id~park"), MEDIA_SERVER("~~id~media");
        
        private String m_credential;
        
        private SpecialUserType(String credential) {
            m_credential = credential;
        }
        
        public String getCredential() {
            return m_credential;
        }
    }

    private SpecialUserType m_type;
    private String m_sipPassword;
    
    public void setType(String type) {
        m_type = SpecialUserType.valueOf(type);
    }
    
    public void setType(SpecialUserType type) {
        m_type = type;
    }
    
    public String getType() {
        return m_type.toString();
    }
    
    public String getCredential() {
        return m_type.getCredential();
    }

    public void setSipPassword(String sipPassword) {
        m_sipPassword = sipPassword;
    }
    
    public String getSipPassword() {
        return m_sipPassword;
    }
}
