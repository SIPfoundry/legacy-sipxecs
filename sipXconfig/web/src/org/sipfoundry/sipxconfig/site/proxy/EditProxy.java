/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.proxy;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.proxy.ProxySettings;

public abstract class EditProxy extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "proxy/EditProxy";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:proxyManager")
    public abstract ProxyManager getProxyManager();

    public abstract ProxySettings getSettings();

    public abstract void setSettings(ProxySettings settings);

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getProxyManager().getSettings());
        }
    }

    public void apply() {
        getProxyManager().saveSettings(getSettings());
    }
}
