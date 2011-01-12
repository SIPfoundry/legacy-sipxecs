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

import org.apache.commons.lang.RandomStringUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.common.User;

public class OpenAcdAgent extends OpenAcdConfigObject {
    public static final String EMPTY_STRING = "";

    public static enum Security {
        AGENT {
            public String toString() {
                return "AGENT";
            }
        },
        SUPERVISOR {
            public String toString() {
                return "SUPERVISOR";
            }
        },
        ADMIN {
            public String toString() {
                return "ADMIN";
            }
        }
    }

    private OpenAcdAgentGroup m_group;
    private User m_user;
    private String m_pin = RandomStringUtils.randomAlphanumeric(8);
    private String m_security = Security.AGENT.toString(); // default 'AGENT'
    private String m_oldName;
    private Set<OpenAcdSkill> m_skills = new LinkedHashSet<OpenAcdSkill>();

    public OpenAcdAgentGroup getGroup() {
        return m_group;
    }

    public void setGroup(OpenAcdAgentGroup group) {
        m_group = group;
    }

    public User getUser() {
        return m_user;
    }

    public void setUser(User user) {
        m_user = user;
    }

    public String getPin() {
        return m_pin;
    }

    public void setPin(String pin) {
        m_pin = pin;
    }

    public String getSecurity() {
        return m_security;
    }

    public void setSecurity(String security) {
        m_security = security;
    }

    public Set<OpenAcdSkill> getSkills() {
        return m_skills;
    }

    public void setSkills(Set<OpenAcdSkill> skills) {
        m_skills = skills;
    }

    public void addSkill(OpenAcdSkill skill) {
        m_skills.add(skill);
    }

    public void removeSkill(OpenAcdSkill skill) {
        m_skills.remove(skill);
    }

    public String getSkillsAtoms() {
        List<String> atoms = new ArrayList<String>();
        for (OpenAcdSkill skill : m_skills) {
            atoms.add(skill.getAtom());
        }
        return StringUtils.join(atoms.iterator(), ", ");
    }

    public List<String> getQueues() {
        List<String> queues = new ArrayList<String>();
        queues.add(EMPTY_STRING);
        return queues;
    }

    public String getName() {
        return m_user.getUserName();
    }

    public String getFirstName() {
        return StringUtils.defaultString(m_user.getFirstName(), StringUtils.EMPTY);
    }

    public String getLastName() {
        return StringUtils.defaultString(m_user.getLastName(), StringUtils.EMPTY);
    }

    public String getAgentGroup() {
        return m_group.getName();
    }

    public String getOldName() {
        return m_oldName;
    }

    public void setOldName(String oldName) {
        m_oldName = oldName;
    }

    @Override
    public List<String> getProperties() {
        List<String> props = new LinkedList<String>();
        props.add("name");
        props.add("pin");
        props.add("agentGroup");
        props.add("skillsAtoms");
        props.add("queues");
        props.add("firstName");
        props.add("lastName");
        props.add("oldName");
        props.add("security");
        return props;
    }

    @Override
    public String getType() {
        return "agent";
    }

    public int hashCode() {
        return new HashCodeBuilder().append(m_group).append(m_user).toHashCode();
    }

    public boolean equals(Object other) {
        if (!(other instanceof OpenAcdAgent)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        OpenAcdAgent bean = (OpenAcdAgent) other;
        return new EqualsBuilder().append(m_group, bean.getGroup()).append(m_user, bean.getUser()).isEquals();
    }
}
