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

import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.firewall.EditableFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.ServerGroup;

public abstract class EditFirewallRules extends BaseComponent implements PageBeginRenderListener {

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:firewallManager")
    public abstract FirewallManager getFirewallManager();

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
        String key = "address." + getRule().getAddressType().getId();
        return TapestryUtils.getDefaultMessage(getMessages(), key, key);
    }

    public String getAddressDescription() {
        String key = format("address.%s.description", getRule().getAddressType().getId());
        return TapestryUtils.getDefaultMessage(getMessages(), key, "");
    }

    public void setServerGroupOption(SelectServerGroup.Option option) {
        option.setProperties(getRule());
    }

    @Override
    public void pageBeginRender(PageEvent arg0) {
        List<ServerGroup> serverGroups = getServerGroups();
        if (serverGroups == null) {
            serverGroups = getFirewallManager().getServerGroups();
            setServerGroups(serverGroups);
        }

        if (getRules() == null) {
            setRules(getFirewallManager().getEditableFirewallRules());
        }
    }

    public void save() {
        getFirewallManager().saveRules(getRules());
    }
}
