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
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.BeanWithId;

public abstract class OpenAcdQueueWithSkills extends BeanWithId {
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

    public Collection<String> getSkillsAtoms() {
        List<String> atoms = new ArrayList<String>();
        for (OpenAcdSkill skill : m_skills) {
            atoms.add(skill.getAtom());
        }
        return atoms;
    }

    public String getProfiles() {
        List<String> profiles = new ArrayList<String>();
        for (OpenAcdAgentGroup profile : m_agentGroups) {
            profiles.add(profile.getName());
        }
        return StringUtils.join(profiles, DELIM);
    }

    public List<String> getAllSkillNames() {
        List<String> skills = new ArrayList<String>();
        for (OpenAcdSkill skill : getSkills()) {
            skills.add(skill.getName());
        }
        for (OpenAcdAgentGroup profile : getAgentGroups()) {
            skills.add(profile.getName());
        }

        return skills;
    }

}
