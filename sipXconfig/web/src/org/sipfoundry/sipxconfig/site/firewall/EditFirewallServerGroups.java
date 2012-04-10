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

import java.io.Serializable;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.callback.PageCallback;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallRule;
import org.sipfoundry.sipxconfig.firewall.ServerGroup;

public abstract class EditFirewallServerGroups extends BaseComponent implements PageBeginRenderListener {

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract List<ServerGroup> getServerGroups();

    public abstract void setServerGroups(List<ServerGroup> groups);

    public abstract ServerGroup getCurrentServerGroup();

    @InjectPage(value = EditFirewallServerGroup.PAGE)
    public abstract EditFirewallServerGroup getEditServerGroupPage();

    @InjectObject("spring:firewallManager")
    public abstract FirewallManager getFirewallManager();

    public List<FirewallRule.SystemId> getSystemGroups() {
        return Arrays.asList(FirewallRule.SystemId.values());
    }

    public abstract FirewallRule.SystemId getCurrentSystemGroup();

    public abstract SelectMap getSelections();

    public abstract void setSelections(SelectMap selections);

    public void delete() {
        Collection<Serializable> allSelected = getSelections().getAllSelected();
        for (Serializable id : allSelected) {
            ServerGroup group = getFirewallManager().getServerGroup((Integer) id);
            getFirewallManager().deleteServerGroup(group);
        }
        setServerGroups(getFirewallManager().getServerGroups());
    }

    public IPage editGroup(Integer groupId) {
        return gotoEditGroupPage(groupId);
    }

    public IPage addGroup() {
        return gotoEditGroupPage(null);
    }

    IPage gotoEditGroupPage(Integer groupId) {
        EditFirewallServerGroup page = getEditServerGroupPage();
        page.setServerGroupId(groupId);
        page.setCallback(new PageCallback(getPage()));
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent arg0) {
        List<ServerGroup> groups = getServerGroups();
        if (groups == null) {
            setServerGroups(getFirewallManager().getServerGroups());
        }

        SelectMap selections = getSelections();
        if (selections == null) {
            setSelections(new SelectMap());
        }
    }
}
