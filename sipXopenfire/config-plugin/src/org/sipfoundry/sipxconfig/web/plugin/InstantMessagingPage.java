/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.web.plugin;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.openfire.Openfire;
import org.sipfoundry.sipxconfig.openfire.OpenfireSettings;

public abstract class InstantMessagingPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "plugin/InstantMessagingPage";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:openfire")
    public abstract Openfire getOpenfire();

    public abstract OpenfireSettings getSettings();

    public abstract void setSettings(OpenfireSettings settings);

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getOpenfire().getSettings());
        }
    }

    public void apply() {
        getOpenfire().saveSettings(getSettings());
    }
}
