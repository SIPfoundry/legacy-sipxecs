/*
 * Copyright (C) 2012 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.imbot;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.imbot.ImBot;
import org.sipfoundry.sipxconfig.imbot.ImBotSettings;

public abstract class EditIMBot extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "imbot/EditIMBot";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:imBot")
    public abstract ImBot getIMBot();

    public abstract ImBotSettings getSettings();

    public abstract void setSettings(ImBotSettings settings);

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getIMBot().getSettings());
        }
    }

    public void apply() {
        getIMBot().saveSettings(getSettings());
    }
}
