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

import javax.servlet.http.HttpServletResponse;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.PageNotFoundException;
import org.apache.tapestry.error.ExceptionPresenterImpl;
import org.apache.tapestry.web.WebResponse;

/**
 * Translate specific exception to standard http response codes
 */
public class ExceptionPresenterWithHttpCodes extends ExceptionPresenterImpl {
    private static final Log LOG = LogFactory.getLog(ExceptionPresenterWithHttpCodes.class);
    private WebResponse m_response;

    public ExceptionPresenterWithHttpCodes(WebResponse response) {
        m_response = response;
    }

    @Override
    public void presentException(IRequestCycle cycle, Throwable t) {
        if (t instanceof PageNotFoundException) {
            try {
                // setStatus seems to have no effect so I'm calling sendError
                m_response.sendError(HttpServletResponse.SC_NOT_FOUND, t.getMessage());
            } catch (IOException e) {
                LOG.error(e);
            }
        }
        super.presentException(cycle, t);
    }
}
