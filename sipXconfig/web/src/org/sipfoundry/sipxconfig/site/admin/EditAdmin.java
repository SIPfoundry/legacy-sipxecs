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

import org.apache.commons.lang.BooleanUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.AdminSettings;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class EditAdmin extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "admin/EditAdmin";
    private static final String OLD = "old";
    private static final String NEW = "new";
    private static final String USER_PORTAL_PATH = "user-portal/old-portal";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:adminContext")
    public abstract AdminContext getAdminContext();

    @InjectObject("spring:configManager")
    public abstract ConfigManager getConfigManager();

    public abstract AdminSettings getSettings();

    public abstract void setSettings(AdminSettings settings);

    public abstract String getWebPortal();

    public abstract void setWebPortal(String portal);

    @Override
    public void pageBeginRender(PageEvent arg0) {
        if (getSettings() == null) {
            setSettings(getAdminContext().getSettings());
        }
        if (getWebPortal() == null) {
            setWebPortal((Boolean) getAdminContext().getSettings().getSettingTypedValue(USER_PORTAL_PATH) ? OLD
                    : NEW);
        }
    }

    public void apply() {
        getSettings().setSettingTypedValue(USER_PORTAL_PATH,
                BooleanUtils.toBoolean(getWebPortal(), OLD, NEW));
        getAdminContext().saveSettings(getSettings());
    }
}
