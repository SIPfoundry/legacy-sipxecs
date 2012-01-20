/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.ivr;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.ivr.IvrSettings;

public abstract class EditIvr extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "ivr/EditIvr";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:ivr")
    public abstract Ivr getIvr();

    public abstract IvrSettings getSettings();

    public abstract void setSettings(IvrSettings settings);

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getIvr().getSettings());
        }
    }

    public void apply() {
        getIvr().saveSettings(getSettings());
    }
}
