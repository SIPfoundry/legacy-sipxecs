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

import org.restlet.Context;
import org.restlet.data.Request;
import org.restlet.data.Response;
import org.restlet.resource.Resource;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.security.StandardUserDetailsService;
import org.sipfoundry.sipxconfig.security.UserDetailsImpl;
import org.springframework.beans.factory.annotation.Required;

/**
 * Special type of the resource accessible for an individual users
 */
public class UserResource extends Resource {

    private CoreContext m_coreContext;
    private User m_user;

    public UserResource() {
        super();
    }

    @Override
    public void init(Context context, Request request, Response response) {
        super.init(context, request, response);
        UserDetailsImpl userDetails = StandardUserDetailsService.getUserDetails();
        if (userDetails != null) {
            m_user = m_coreContext.loadUser(userDetails.getUserId());
        }
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    protected final User getUser() {
        return m_user;
    }

    protected final CoreContext getCoreContext() {
        return m_coreContext;
    }
}
