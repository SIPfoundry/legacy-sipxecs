/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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

import java.util.Arrays;
import java.util.Collection;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class LdapPage extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/ldap/LdapPage";
    private static final String CONFIGURATION_TAB = "configurationTarget";
    private static final String IMPORT_TAB = "importTarget";
    private static final String SETTINGS_TAB = "settingsTarget";

    @InjectObject(value = "spring:ldapManager")
    public abstract LdapManager getLdapManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InitialValue("literal:" + CONFIGURATION_TAB)
    @Persist
    public abstract String getTab();
    public abstract void setTab(String tab);

    public abstract LdapSelectionModel getLdapSelectionModel();
    public abstract void setLdapSelectionModel(LdapSelectionModel ldapSelectionModel);

    @Persist
    public abstract int getCurrentConnectionId();
    public abstract void setCurrentConnectionId(int connectionId);

    public abstract LdapSystemSettings getSettings();
    public abstract void setSettings(LdapSystemSettings settings);

    @Persist
    public abstract boolean isAddMode();

    public void pageBeginRender(PageEvent event) {
        setSettings(getLdapManager().getSystemSettings());
        setLdapSelectionModel(new LdapSelectionModel(getLdapManager()));
        if (getLdapSelectionModel().getOptionCount() > 0 && getCurrentConnectionId() <= 0) {
            if (!isAddMode()) {
                setCurrentConnectionId((Integer) getLdapSelectionModel().getOption(0));
            }
        }
    }

    public Collection<String> getAvailableTabNames() {
        if (!getLdapManager().getSystemSettings().isConfigured()) {
            return Arrays.asList(CONFIGURATION_TAB, IMPORT_TAB);
        }
        return Arrays.asList(CONFIGURATION_TAB, IMPORT_TAB, SETTINGS_TAB);
    }

    public boolean isConnectionStage() {
        LdapServer configurationPanel = (LdapServer) getComponent("configurationPanel");
        return StringUtils.equals(configurationPanel.getStage(), LdapServer.CONNECTION_STAGE);
    }

    public void applySettings() {
        // save system settings even if no valid connection - e.g. if uncheck LDAP configured
        getLdapManager().saveSystemSettings(getSettings());
    }

    public boolean isLdapConfigured() {
        return !getLdapManager().getAllConnectionParams().isEmpty() && isConnectionStage() && !isAddMode();
    }
}
