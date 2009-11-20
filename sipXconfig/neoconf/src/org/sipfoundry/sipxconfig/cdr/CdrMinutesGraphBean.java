/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cdr;

import org.apache.commons.lang.StringUtils;

public class CdrMinutesGraphBean implements Comparable<CdrMinutesGraphBean> {
    private String m_extension;

    private Double m_minutes;

    public CdrMinutesGraphBean(String extension, Double minutes) {
        this.m_extension = StringUtils.defaultString(extension);
        this.m_minutes = minutes;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }

    public Double getMinutes() {
        return m_minutes;
    }

    public void setMinutes(Double minutes) {
        m_minutes = minutes;
    }

    public int compareTo(CdrMinutesGraphBean obj) {
        if (obj == null) {
            return -1;
        }
        return getMinutes().compareTo(obj.getMinutes());
    }
}
