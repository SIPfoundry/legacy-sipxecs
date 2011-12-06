/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import org.sipfoundry.sipxconfig.service.SipxBridgeService;
import org.sipfoundry.sipxconfig.service.SipxConfigService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;

import junit.framework.TestCase;

public class RestartNeededServiceTest extends TestCase {
    public void testRestartNeeded() {
        RestartNeededService service1 = new RestartNeededService("location1", SipxConfigService.BEAN_ID);
        assertTrue(service1.isConfigurationRestartNeeded());
        RestartNeededService service2 = new RestartNeededService("location1", SipxBridgeService.BEAN_ID);
        assertFalse(service2.isConfigurationRestartNeeded());
        RestartNeededService service3 = new RestartNeededService("location1", SipxProxyService.BEAN_ID);
        assertFalse(service3.isConfigurationRestartNeeded());
    }
}
