/**
 *
 * Copyright (c) 2013 Karel Electronics Corp. All rights reserved.
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
 *
 */

package org.sipfoundry.sipxconfig.site.admin.systemaudit;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.AdminSettings;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class ManageSystemAudit extends SipxBasePage implements PageBeginRenderListener {

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InjectObject("spring:adminContext")
    public abstract AdminContext getAdminContext();

    public abstract AdminSettings getSettings();

    public abstract void setSettings(AdminSettings settings);

    @Persist
    @InitialValue(value = "literal:history")
    public abstract String getTab();

    public abstract void setTab(String tab);

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

