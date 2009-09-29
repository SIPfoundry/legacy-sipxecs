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

public class BeanWithGroupsModel extends BeanWithSettingsModel {
    private MulticastSettingValueHandler m_groupsHandler;

    public BeanWithGroupsModel(BeanWithGroups bean) {
        super(bean);
    }

    public void setGroups(Collection< ? extends SettingValueHandler> groups) {
        m_groupsHandler = new MulticastSettingValueHandler(groups);
    }

    @Override
    protected SettingValue getDefault(Setting setting) {
        SettingValue value = null;

        if (m_groupsHandler != null) {
            value = m_groupsHandler.getSettingValue(setting);
        }

        if (value == null) {
            value = getDefaultsHandler().getSettingValue(setting);
        }

        return value;
    }
}
