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

public class CdrMinutesGraphBean implements Comparable<CdrMinutesGraphBean> {
    private String m_extension;

    private Long m_minutes;

    public CdrMinutesGraphBean(String extension, Long minutes) {
        this.m_extension = extension;
        this.m_minutes = minutes;
    }

    public String getExtension() {
        return m_extension;
    }

    public void setExtension(String extension) {
        m_extension = extension;
    }

    public Long getMinutes() {
        return m_minutes;
    }

    public void setMinutes(Long minutes) {
        m_minutes = minutes;
    }

    public int compareTo(CdrMinutesGraphBean obj) {
        if (obj == null) {
            return -1;
        }
        return getMinutes().compareTo(obj.getMinutes());
    }
}
