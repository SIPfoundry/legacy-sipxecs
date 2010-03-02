/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.admin.alarm;

import java.io.Serializable;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;

public class AlarmTrapReceiver implements Serializable {

    private String m_hostAddress;
    private String m_communityString;
    private int m_port = 162;

    public AlarmTrapReceiver() {
    }

    public String getHostAddress() {
        return m_hostAddress;
    }

    public void setHostAddress(String address) {
        m_hostAddress = address;
    }

    public String getCommunityString() {
        return m_communityString;
    }

    public void setCommunityString(String string) {
        m_communityString = string;
    }

    public Integer getPort() {
        return m_port;
    }

    public void setPort(Integer port) {
        m_port = port;
    }

    @Override
    public int hashCode() {
        return new HashCodeBuilder().append(m_hostAddress).append(m_communityString).append(m_port).toHashCode();
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof AlarmTrapReceiver)) {
            return false;
        }
        if (this == obj) {
            return true;
        }
        AlarmTrapReceiver rhs = (AlarmTrapReceiver) obj;
        return new EqualsBuilder().append(m_hostAddress, rhs.m_hostAddress).append(m_communityString,
                rhs.m_communityString).append(m_port, rhs.m_port).isEquals();
    }
}
