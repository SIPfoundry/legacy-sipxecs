/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.util.Collection;

public class MulticastSettingValueHandler implements SettingValueHandler {
    private Collection< ? extends SettingValueHandler> m_resolvers;

    public MulticastSettingValueHandler(Collection< ? extends SettingValueHandler> resolvers) {
        m_resolvers = resolvers;
    }

    public SettingValue getSettingValue(Setting setting) {
        SettingValue value = null;
        for (SettingValueHandler resolver : m_resolvers) {
            SettingValue sv = resolver.getSettingValue(setting);
            if (sv != null) {
                value = sv;
            }
        }

        return value;
    }
}
