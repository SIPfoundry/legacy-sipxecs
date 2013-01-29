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
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.Replicable;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;

import com.mongodb.BasicDBObject;

public class OpenAcdQueue extends OpenAcdQueueWithSkills implements Replicable, DeployConfigOnEdit,
    OpenAcdObjectWithRecipe {
    private String m_name;
    private String m_description;
    private OpenAcdQueueGroup m_group;
    private int m_weight = 1;
    private Set<OpenAcdRecipeStep> m_steps = new LinkedHashSet<OpenAcdRecipeStep>();
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

    public int getWeight() {
        return m_weight;
    }

    public void setWeight(int weight) {
        m_weight = weight;
    }

    public String getQueueGroup() {
        return m_group.getName();
    }

    public Set<OpenAcdRecipeStep> getSteps() {
        return m_steps;
    }

    public void setSteps(Set<OpenAcdRecipeStep> steps) {
        m_steps = steps;
    }

    public void addStep(OpenAcdRecipeStep step) {
        m_steps.add(step);
    }

    public void removeStep(OpenAcdRecipeStep step) {
        m_steps.remove(step);
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
        props.put(OpenAcdContext.QUEUE_GROUP, getQueueGroup());
        List<String> skills = new ArrayList<String>();
        for (OpenAcdSkill skill : getSkills()) {
            skills.add(skill.getAtom());
        }
        props.put(OpenAcdContext.SKILLS, skills);
        List<String> profiles = new ArrayList<String>();
        for (OpenAcdAgentGroup profile : getAgentGroups()) {
            profiles.add(profile.getName());
        }
        props.put(OpenAcdContext.PROFILES, profiles);
        props.put(OpenAcdContext.WEIGHT, getWeight());
        props.put(OpenAcdContext.OLD_NAME, getOldName());

        props.put(OpenAcdContext.RECIPES, constructRecipeMongoObject(m_steps));
        return props;
    }

    public static List<BasicDBObject> constructRecipeMongoObject(Set<OpenAcdRecipeStep> steps) {
        List<BasicDBObject> objects = new ArrayList<BasicDBObject>();
        for (OpenAcdRecipeStep step : steps) {
            BasicDBObject recipeStep = new BasicDBObject();
            recipeStep.put(OpenAcdContext.ACTION, step.getAction().getMongoObject());
            List<BasicDBObject> conditions = new ArrayList<BasicDBObject>();
            for (OpenAcdRecipeCondition condition : step.getConditions()) {
                conditions.add(condition.getMongoObject());
            }
            recipeStep.put(OpenAcdContext.CONDITION, conditions);
            recipeStep.put(OpenAcdContext.FREQUENCY, step.getFrequency());
            recipeStep.put(OpenAcdContext.STEP_NAME, "New Step");
            objects.add(recipeStep);
        }
        return objects;
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) FreeswitchFeature.FEATURE);
    }

    @Override
    public String getEntityName() {
        return getClass().getSimpleName();
    }
}
