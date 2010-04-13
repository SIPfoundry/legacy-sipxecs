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

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class LocationsDaoListenerTest extends TestCase {

    public void testOnSaveLocation() {
        LocationsDaoListener out = new LocationsDaoListener();

        Location location = new Location();
        SipxSupervisorService sipxSupervisorService = new SipxSupervisorService();

        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxSupervisorService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(sipxSupervisorService);
        EasyMock.replay(sipxServiceManager);

        ServiceConfigurator serviceConfigurator = EasyMock.createMock(ServiceConfigurator.class);
        serviceConfigurator.replicateServiceConfig(location, sipxSupervisorService);
        EasyMock.expectLastCall();
        EasyMock.replay(serviceConfigurator);

        out.setSipxServiceManager(sipxServiceManager);
        out.setServiceConfigurator(serviceConfigurator);
        out.onSave(location);
        EasyMock.verify(sipxServiceManager, serviceConfigurator);
    }

    public void testOnDeleteLocation() {
        LocationsDaoListener out = new LocationsDaoListener();

        Location location = new Location();

        ServiceConfigurator serviceConfigurator = EasyMock.createMock(ServiceConfigurator.class);
        serviceConfigurator.replicateAllServiceConfig();
        EasyMock.expectLastCall();
        EasyMock.replay(serviceConfigurator);

        out.setServiceConfigurator(serviceConfigurator);
        out.onDelete(location);
        EasyMock.verify(serviceConfigurator);
    }

    public void testOnSaveNonLocation() {
        // no action expected
        LocationsDaoListener out = new LocationsDaoListener();

        ServiceConfigurator serviceConfigurator = EasyMock.createMock(ServiceConfigurator.class);
        EasyMock.replay(serviceConfigurator);

        out.onSave(new SipxProxyService());

        EasyMock.verify(serviceConfigurator);
    }
}
