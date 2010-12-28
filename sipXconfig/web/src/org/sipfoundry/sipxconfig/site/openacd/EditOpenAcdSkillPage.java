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

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdSkill;

public abstract class EditOpenAcdSkillPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "openacd/EditOpenAcdSkillPage";

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getSkillId();

    public abstract void setSkillId(Integer skillId);

    @Persist
    public abstract OpenAcdSkill getSkill();

    public abstract void setSkill(OpenAcdSkill skill);

    public abstract int getIndex();

    public abstract void setIndex(int i);

    public abstract String getFilter();

    public abstract void setFilter(String filter);

    public void addSkill(String returnPage) {
        setSkillId(null);
        setSkill(null);
        setReturnPage(returnPage);
    }

    public void editSkill(Integer skillId, String returnPage) {
        setSkillId(skillId);
        setSkill(getOpenAcdContext().getSkillById(skillId));
        setReturnPage(returnPage);
    }

    @Override
    public void pageBeginRender(PageEvent event_) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        if (getSkill() == null && getSkillId() == null) {
            setSkill(new OpenAcdSkill());
        }
    }

    public String[] getSkillGroupNames() {
        String filter = getFilter();
        List<OpenAcdSkill> skills = getOpenAcdContext().getSkills();
        Set<String> groupNames = new HashSet<String>();
        for (OpenAcdSkill skill : skills) {
            groupNames.add(skill.getGroupName());
        }
        String[] names = groupNames.toArray(new String[0]);

        if (filter == null || filter.length() < 1) {
            return names;
        }
        List<String> temp = new ArrayList<String>();
        for (String app : names) {
            if (app.startsWith(filter)) {
                temp.add(app);
            }
        }
        return temp.toArray(new String[0]);
    }

    public void filterList(String filter) {
        setFilter(filter);
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        getOpenAcdContext().saveSkill(getSkill());
    }
}
