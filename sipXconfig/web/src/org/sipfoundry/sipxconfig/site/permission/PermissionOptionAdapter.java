/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.permission;

import java.util.Locale;

import org.sipfoundry.sipxconfig.components.selection.OptionAdapter;
import org.sipfoundry.sipxconfig.permission.Permission;

public class PermissionOptionAdapter implements OptionAdapter<Permission> {
    private Locale m_locale;

    public void setLocale(Locale locale) {
        m_locale = locale;
    }

    public String getLabel(Permission option, int index) {
        return option.getLabel(m_locale);
    }

    public Object getValue(Permission option, int index) {
        return option;
    }

    public String squeezeOption(Permission option, int index) {
        return option.getName();
    }
}
