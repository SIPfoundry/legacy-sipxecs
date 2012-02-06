/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.dns;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.dns.DnsManager;
import org.sipfoundry.sipxconfig.dns.DnsSettings;

public abstract class EditDns extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "dns/EditDns";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:dnsManager")
    public abstract DnsManager getDnsManager();

    public abstract DnsSettings getSettings();

    public abstract void setSettings(DnsSettings settings);

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getDnsManager().getSettings());
        }
    }

    public void apply() {
        getDnsManager().saveSettings(getSettings());
    }
}
