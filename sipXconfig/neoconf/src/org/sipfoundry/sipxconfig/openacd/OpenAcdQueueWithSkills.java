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
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;

public abstract class OpenAcdQueueWithSkills extends OpenAcdConfigObject {
    private static final String DELIM = ", ";

    private Set<OpenAcdSkill> m_skills = new LinkedHashSet<OpenAcdSkill>();
    private Set<OpenAcdAgentGroup> m_agentGroups = new LinkedHashSet<OpenAcdAgentGroup>();

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

    public Set<OpenAcdAgentGroup> getAgentGroups() {
        return m_agentGroups;
    }

    public void setAgentGroups(Set<OpenAcdAgentGroup> groups) {
        m_agentGroups = groups;
    }

    public void addAgentGroup(OpenAcdAgentGroup group) {
        m_agentGroups.add(group);
    }

    public void removeAgentGroup(OpenAcdAgentGroup group) {
        m_agentGroups.remove(group);
    }

    public String getSkillsAtoms() {
        List<String> atoms = new ArrayList<String>();
        for (OpenAcdSkill skill : m_skills) {
            atoms.add(skill.getAtom());
        }
        return StringUtils.join(atoms, DELIM);
    }

    public String getProfiles() {
        List<String> profiles = new ArrayList<String>();
        for (OpenAcdAgentGroup profile : m_agentGroups) {
            profiles.add(profile.getName());
        }
        return StringUtils.join(profiles, DELIM);
    }

}
