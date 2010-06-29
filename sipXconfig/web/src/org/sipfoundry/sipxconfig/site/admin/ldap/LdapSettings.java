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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class LdapSettings extends BaseComponent implements PageBeginRenderListener {

    @InjectObject("spring:ldapManager")
    public abstract LdapManager getLdapManager();

    public abstract LdapSystemSettings getSettings();

    public abstract void setSettings(LdapSystemSettings settings);

    public void pageBeginRender(PageEvent event_) {
        setSettings(getLdapManager().getSystemSettings());
    }

    public void ok() {
        getLdapManager().saveSystemSettings(getSettings());
    }
}
