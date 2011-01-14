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
import org.sipfoundry.sipxconfig.openacd.OpenAcdClient;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class OpenAcdClientsPanel extends BaseComponent implements PageBeginRenderListener {

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SelectMap getSelections();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract void setCurrentRow(OpenAcdClient client);

    public abstract Collection getSelectedRows();

    public IPage addClient(IRequestCycle cycle) {
        EditOpenAcdClientPage page = (EditOpenAcdClientPage) cycle.getPage(EditOpenAcdClientPage.PAGE);
        page.addClient(getPage().getPageName());
        return page;
    }

    public IPage editClient(IRequestCycle cycle, Integer clientId) {
        EditOpenAcdClientPage page = (EditOpenAcdClientPage) cycle.getPage(EditOpenAcdClientPage.PAGE);
        page.editClient(clientId, getPage().getPageName());
        return page;
    }

    @Override
    public void pageBeginRender(PageEvent event) {

    }

    public void delete() {
        Collection clientIds = getSelections().getAllSelected();
        if (clientIds.isEmpty()) {
            return;
        }
        getOpenAcdContext().removeClients(clientIds);
    }
}
