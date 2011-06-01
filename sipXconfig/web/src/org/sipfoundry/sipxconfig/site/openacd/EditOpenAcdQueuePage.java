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
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueue;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueueGroup;

public abstract class EditOpenAcdQueuePage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "openacd/EditOpenAcdQueuePage";

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    @Persist
    public abstract Integer getQueueId();

    public abstract void setQueueId(Integer queueId);

    public abstract OpenAcdQueue getQueue();

    public abstract void setQueue(OpenAcdQueue queue);

    public abstract void setSelectedQueueGroup(OpenAcdQueueGroup selectedQueueGroup);

    public abstract OpenAcdQueueGroup getSelectedQueueGroup();

    public abstract List<String> getInheritedSkills();

    public abstract void setInheritedSkills(List<String> skills);

    public abstract String getSkill();

    public abstract void setSkill(String skill);

    public void addQueue(String returnPage) {
        setQueueId(null);
        setQueue(null);
        setReturnPage(returnPage);
    }

    public void editQueue(Integer queueId, String returnPage) {
        setQueueId(queueId);
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

        if (getQueueId() == null) {
            setQueue(new OpenAcdQueue());
        } else {
            OpenAcdQueue queue = getOpenAcdContext().getQueueById(getQueueId());
            setQueue(queue);
            OpenAcdQueueGroup queueGroup = queue.getGroup();
            if (queueGroup != null) {
                setSelectedQueueGroup(queueGroup);
                setInheritedSkills(queueGroup.getAllSkillNames());
            }
        }
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        OpenAcdQueue queue = getQueue();
        if (getSelectedQueueGroup() == null) {
            throw new UserException(getMessages().getMessage("error.requiredQueueGroup"));
        }
        queue.setGroup(getSelectedQueueGroup());
        getOpenAcdContext().saveQueue(queue);
        setQueueId(getQueue().getId());
    }
}
