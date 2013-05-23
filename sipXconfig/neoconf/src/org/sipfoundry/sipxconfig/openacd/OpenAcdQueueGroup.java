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

public class OpenAcdQueueGroup extends OpenAcdQueueWithSkills implements Replicable, OpenAcdObjectWithRecipe {
    private String m_name;
    private String m_description;
    private Set<OpenAcdQueue> m_queues = new LinkedHashSet<OpenAcdQueue>();
    private String m_oldName;
    private Set<OpenAcdRecipeStep> m_steps = new LinkedHashSet<OpenAcdRecipeStep>();

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

    public String getOldName() {
        return m_oldName;
    }

    public void setOldName(String oldName) {
        m_oldName = oldName;
    }

    @Override
    public Set<OpenAcdRecipeStep> getSteps() {
        return m_steps;
    }

    @Override
    public void setSteps(Set<OpenAcdRecipeStep> steps) {
        m_steps = steps;
    }

    @Override
    public void addStep(OpenAcdRecipeStep step) {
        m_steps.add(step);
    }

    @Override
    public void removeStep(OpenAcdRecipeStep step) {
        m_steps.remove(step);
    }

    @Override
    public int hashCode() {
        return new HashCodeBuilder().append(m_name).toHashCode();
    }

    @Override
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
    public Map<String, Object> getMongoProperties(String domain) {
        Map<String, Object> props = new HashMap<String, Object>();
        List<String> skills = new ArrayList<String>();
        for (OpenAcdSkill skill : getSkills()) {
            skills.add(skill.getAtom());
        }
        props.put(OpenAcdContext.SKILLS, skills);

        List<String> profiles = new ArrayList<String>();
        for (OpenAcdAgentGroup profile : getAgentGroups()) {
            profiles.add(profile.getName());
        }
        props.put(OpenAcdContext.RECIPES, OpenAcdQueue.constructRecipeMongoObject(domain, m_steps));
        props.put(OpenAcdContext.PROFILES, profiles);
        props.put(OpenAcdContext.OLD_NAME, getOldName());
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
