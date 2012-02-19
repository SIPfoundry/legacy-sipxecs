/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.mwi;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.mwi.Mwi;
import org.sipfoundry.sipxconfig.mwi.MwiSettings;

public abstract class EditMwi extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "mwi/EditMwi";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:mwi")
    public abstract Mwi getMwi();

    public abstract MwiSettings getSettings();

    public abstract void setSettings(MwiSettings settings);

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getMwi().getSettings());
        }
    }

    public void apply() {
        getMwi().saveSettings(getSettings());
    }
}
