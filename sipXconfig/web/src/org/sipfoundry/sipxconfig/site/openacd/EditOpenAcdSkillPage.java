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
package org.sipfoundry.sipxconfig.site.openacd;

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
import org.sipfoundry.sipxconfig.openacd.OpenAcdSkillGroup;

public abstract class EditOpenAcdSkillPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "openacd/EditOpenAcdSkillPage";

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getSkillId();

    public abstract void setSkillId(Integer skillId);

    public abstract OpenAcdSkill getSkill();

    public abstract void setSkill(OpenAcdSkill skill);

    public abstract void setSelectedSkillGroup(OpenAcdSkillGroup selectedSkillGroup);

    public abstract OpenAcdSkillGroup getSelectedSkillGroup();

    public abstract boolean isDisabled();

    public abstract void setDisabled(boolean disabled);

    public void addSkill(String returnPage) {
        setSkillId(null);
        setReturnPage(returnPage);
    }

    public void editSkill(Integer skillId, String returnPage) {
        setSkillId(skillId);
        setReturnPage(returnPage);
    }

    @Override
    public void pageBeginRender(PageEvent event_) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        if (getSkillId() == null) {
            setSkill(new OpenAcdSkill());
        } else {
            OpenAcdSkill skill = getOpenAcdContext().getSkillById(getSkillId());
            setSkill(skill);
            setDisabled(true);
            OpenAcdSkillGroup skillGroup = skill.getGroup();
            if (skillGroup != null) {
                setSelectedSkillGroup(skillGroup);
            }
        }
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        OpenAcdSkill skill = getSkill();
        skill.setGroup(getSelectedSkillGroup());
        getOpenAcdContext().saveSkill(skill);
        setSkillId(getSkill().getId());
    }
}
