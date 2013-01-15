/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.firewall;

import java.io.Serializable;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedList;
import java.util.List;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.feature.Feature;

public class CallRateRule extends BeanWithId implements DeployConfigOnEdit, Serializable {
    private String m_name;
    private String m_description;
    private String m_startIp;
    private String m_endIp;
    private int m_position;
    private List<CallRateLimit> m_limits = new LinkedList<CallRateLimit>();

    public void setStartIp(String ip) {
        m_startIp = ip;
    }

    public void setEndIp(String ip) {
        m_endIp = ip;
    }

    public void setName(String name) {
        m_name = name;
    }

    public void setPosition(int position) {
        m_position = position;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public void setCallRateLimits(List<CallRateLimit> limits) {
        m_limits = limits;
    }

    public String getStartIp() {
        return m_startIp;
    }

    public String getEndIp() {
        return m_endIp;
    }

    public String getName() {
        return m_name;
    }

    public int getPosition() {
        return m_position;
    }

    public String getDescription() {
        return m_description;
    }

    public List<CallRateLimit> getCallRateLimits() {
        return m_limits;
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) FirewallManager.FEATURE);
    }

    public String getAppliesTo() {
        StringBuilder sb = new StringBuilder();
        sb.append(m_startIp);
        if (m_endIp != null) {
            sb.append(" - ");
            sb.append(m_endIp);
        }
        return sb.toString();
    }

}
