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
package org.sipfoundry.sipxconfig.site.firewall;

import java.util.List;

import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.firewall.EditableFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallRule;
import org.sipfoundry.sipxconfig.firewall.ServerGroup;

public class SelectServerGroup implements IPropertySelectionModel {
    private List<ServerGroup> m_serverGroups;

    public static class Option {
        private FirewallRule.SystemId m_systemId;
        private ServerGroup m_serverGroup;

        public Option(EditableFirewallRule rule) {
            m_serverGroup = rule.getServerGroup();
            if (m_serverGroup == null) {
                m_systemId = rule.getSystemId();
            }
        }

        public Option(ServerGroup serverGroup) {
            m_serverGroup = serverGroup;
        }

        public Option(FirewallRule.SystemId systemId) {
            m_systemId = systemId;
        }

        public void setProperties(EditableFirewallRule rule) {
            if (m_serverGroup != null) {
                rule.setServerGroup(m_serverGroup);
            } else {
                rule.setSystemId(m_systemId);
            }
        }

        @Override
        public boolean equals(Object obj) {
            if (obj == null || !(obj instanceof Option)) {
                return false;
            }

            Option rhs = (Option) obj;
            if (m_serverGroup == null || rhs.m_serverGroup == null) {
                if (this.m_serverGroup != rhs.m_serverGroup) {
                    return false;
                }
            } else {
                if (!m_serverGroup.equals(rhs.m_serverGroup)) {
                    return false;
                }
            }

            return (m_systemId == rhs.m_systemId);
        }

        @Override
        public int hashCode() {
            return super.hashCode();
        }

        public FirewallRule.SystemId getSystemId() {
            return m_systemId;
        }

        public ServerGroup getServerGroup() {
            return m_serverGroup;
        }
    }

    public SelectServerGroup(List<ServerGroup> serverGroups) {
        m_serverGroups = serverGroups;
    }

    @Override
    public int getOptionCount() {
        return 2 + m_serverGroups.size();
    }

    @Override
    public Object getOption(int index) {
        switch (index) {
        case 0:
            return new Option(FirewallRule.SystemId.CLUSTER);
        case 1:
            return new Option(FirewallRule.SystemId.PUBLIC);
        default:
            return new Option(m_serverGroups.get(index - 2));
        }
    }

    @Override
    public String getLabel(int index) {
        switch (index) {
        case 0:
            return FirewallRule.SystemId.CLUSTER.toString();
        case 1:
            return FirewallRule.SystemId.PUBLIC.toString();
        default:
            return m_serverGroups.get(index - 2).getName();
        }
    }

    @Override
    public String getValue(int index) {
        switch (index) {
        case 0:
            return FirewallRule.SystemId.CLUSTER.toString();
        case 1:
            return FirewallRule.SystemId.PUBLIC.toString();
        default:
            return m_serverGroups.get(index - 2).getId().toString();
        }
    }

    @Override
    public boolean isDisabled(int index) {
        return false;
    }

    @Override
    public Object translateValue(String value) {
        try {
            return new Option(FirewallRule.SystemId.valueOf(value));
        } catch (Exception e) {
            Integer id = Integer.parseInt(value);
            for (ServerGroup group : m_serverGroups) {
                if (group.getId().equals(id)) {
                    return new Option(group);
                }
            }
        }
        throw new UserException("Invalid value " + value);
    }
}
