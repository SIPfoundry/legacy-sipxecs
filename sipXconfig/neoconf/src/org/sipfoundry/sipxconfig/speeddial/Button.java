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

    @Override
    public int hashCode() {
        return new HashCodeBuilder().append(m_label).append(m_number).toHashCode();
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof Button)) {
            return false;
        }
        if (this == obj) {
            return true;
        }
        Button rhs = (Button) obj;
        return new EqualsBuilder().append(m_label, rhs.m_label).append(m_number, rhs.m_number).isEquals();
    }
}
