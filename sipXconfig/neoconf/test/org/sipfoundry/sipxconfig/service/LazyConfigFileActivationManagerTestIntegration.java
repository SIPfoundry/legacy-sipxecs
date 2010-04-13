/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import junit.framework.TestCase;
import static java.lang.Thread.sleep;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

// marked integration due to time sensitivity
public class LazyConfigFileActivationManagerTestIntegration extends TestCase {

    public void testLazy() throws Exception {
        SipxServiceManager manager = createMock(SipxServiceManager.class);
        manager.getServiceByBeanId("serviceBeanId");
        expectLastCall().andReturn(null).times(2);

        replay(manager);

        LazyConfigFileActivationManager activationMgr = new MockConfigFileManager();
        activationMgr.setSleepInterval(100);
        activationMgr.setServiceBeanId("serviceBeanId");
        activationMgr.setSipxServiceManager(manager);
        activationMgr.init();

        activationMgr.activateConfigFiles();
        activationMgr.activateConfigFiles();
        sleep(300);

        activationMgr.activateConfigFiles();
        activationMgr.activateConfigFiles();
        sleep(300);

        verify(manager);

    }

    static class MockConfigFileManager extends LazyConfigFileActivationManager {

        public MockConfigFileManager() {
        }

        @Override
        public ServiceConfigurator getServiceConfigurator() {
            return null;
        }
    }

}
