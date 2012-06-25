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

import java.util.List;

import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgent;
import org.sipfoundry.sipxconfig.openacd.OpenAcdAgentGroup;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;

public abstract class EditOpenAcdAgentPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "openacd/EditOpenAcdAgentPage";

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Persist
    public abstract Integer getAgentId();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract void setAgentId(Integer agentId);

    @Persist
    public abstract OpenAcdAgent getAgent();

    public abstract void setAgent(OpenAcdAgent agent);

    public abstract void setSelectedAgentGroup(OpenAcdAgentGroup selectedAgentGroup);

    public abstract OpenAcdAgentGroup getSelectedAgentGroup();

    public abstract List<String> getInheritedSkills();

    public abstract void setInheritedSkills(List<String> skills);

    public abstract String getSkill();

    public abstract void setSkill(String skill);

    public void editAgent(Integer groupId, String returnPage) {
        setAgentId(groupId);
        setAgent(getOpenAcdContext().getAgentById(groupId));
        setReturnPage(returnPage);
    }

    public IPage addSkills(IRequestCycle cycle) {
        OpenAcdServerPage page = (OpenAcdServerPage) cycle.getPage(OpenAcdServerPage.PAGE);
        page.setTab("skills");
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        OpenAcdAgentGroup group = getAgent().getGroup();
        if (group != null) {
            setSelectedAgentGroup(group);
            setInheritedSkills(group.getAllSkillNames());
        }
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        OpenAcdAgent agent = getAgent();
        agent.setGroup(getSelectedAgentGroup());
        getOpenAcdContext().saveAgent(agent);
    }

    public IPropertySelectionModel getAgentSecurityModel() {
        return new StringPropertySelectionModel(new String[] {
            OpenAcdAgent.Security.AGENT.toString(), OpenAcdAgent.Security.SUPERVISOR.toString(),
            OpenAcdAgent.Security.ADMIN.toString()
        });
    }
}
