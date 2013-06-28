/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.site.admin.ldap;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.AdminSettings;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;
import org.sipfoundry.sipxconfig.bulk.ldap.OverwritePinBean;

public abstract class LdapManagementSettings extends BaseComponent implements PageBeginRenderListener {

    @InjectObject(value = "spring:ldapManager")
    public abstract LdapManager getLdapManager();

    public abstract LdapSystemSettings getSettings();

    public abstract void setSettings(LdapSystemSettings settings);

    public abstract Boolean getOverwritePin();

    public abstract void setOverwritePin(Boolean overwritePin);

    @InjectObject("spring:adminContext")
    public abstract AdminContext getAdminContext();

    public abstract AdminSettings getAdminSettings();

    public abstract void setAdminSettings(AdminSettings adminSettings);

    @Override
    public void pageBeginRender(PageEvent event) {
        if (getOverwritePin() == null) {
            OverwritePinBean overwritePinBean = getLdapManager().retriveOverwritePin();
            setOverwritePin(overwritePinBean == null ? true : overwritePinBean.isValue());
        }
        if (getAdminSettings() == null) {
            setAdminSettings(getAdminContext().getSettings());
        }

        LdapManager manager = getLdapManager();
        setSettings(manager.getSystemSettings());
    }

    public void submitManagementSettings() {
        // save system settings even if no valid connection - e.g. if uncheck LDAP configured
        getLdapManager().saveSystemSettings(getSettings());
        getLdapManager().saveOverwritePin(getOverwritePin());
        getAdminContext().saveSettings(getAdminSettings());
    }
}
