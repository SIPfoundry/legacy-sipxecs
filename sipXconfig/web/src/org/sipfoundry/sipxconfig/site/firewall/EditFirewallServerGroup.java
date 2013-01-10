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


import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.ServerGroup;

public abstract class EditFirewallServerGroup extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "firewall/EditFirewallServerGroup";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getServerGroupId();

    public abstract void setServerGroupId(Integer serverGroupId);

    public abstract ServerGroup getServerGroup();

    public abstract void setServerGroup(ServerGroup serverGroup);

    @InjectObject("spring:firewallManager")
    public abstract FirewallManager getFirewallManager();

    @Override
    public void pageBeginRender(PageEvent arg0) {
        ServerGroup group = getServerGroup();
        if (group == null) {
            Integer id = getServerGroupId();
            if (id == null) {
                group = new ServerGroup();
            } else {
                group = getFirewallManager().getServerGroup(id);
            }
            setServerGroup(group);
        }
    }

    public void save() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        if (StringUtils.contains(getServerGroup().getServerList(), ",")) {
            getValidator().record(new ValidatorException(getMessages().getMessage("err.wrongformat")));
            return;
        }
        getFirewallManager().saveServerGroup(getServerGroup());
    }
}
