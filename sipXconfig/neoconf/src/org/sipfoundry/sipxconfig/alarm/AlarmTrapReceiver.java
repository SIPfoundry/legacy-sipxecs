/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.alarm;


import java.util.Collection;
import java.util.Collections;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.feature.Feature;

public class AlarmTrapReceiver extends BeanWithId implements DeployConfigOnEdit {

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

    public String toString() {
        StringBuilder sb = new StringBuilder();
        sb.append(getHostAddress());
        if (getPort() != null) {
            sb.append(':').append(getPort());
        }
        if (StringUtils.isNotBlank(getCommunityString())) {
            sb.append(' ').append(getCommunityString());
        }
        return sb.toString();
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
        return new EqualsBuilder().append(m_hostAddress, rhs.m_hostAddress)
                .append(m_communityString, rhs.m_communityString).append(m_port, rhs.m_port).isEquals();
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) Alarms.FEATURE);
    }
}
