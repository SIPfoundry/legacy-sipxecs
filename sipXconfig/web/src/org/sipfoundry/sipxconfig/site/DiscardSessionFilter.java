/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

import java.io.IOException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.services.WebRequestServicer;
import org.apache.tapestry.services.WebRequestServicerFilter;
import org.apache.tapestry.web.WebRequest;
import org.apache.tapestry.web.WebResponse;
import org.apache.tapestry.web.WebSession;

/**
 * Logout copied from Vlib example, also see
 * http://thread.gmane.org/gmane.comp.java.tapestry.user/31641
 *
 * Filter, injected into the tapestry.request.WebRequestServicerPipeline configuration, that
 * optionally discards the session at the end of the request (after a logout, typically).
 */
public class DiscardSessionFilter implements WebRequestServicerFilter {
    private static final Log LOG = LogFactory.getLog(DiscardSessionFilter.class);
    private ApplicationLifecycle m_applicationLifecycle;

    public void service(WebRequest request, WebResponse response, WebRequestServicer servicer)
        throws IOException {
        try {
            servicer.service(request, response);
        } finally {
            if (m_applicationLifecycle.getDiscardSession()) {
                discardSession(request);
            }
        }
    }

    private void discardSession(WebRequest request) {
        WebSession session = request.getSession(false);

        if (session != null) {
            try {
                session.invalidate();
            } catch (IllegalStateException e) {
                LOG.warn("benign error invalidating session", e);
            }
        }
    }

    public void setApplicationLifecycle(ApplicationLifecycle applicationLifecycle) {
        m_applicationLifecycle = applicationLifecycle;
    }
}
