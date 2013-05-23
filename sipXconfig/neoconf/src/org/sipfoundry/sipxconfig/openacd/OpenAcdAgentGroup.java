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

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;

public class OpenAcdAgentGroup extends OpenAcdAgentWithSkills implements Replicable {
    private String m_name;
    private String m_description;
    private Set<OpenAcdAgent> m_agents = new LinkedHashSet<OpenAcdAgent>();
    private String m_oldName;

    @Override
    public String getName() {
        return m_name;
    }

    @Override
    public void setName(String name) {
        m_name = name;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public Set<OpenAcdAgent> getAgents() {
        return m_agents;
    }

    public void setAgents(Set<OpenAcdAgent> agents) {
        m_agents = agents;
    }

    public void addAgent(OpenAcdAgent agent) {
        m_agents.add(agent);
    }

    public void removeAgent(OpenAcdAgent agent) {
        m_agents.remove(agent);
    }

    public String getOldName() {
        return m_oldName;
    }

    public void setOldName(String oldName) {
        m_oldName = oldName;
    }

    @Override
    public int hashCode() {
        return new HashCodeBuilder().append(m_name).toHashCode();
    }

    @Override
    public boolean equals(Object other) {
        if (!(other instanceof OpenAcdAgentGroup)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        OpenAcdAgentGroup bean = (OpenAcdAgentGroup) other;
        return new EqualsBuilder().append(m_name, bean.getName()).isEquals();
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        Map<String, Object> props = new HashMap<String, Object>();
        List<String> skills = new ArrayList<String>();
        for (OpenAcdSkill skill : getSkills()) {
            skills.add(skill.getAtom());
        }
        props.put(OpenAcdContext.SKILLS, skills);

        props.put(OpenAcdContext.OLD_NAME, getOldName());

        List<String> queues = new ArrayList<String>();
        for (OpenAcdQueue queue : getQueues()) {
            queues.add(queue.getName());
        }
        props.put(OpenAcdContext.QUEUES, queues);
        return props;
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
    public String getEntityName() {
        return getClass().getSimpleName();
    }

    @Override
    public boolean isEnabled() {
        return true;
    }
}
