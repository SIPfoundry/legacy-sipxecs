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
package org.sipfoundry.sipxconfig.openacd;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;

public class OpenAcdSkill extends BeanWithId implements Replicable {
    private String m_name;
    private String m_atom;
    private OpenAcdSkillGroup m_group;
    private String m_description;
    private boolean m_defaultSkill;

    @Override
    public String getName() {
        return m_name;
    }

    @Override
    public void setName(String name) {
        m_name = name;
    }

    public String getAtom() {
        return m_atom;
    }

    public void setAtom(String atom) {
        m_atom = atom;
    }

    public OpenAcdSkillGroup getGroup() {
        return m_group;
    }

    public void setGroup(OpenAcdSkillGroup group) {
        m_group = group;
    }

    public String getGroupName() {
        if (getGroup() != null) {
            return getGroup().getName();
        }
        return null;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public boolean isDefaultSkill() {
        return m_defaultSkill;
    }

    public void setDefaultSkill(boolean defaultSkill) {
        m_defaultSkill = defaultSkill;
    }

    @Override
    public int hashCode() {
        return new HashCodeBuilder().append(m_name).append(m_atom).toHashCode();
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof OpenAcdSkill)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        OpenAcdSkill bean = (OpenAcdSkill) other;
        return new EqualsBuilder().append(m_name, bean.getName()).append(m_atom, bean.getAtom()).isEquals();
    }

    @Override
    public Set<DataSet> getDataSets() {
        return Collections.singleton(DataSet.OPENACD);
    }

    @Override
    public String getIdentity(String domainName) {
        return null;
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domainName) {
        return null;
    }

    @Override
    public boolean isValidUser() {
        return false;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        Map<String, Object> props = new HashMap<String, Object>();
        props.put(OpenAcdContext.ATOM, getAtom());
        props.put(OpenAcdContext.GROUP_NAME, getGroupName());
        props.put(OpenAcdContext.DESCRIPTION, getDescription());
        return props;
    }

    @Override
    public String getEntityName() {
        return getClass().getSimpleName();
    }

    @Override
    public boolean isEnabled() {
        return true;
    }
}
