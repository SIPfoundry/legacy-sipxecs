/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.TestHelper;

public class StatusPluginConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxStatusService statusService = new SipxStatusService();
        initCommonAttributes(statusService);
        statusService.setHttpPort(8100);
        statusService.setHttpsPort(8101);
        statusService.setStatusServerSipPort(5110);

        StatusPluginConfiguration out = new StatusPluginConfiguration();
        out.setVelocityEngine(TestHelper.getVelocityEngine());
        out.setTemplate("sipxstatus/status-plugin.vm");
        out.generate(statusService);

        assertCorrectFileGeneration(out, "expected-status-plugin-config");
    }
}
