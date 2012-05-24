/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.site.dhcp;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.dhcp.DhcpManager;
import org.sipfoundry.sipxconfig.dhcp.DhcpSettings;

public abstract class EditDhcpd extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "dhcp/EditDhcpd";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:dhcpManager")
    public abstract DhcpManager getDhcpManager();

    public abstract DhcpSettings getSettings();

    public abstract void setSettings(DhcpSettings settings);

    @Persist
    @InitialValue(value = "literal:dhcp")
    public abstract String getTab();

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getDhcpManager().getSettings());
        }
    }

    public void apply() {
        getDhcpManager().saveSettings(getSettings());
    }
}
