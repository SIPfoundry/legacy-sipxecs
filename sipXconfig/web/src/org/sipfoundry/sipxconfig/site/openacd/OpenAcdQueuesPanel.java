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

import java.util.Collection;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdQueue;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class OpenAcdQueuesPanel extends BaseComponent implements PageBeginRenderListener {

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract List<OpenAcdQueue> getQueues();

    public abstract void setQueues(List<OpenAcdQueue> queues);

    public abstract Collection getSelectedRows();

    public abstract void setCurrentRow(OpenAcdQueue queue);

    public IPage addQueue(IRequestCycle cycle) {
        EditOpenAcdQueuePage page = (EditOpenAcdQueuePage) cycle.getPage(EditOpenAcdQueuePage.PAGE);
        page.addQueue(getPage().getPageName());
        return page;
    }

    public IPage editQueue(IRequestCycle cycle, Integer queueId) {
        EditOpenAcdQueuePage page = (EditOpenAcdQueuePage) cycle.getPage(EditOpenAcdQueuePage.PAGE);
        page.editQueue(queueId, getPage().getPageName());
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        List<OpenAcdQueue> queues = getOpenAcdContext().getQueues();
        setQueues(queues);
    }

    public void delete() {
        Collection ids = getSelections().getAllSelected();
        if (ids.isEmpty()) {
            return;
        }
        getOpenAcdContext().removeQueues(ids);
    }
}
