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
package org.sipfoundry.sipxconfig.openacd;

import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.LinkedList;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;

public class OpenAcdAgentGroup extends OpenAcdConfigObject {
    private String m_name;
    private String m_description;
    private Set<OpenAcdAgent> m_agents = new LinkedHashSet<OpenAcdAgent>();
    private String m_oldName;

    public String getName() {
        return m_name;
    }

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

    // TODO add real skills when supported
    public List<String> getSkills() {
        List<String> skills = new ArrayList<String>();
        skills.add("");
        return skills;
    }

    public String getOldName() {
        return m_oldName;
    }

    public void setOldName(String oldName) {
        m_oldName = oldName;
    }

    public int hashCode() {
        return new HashCodeBuilder().append(m_name).toHashCode();
    }

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
    public List<String> getProperties() {
        List<String> props = new LinkedList<String>();
        props.add("name");
        props.add("skills");
        props.add("oldName");
        return props;
    }

    @Override
    public String getType() {
        return "profile";
    }
}
