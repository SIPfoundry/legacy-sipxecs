/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.IntegrationTestCase;

import java.util.List;

public class SipxServiceTestIntegration extends IntegrationTestCase {

    private SipxStatusService m_statusService;

    public void testGetConfigurations() {
        List<SipxServiceConfiguration> configurations = m_statusService.getConfigurations();
        assertEquals(2, configurations.size());
    }

    public void setSipxStatusService(SipxStatusService sipxStatusService) {
        m_statusService = sipxStatusService;
    }
}
