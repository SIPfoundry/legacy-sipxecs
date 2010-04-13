/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components.service;

import java.io.IOException;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.services.ResponseRenderer;

public class EmptyResponseRenderer implements ResponseRenderer {

    public void renderResponse(IRequestCycle cycle) throws IOException {
        // do nothing - it is used by services that pre-render response in listeners
    }

}
