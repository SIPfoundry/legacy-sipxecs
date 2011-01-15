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

public class OpenAcdQueueGroup extends OpenAcdConfigObject {
    private String m_name;
    private int m_sort = 10;
    private String m_description;
    private Set<OpenAcdQueue> m_queues = new LinkedHashSet<OpenAcdQueue>();
    private Set<OpenAcdSkill> m_skills = new LinkedHashSet<OpenAcdSkill>();
    private String m_oldName;

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public int getSort() {
        return m_sort;
    }

    public void setSort(int sort) {
        m_sort = sort;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public Set<OpenAcdQueue> getQueues() {
        return m_queues;
    }

    public void setQueues(Set<OpenAcdQueue> queues) {
        m_queues = queues;
    }

    public void addQueue(OpenAcdQueue queue) {
        m_queues.add(queue);
    }

    public void removeQueue(OpenAcdQueue queue) {
        m_queues.remove(queue);
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

    public String getOldName() {
        return m_oldName;
    }

    public void setOldName(String oldName) {
        m_oldName = oldName;
    }

    public String getSkillsAtoms() {
        List<String> atoms = new ArrayList<String>();
        for (OpenAcdSkill skill : m_skills) {
            atoms.add(skill.getAtom());
        }
        return StringUtils.join(atoms.iterator(), ", ");
    }

    public int hashCode() {
        return new HashCodeBuilder().append(m_name).toHashCode();
    }

    public boolean equals(Object other) {
        if (!(other instanceof OpenAcdQueueGroup)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        OpenAcdQueueGroup bean = (OpenAcdQueueGroup) other;
        return new EqualsBuilder().append(m_name, bean.getName()).isEquals();
    }

    @Override
    public List<String> getProperties() {
        List<String> props = new LinkedList<String>();
        props.add("name");
        props.add("skillsAtoms");
        props.add("sort");
        props.add("oldName");
        return props;
    }

    @Override
    public String getType() {
        return "queueGroup";
    }
}
