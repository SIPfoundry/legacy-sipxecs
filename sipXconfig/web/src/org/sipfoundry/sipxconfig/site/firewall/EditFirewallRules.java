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

import static java.lang.String.format;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Map;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.components.IPrimaryKeyConverter;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.firewall.EditableFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.ServerGroup;

public abstract class EditFirewallRules extends BaseComponent implements PageBeginRenderListener {

    private static final String ADDRESS = "address.";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:firewallManager")
    public abstract FirewallManager getFirewallManager();

    @InjectObject("spring:addressManager")
    public abstract AddressManager getAddressManager();

    public abstract EditableFirewallRule getRule();

    public abstract List<ServerGroup> getServerGroups();

    public abstract void setServerGroups(List<ServerGroup> groups);

    public SelectServerGroup getSelectServerGroup() {
        return new SelectServerGroup(getServerGroups());
    }

    public abstract List<EditableFirewallRule> getRules();

    public abstract void setRules(List<EditableFirewallRule> rules);

    public SelectServerGroup.Option getServerGroupOption() {
        return new SelectServerGroup.Option(getRule());
    }

    public String getAddressLabel() {
        AddressType type = getRule().getAddressType();
        String key = ADDRESS + type.getId();
        return TapestryUtils.getDefaultMessage(getMessages(), key, key);
    }

    public String getPortDetails() {
        AddressType type = getRule().getAddressType();
        List<Address> addresses = getAddressManager().getAddresses(type);
        String disabled = getMessages().getMessage("disabled");
        if (addresses.size() == 0) {
            return disabled;
        }
        List<String> details = new ArrayList<String>();
        for (Address address : addresses) {
            StringBuilder detail = new StringBuilder();
            if (address.getCanonicalPort() == 0) {
                detail.append(disabled);
            } else {
                if (address.getAddress() != null) {
                    detail.append(address.getAddress());
                    detail.append(" : ");
                }
                detail.append(address.getCanonicalPort());
                if (address.getEndPort() != 0) {
                    detail.append(":");
                    detail.append(address.getEndPort());
                }
            }
            details.add(detail.toString());
        }
        return StringUtils.join(details, "<br/>");
    }

    public String getAddressDescription() {
        String key = format("address.%s.description", getRule().getAddressType().getId());
        return TapestryUtils.getDefaultMessage(getMessages(), key, "");
    }

    public void setServerGroupOption(SelectServerGroup.Option option) {
        option.setProperties(getRule());
    }

    public IPrimaryKeyConverter getRuleConverter() {
        final Map<String, EditableFirewallRule> ruleMap = FirewallManager.Util.defaultsByAddressTypeId(getRules());
        return new IPrimaryKeyConverter() {
            public Object getPrimaryKey(Object value) {
                return ((EditableFirewallRule) value).getAddressType().getId();
            }
            public Object getValue(Object primaryKey) {
                return ruleMap.get(primaryKey);
            }
        };
    }

    @Override
    public void pageBeginRender(PageEvent arg0) {
        List<ServerGroup> serverGroups = getServerGroups();
        if (serverGroups == null) {
            serverGroups = getFirewallManager().getServerGroups();
            setServerGroups(serverGroups);
        }

        if (getRules() == null) {
            List<EditableFirewallRule> rules = getFirewallManager().getEditableFirewallRules();
            Collections.sort(rules, new Comparator<EditableFirewallRule>() {
                public int compare(EditableFirewallRule o1, EditableFirewallRule o2) {
                    return (getMessages().getMessage(ADDRESS + o1.getAddressType().getId())).compareTo(getMessages()
                            .getMessage(ADDRESS + o2.getAddressType().getId()));
                }
            });
            setRules(rules);
        }
    }

    public void save() {
        getFirewallManager().saveRules(getRules());
    }
}
