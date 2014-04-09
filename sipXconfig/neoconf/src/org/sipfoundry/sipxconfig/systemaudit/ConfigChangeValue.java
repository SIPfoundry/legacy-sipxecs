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

import org.sipfoundry.sipxconfig.common.BeanWithId;

public class ConfigChangeValue extends BeanWithId {

    private ConfigChange m_configChange;
    private String m_propertyName;
    private String m_valueBefore;
    private String m_valueAfter;

    public ConfigChangeValue() {
    }

    public ConfigChangeValue(ConfigChange configChange) {
        super();
        this.m_configChange = configChange;
    }

    public ConfigChange getConfigChange() {
        return m_configChange;
    }

    public void setConfigChange(ConfigChange configChange) {
        this.m_configChange = configChange;
    }

    public String getValueBefore() {
        return m_valueBefore;
    }

    public void setValueBefore(String valueBefore) {
        this.m_valueBefore = valueBefore;
    }

    public String getValueAfter() {
        return m_valueAfter;
    }

    public void setValueAfter(String valueAfter) {
        this.m_valueAfter = valueAfter;
    }

    public String getPropertyName() {
        return m_propertyName;
    }

    public void setPropertyName(String propertyName) {
        this.m_propertyName = propertyName;
    }
}
