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

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class LdapPage extends SipxBasePage {
    public static final String PAGE = "admin/ldap/LdapPage";
    private static final String CONFIGURATION_TAB = "configurationTarget";
    private static final String IMPORT_TAB = "importTarget";
    private static final String SETTINGS_TAB = "settingsTarget";

    @InjectObject(value = "spring:ldapManager")
    public abstract LdapManager getLdapManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @InitialValue("literal:" + CONFIGURATION_TAB)
    public abstract String getTab();

    public abstract void setTab(String tab);

    public Collection<String> getAvailableTabNames() {
        if (!getLdapManager().getSystemSettings().isConfigured()) {
            return Arrays.asList(CONFIGURATION_TAB, IMPORT_TAB);
        }
        return Arrays.asList(CONFIGURATION_TAB, IMPORT_TAB, SETTINGS_TAB);
    }
}
