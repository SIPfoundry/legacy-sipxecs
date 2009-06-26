/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site;

import java.io.IOException;

import org.apache.log4j.MDC;
import org.apache.tapestry.engine.state.ApplicationStateManager;
import org.apache.tapestry.services.WebRequestServicer;
import org.apache.tapestry.services.WebRequestServicerFilter;
import org.apache.tapestry.web.WebRequest;
import org.apache.tapestry.web.WebResponse;
import org.sipfoundry.sipxconfig.common.CoreContext;

public class DiagnosticContextFilter implements WebRequestServicerFilter {

    private ApplicationStateManager m_applicationStateManager;
    private CoreContext m_coreContext;

    @Override
    public void service(WebRequest request, WebResponse response, WebRequestServicer servicer) throws IOException {
        UserSession userSession = (UserSession) m_applicationStateManager.get("userSession");
        if (userSession.getUserId() != null) {
            MDC.put("username", userSession.getUser(m_coreContext).getName());
        }

        servicer.service(request, response);
    }

    public void setApplicationStateManager(ApplicationStateManager applicationStateManager) {
        m_applicationStateManager = applicationStateManager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
