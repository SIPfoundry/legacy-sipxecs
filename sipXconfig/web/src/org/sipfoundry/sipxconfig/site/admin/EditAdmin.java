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
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Message;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.AdminSettings;
import org.sipfoundry.sipxconfig.admin.ResLimitsConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class EditAdmin extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "admin/EditAdmin";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:adminContext")
    public abstract AdminContext getAdminContext();

    @InjectObject("spring:configManager")
    public abstract ConfigManager getConfigManager();

    @InjectObject("spring:resLimitsConfiguration")
    public abstract ResLimitsConfiguration getResLimitsConfiguration();

    public abstract AdminSettings getSettings();

    public abstract void setSettings(AdminSettings settings);

    @Message(value = "error.resource-limits.default")
    public abstract String getResourceLimitsError();

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getAdminContext().getSettings());
        }
    }

    public void apply() {
        getAdminContext().saveSettings(getSettings());
    }
}
