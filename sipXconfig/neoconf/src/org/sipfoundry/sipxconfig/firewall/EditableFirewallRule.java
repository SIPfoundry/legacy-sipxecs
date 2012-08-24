/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
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

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.feature.Feature;

public class EditableFirewallRule extends BeanWithId implements DeployConfigOnEdit, FirewallRule {
    private DefaultFirewallRule m_delegate;
    private ServerGroup m_serverGroup;
    private Boolean m_priority;
    private SystemId m_systemId;

    public EditableFirewallRule(DefaultFirewallRule delegate) {
        m_delegate = delegate;
    }

    public boolean isChangedFromDefault() {
        if (m_serverGroup == null) {
            if (getSystemId() == m_delegate.getSystemId()) {
                if (isPriority() == m_delegate.isPriority()) {
                    return false;
                }
            }
        }
        return true;
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) FirewallManager.FEATURE);
    }

    public ServerGroup getServerGroup() {
        return m_serverGroup;
    }

    public void setServerGroup(ServerGroup serverGroup) {
        m_serverGroup = serverGroup;
        m_systemId = null;
    }

    @Override
    public boolean isPriority() {
        return m_priority == null ? m_delegate.isPriority() : m_priority;
    }

    public void setPriority(boolean priority) {
        m_priority = priority;
    }

    @Override
    public AddressType getAddressType() {
        return m_delegate.getAddressType();
    }

    @Override
    public SystemId getSystemId() {
        return m_systemId == null ? m_delegate.getSystemId() : m_systemId;
    }

    public void setSystemId(SystemId systemId) {
        m_systemId = systemId;
        m_serverGroup = null;
    }
}
