/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.registrar;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.registrar.Registrar;
import org.sipfoundry.sipxconfig.registrar.RegistrarSettings;

public abstract class EditRegistrar extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "registrar/EditRegistrar";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:registrar")
    public abstract Registrar getRegistrar();

    public abstract RegistrarSettings getSettings();

    public abstract void setSettings(RegistrarSettings settings);

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getRegistrar().getSettings());
        }
    }

    public void apply() {
        getRegistrar().saveSettings(getSettings());
    }
}
