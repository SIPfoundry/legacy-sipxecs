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
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.openacd.OpenAcdClient;
import org.sipfoundry.sipxconfig.openacd.OpenAcdContext;

public abstract class EditOpenAcdClientPage extends PageWithCallback implements PageBeginRenderListener {
    public static final String PAGE = "openacd/EditOpenAcdClientPage";

    @InjectObject("spring:openAcdContext")
    public abstract OpenAcdContext getOpenAcdContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract Integer getOpenAcdClientId();

    public abstract void setOpenAcdClientId(Integer id);

    public abstract OpenAcdClient getOpenAcdClient();

    public abstract void setOpenAcdClient(OpenAcdClient client);

    public abstract int getIndex();

    public abstract void setIndex(int i);

    public void addClient(String returnPage) {
        setOpenAcdClientId(null);
        setReturnPage(returnPage);
    }

    public void editClient(Integer clientId, String returnPage) {
        setOpenAcdClientId(clientId);
        setReturnPage(returnPage);
    }

    @Override
    public void pageBeginRender(PageEvent event) {
        if (getOpenAcdClientId() != null) {
            setOpenAcdClient(getOpenAcdContext().getClientById(getOpenAcdClientId()));
        } else {
            setOpenAcdClient(new OpenAcdClient());
        }
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        OpenAcdClient client = getOpenAcdClient();
        getOpenAcdContext().saveClient(client);
        setOpenAcdClientId(client.getId());
    }
}
