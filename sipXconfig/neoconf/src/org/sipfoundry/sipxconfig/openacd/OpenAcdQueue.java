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

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;

public class OpenAcdQueue extends OpenAcdConfigObject {
    private String m_name;
    private String m_description;
    private OpenAcdQueueGroup m_group;
    private Set<OpenAcdSkill> m_skills = new LinkedHashSet<OpenAcdSkill>();
    private int m_weight = 1;
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

    public OpenAcdQueueGroup getGroup() {
        return m_group;
    }

    public void setGroup(OpenAcdQueueGroup group) {
        m_group = group;
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

    public int getWeight() {
        return m_weight;
    }

    public void setWeight(int weight) {
        m_weight = weight;
    }

    public String getQueueGroup() {
        return m_group.getName();
    }

    public String getSkillsAtoms() {
        List<String> atoms = new ArrayList<String>();
        for (OpenAcdSkill skill : m_skills) {
            atoms.add(skill.getAtom());
        }
        return StringUtils.join(atoms.iterator(), ", ");
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
        if (!(other instanceof OpenAcdQueue)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        OpenAcdQueue bean = (OpenAcdQueue) other;
        return new EqualsBuilder().append(m_name, bean.getName()).isEquals();
    }

    @Override
    public List<String> getProperties() {
        List<String> props = new LinkedList<String>();
        props.add("name");
        props.add("queueGroup");
        props.add("skillsAtoms");
        props.add("weight");
        props.add("oldName");
        return props;
    }

    @Override
    public String getType() {
        return "queue";
    }
}
