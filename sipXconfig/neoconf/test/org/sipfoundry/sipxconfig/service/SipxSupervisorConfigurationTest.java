/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.util.ArrayList;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.LoggingManager;

public class SipxSupervisorConfigurationTest extends SipxServiceTestBase {
    @Override
    protected void setUp() throws Exception {

    }

    public void testGenerateSipxSupervisor() throws Exception {
        SipxSupervisorConfiguration supervisorConf = new SipxSupervisorConfiguration();
        SipxSupervisorService supervisor = new SipxSupervisorService ();

        SipxServiceManager serviceManager = createMock(SipxServiceManager.class);
        serviceManager.getServiceByBeanId(SipxSupervisorService.BEAN_ID);
        expectLastCall().andReturn(supervisor).atLeastOnce();
        serviceManager.storeService(supervisor);
        replay(serviceManager);

        supervisor.setSipxServiceManager(serviceManager);

        LoggingManager loggingManagerMock = createMock(LoggingManager.class);
        loggingManagerMock.getEntitiesToProcess();
        expectLastCall().andReturn(new ArrayList<LoggingEntity>()).atLeastOnce();
        replay(loggingManagerMock);
        supervisor.setLoggingManager(loggingManagerMock);

        supervisor.setLogLevel("DEBUG");

        supervisorConf.setSipxServiceManager(serviceManager);
        supervisorConf.setVelocityEngine(TestHelper.getVelocityEngine());
        supervisorConf.setTemplate("commserver/sipxsupervisor-config.vm");

        assertCorrectFileGeneration(supervisorConf, "expected-sipxsupervisor-config");
    }
}