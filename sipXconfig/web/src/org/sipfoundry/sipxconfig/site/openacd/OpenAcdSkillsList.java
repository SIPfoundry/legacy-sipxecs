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
package org.sipfoundry.sipxconfig.site.openacd;

import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgent;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgentGroup;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdSkill;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class OpenAcdSkillsList extends BaseComponent {

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Parameter
    public abstract OpenAcdAgentGroup getAgentGroup();

    @Parameter
    public abstract OpenAcdAgent getAgent();

    public abstract OpenAcdSkill getCurrentSkill();

    // map to hold skill and boolean selected, true if skill associated with group
    public abstract Map<OpenAcdSkill, Boolean> getSkillSelections();

    public abstract void setSkillSelections(Map<OpenAcdSkill, Boolean> selections);

    public abstract Map<String, List<OpenAcdSkill>> getGroupedSkills();

    public abstract void setGroupedSkills(Map<String, List<OpenAcdSkill>> skills);

    public abstract String getGroupName();

    public abstract void setGroupName(String groupName);

    public abstract int getSkillsSize();

    public abstract void setSkillsSize(int size);

    @Override
    protected void prepareForRender(IRequestCycle cycle) {
        super.prepareForRender(cycle);
        if (getSkillSelections() == null) {
            setSkillSelections(new HashMap<OpenAcdSkill, Boolean>());
        }

        setGroupedSkills(getOpenAcdContext().getGroupedSkills());
        // initialize skills map, mark all skills as unselected
        List<OpenAcdSkill> skills = getOpenAcdContext().getSkills();
        for (OpenAcdSkill skill : skills) {
            getSkillSelections().put(skill, false);
        }
        // set the number of options to show in skills select component
        setSkillsSize(getGroupedSkills().keySet().size() + skills.size());
        Set<OpenAcdSkill> assignedSkills = null;
        if (getAgentGroup() != null) {
            assignedSkills = getAgentGroup().getSkills();
        } else if (getAgent() != null) {
            assignedSkills = getAgent().getSkills();
        }
        if (assignedSkills != null) {
            for (OpenAcdSkill skill : assignedSkills) {
                // mark skills associated with this group as selected
                getSkillSelections().put(skill, true);
            }
        }
    }

    @Override
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        super.renderComponent(writer, cycle);
        if (TapestryUtils.isRewinding(cycle, this)) {
            afterRewind(cycle);
        }
    }

    private void afterRewind(IRequestCycle cycle) {
        Set<OpenAcdSkill> skills = new LinkedHashSet<OpenAcdSkill>();
        for (OpenAcdSkill skill : getSkillSelections().keySet()) {
            if (getSkillSelections().get(skill)) {
                skills.add(skill);
            }
        }
        if (getAgentGroup() != null) {
            getAgentGroup().setSkills(skills);
        } else if (getAgent() != null) {
            getAgent().setSkills(skills);
        }
    }
}
