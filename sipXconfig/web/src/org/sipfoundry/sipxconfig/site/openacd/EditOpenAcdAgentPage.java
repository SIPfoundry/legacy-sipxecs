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

    public void editAgent(Integer groupId, String returnPage) {
        setAgentId(groupId);
        setAgent(getOpenAcdContext().getAgentById(groupId));
        setReturnPage(returnPage);
    }

    @Override
    public void pageBeginRender(PageEvent event_) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        getOpenAcdContext().saveAgent(getAgent());
    }

    public IPropertySelectionModel getAgentSecurityModel() {
        return new StringPropertySelectionModel(new String[] {
            OpenAcdAgent.Security.AGENT.toString(), OpenAcdAgent.Security.SUPERVISOR.toString(),
            OpenAcdAgent.Security.ADMIN.toString()
        });
    }
}
