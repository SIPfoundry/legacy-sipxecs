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
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueueGroup;

public abstract class EditOpenAcdQueueGroupPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "openacd/EditOpenAcdQueueGroupPage";

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    @Persist
    public abstract Integer getGroupId();

    public abstract void setGroupId(Integer groupId);

    public abstract OpenAcdQueueGroup getQueueGroup();

    public abstract void setQueueGroup(OpenAcdQueueGroup queueGroup);

    public void addQueueGroup(String returnPage) {
        setGroupId(null);
        setQueueGroup(null);
        setReturnPage(returnPage);
    }

    public void editQueueGroup(Integer groupId, String returnPage) {
        setGroupId(groupId);
        setReturnPage(returnPage);
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        if (getGroupId() == null) {
            setQueueGroup(new OpenAcdQueueGroup());
        } else {
            OpenAcdQueueGroup queueGroup = getOpenAcdContext().getQueueGroupById(getGroupId());
            setQueueGroup(queueGroup);
        }
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        OpenAcdQueueGroup group = getQueueGroup();
        getOpenAcdContext().saveQueueGroup(group);
        setGroupId(getQueueGroup().getId());
    }
}
