/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import java.io.Serializable;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.common.SipUri;

public class Button implements Serializable {
    private String m_label;
    private String m_number;
    private boolean m_blf;
    
    public Button() {        
    }

    public Button(String label, String number) {
        setNumber(number);
        setLabel(label);
    }

    /**
     * @return null if not set, may want to use number
     */
    public String getLabel() {
        return m_label;
    }

    public void setLabel(String label) {
        m_label = label;
    }

    public String getNumber() {
        return m_number;
    }

    public void setNumber(String number) {
        m_number = number;
    }
    
    public boolean isBlf() {
        return m_blf;
    }

    public void setBlf(boolean blf) {
        m_blf = blf;
    }

    public int hashCode() {
        return new HashCodeBuilder().append(m_label).append(m_number).toHashCode();
    }

    public boolean equals(Object obj) {
        if (!(obj instanceof Button)) {
            return false;
        }
        if (this == obj) {
            return true;
        }
        Button rhs = (Button) obj;
        return new EqualsBuilder().append(m_label, rhs.m_label).append(m_number, rhs.m_number)
                .isEquals();
    }

    /**
     * Creates a URI from button "number". If number already has a form of SIP URI it's value is
     * return. If not domain is appended to number to create a SIP URI.
     * 
     * @param domainName to be appended to URI id it's not a full URI
     */
    public String getUri(String domainName) {
        if (SipUri.matches(m_number)) {
            return SipUri.normalize(m_number);
        }
        return SipUri.format(m_number, domainName, false);
    }
}
