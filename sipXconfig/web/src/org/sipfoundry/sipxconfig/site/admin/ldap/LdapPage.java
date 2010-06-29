/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.site.admin.ldap;

import java.util.Arrays;
import java.util.Collection;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Persist;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class LdapPage extends SipxBasePage {
    public static final String PAGE = "admin/ldap/LdapPage";
    public static final String IMPORT_TAB = "importTarget";
    private static final String SETTINGS_TAB = "settingsTarget";

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    @InitialValue("literal:" + IMPORT_TAB)
    public abstract String getTab();

    public abstract void setTab(String tab);

    public Collection<String> getAvailableTabNames() {
        return Arrays.asList(IMPORT_TAB, SETTINGS_TAB);
    }
}
