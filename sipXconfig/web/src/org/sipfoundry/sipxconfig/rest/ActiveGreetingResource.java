/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.rest;

import static org.restlet.data.MediaType.TEXT_PLAIN;

import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Representation;
import org.restlet.resource.Resource;
import org.restlet.resource.ResourceException;
import org.restlet.resource.Variant;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.ActiveGreeting;

public class ActiveGreetingResource extends Resource {
    private CoreContext m_coreContext;
    private String m_userName;

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        getVariants().add(new Variant(TEXT_PLAIN));
        m_userName = (String) getRequest().getAttributes().get("user");

    }

    public void storeRepresentation(Representation entity) throws ResourceException {
        String greeting = (String) getRequest().getAttributes().get("greeting");
        User user = m_coreContext.loadUserByUserName(m_userName);
        // we need the id here. we also have to make sure the id is recognized and not a random
        // string
        user.setSettingValue(MailboxPreferences.ACTIVE_GREETING, ActiveGreeting.fromId(greeting).getId());
        m_coreContext.saveUser(user);
    }

    public CoreContext getCoreContext() {
        return m_coreContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
