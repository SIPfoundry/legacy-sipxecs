/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.Serializable;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.common.PrimaryKeySource;

public class CertificateDecorator implements Comparable, PrimaryKeySource, Serializable {
    private String m_fileName;

    public int compareTo(Object certificate) {
        return m_fileName.compareTo(((CertificateDecorator) certificate).getFileName());
    }

    @Override
    public int hashCode() {
        return new HashCodeBuilder().append(m_fileName).toHashCode();
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof CertificateDecorator)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        CertificateDecorator cert = (CertificateDecorator) other;
        return new EqualsBuilder().append(m_fileName,
                cert.m_fileName).isEquals();
    }

    @Override
    public String toString() {
        return "Certificate: " + m_fileName;
    }

    public Object getPrimaryKey() {
        return this;
    }

    public String getFileName() {
        return m_fileName;
    }

    public void setFileName(String fileName) {
        m_fileName = fileName;
    }

}
