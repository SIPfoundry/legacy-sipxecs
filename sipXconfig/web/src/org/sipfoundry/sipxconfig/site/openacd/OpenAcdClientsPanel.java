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

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdClient;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContextImpl.ClientInUseException;

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
        Collection<Integer> clientIds = getSelections().getAllSelected();
        if (clientIds.isEmpty()) {
            return;
        }
        try {
            List<String> clients = new ArrayList<String>();
            for (Integer client : clientIds) {
                OpenAcdClient c = getOpenAcdContext().getClientById(client);
                try {
                    getOpenAcdContext().deleteClient(c);
                } catch (ClientInUseException e) {
                    clients.add(c.getName());
                }
            }

            if (!clients.isEmpty()) {
                String queueNames = StringUtils.join(clients.iterator(), ", ");
                String errMessage = getMessages().format("msg.err.queueClient", queueNames);
                IValidationDelegate validator = TapestryUtils.getValidator(getPage());
                validator.record(new ValidatorException(errMessage));
            }
        } catch (UserException ex) {
            IValidationDelegate validator = TapestryUtils.getValidator(getPage());
            validator.record(new ValidatorException(getMessages().getMessage("msg.cannot.connect")));
        }
    }
}
