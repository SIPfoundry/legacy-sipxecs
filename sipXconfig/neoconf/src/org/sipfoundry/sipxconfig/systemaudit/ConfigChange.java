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

package org.sipfoundry.sipxconfig.systemaudit;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.UserIpAddress;

public class ConfigChange extends BeanWithId {

    private Date m_dateTime = new Date();
    private UserIpAddress m_userIpAddress;
    private ConfigChangeAction m_configChangeAction;
    private ConfigChangeType m_configChangeType;
    private String m_details;
    private List<ConfigChangeValue> m_values = new ArrayList<ConfigChangeValue>(0);

    public ConfigChange() {
    }

    public void addValue(ConfigChangeValue values) {
        m_values.add(values);
        values.setConfigChange(this);
    }

    public List<ConfigChangeValue> getValues() {
        return m_values;
    }

    public void setValues(List<ConfigChangeValue> values) {
        m_values = values;
    }

    public ConfigChangeType getConfigChangeType() {
        return m_configChangeType;
    }

    public void setConfigChangeType(ConfigChangeType configChangeType) {
        this.m_configChangeType = configChangeType;
    }

    public Date getDateTime() {
        return m_dateTime;
    }

    public void setDateTime(Date dateTime) {
        this.m_dateTime = dateTime;
    }

    public UserIpAddress getUserIpAddress() {
        return m_userIpAddress;
    }

    public void setUserIpAddress(UserIpAddress userIpAddress) {
        this.m_userIpAddress = userIpAddress;
    }

    public String getDetails() {
        return m_details;
    }

    public void setDetails(String identifier) {
        this.m_details = identifier;
    }

    public ConfigChangeAction getConfigChangeAction() {
        return m_configChangeAction;
    }

    public void setConfigChangeAction(ConfigChangeAction configChangeAction) {
        this.m_configChangeAction = configChangeAction;
    }

}
