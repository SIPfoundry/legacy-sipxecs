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

import org.apache.tapestry.engine.DirectService;

public class ExportService extends DirectService {

    public static final String SERVICE_NAME = "export";

    public String getName() {
        return SERVICE_NAME;
    }
}
