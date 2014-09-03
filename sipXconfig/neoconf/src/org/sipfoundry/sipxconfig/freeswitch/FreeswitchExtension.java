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
package org.sipfoundry.sipxconfig.freeswitch;

import java.util.LinkedHashSet;
import java.util.Set;

import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.systemaudit.SystemAuditable;

public abstract class FreeswitchExtension extends BeanWithSettings implements SystemAuditable {
    private String m_name;
    private boolean m_enabled = true; // default enabled
    private String m_description;
    private String m_alias;
    private String m_did;
    private Set<FreeswitchCondition> m_conditions;

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

    public Set<FreeswitchCondition> getConditions() {
        return m_conditions;
    }

    public void setConditions(Set<FreeswitchCondition> conditions) {
        m_conditions = conditions;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public void addCondition(FreeswitchCondition condition) {
        if (m_conditions == null) {
            m_conditions = new LinkedHashSet<FreeswitchCondition>();
        }
        if (condition != null) {
            m_conditions.add(condition);
        }
    }

    public String getAlias() {
        return m_alias;
    }

    public void setAlias(String alias) {
        m_alias = alias;
    }

    public String getDid() {
        return m_did;
    }

    public void setDid(String did) {
        m_did = did;
    }

    @Override
    public String getEntityIdentifier() {
        return getName();
    }

    @Override
    public String getConfigChangeType() {
        return getClass().getSimpleName();
    }

}
