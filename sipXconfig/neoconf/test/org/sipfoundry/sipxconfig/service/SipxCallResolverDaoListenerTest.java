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

public class SipxCallResolverDaoListenerTest extends TestCase {

    public void testOnSaveLocation() {
        SipxCallResolverDaoListener out = new SipxCallResolverDaoListener();

        Location location = new Location();
        SipxCallResolverService sipxCallResolverService = new SipxCallResolverService();

        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxCallResolverService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(sipxCallResolverService);
        EasyMock.replay(sipxServiceManager);

        ServiceConfigurator serviceConfigurator = EasyMock.createMock(ServiceConfigurator.class);
        serviceConfigurator.replicateServiceConfig(sipxCallResolverService);
        EasyMock.expectLastCall();
        EasyMock.replay(serviceConfigurator);

        out.setSipxServiceManager(sipxServiceManager);
        out.setServiceConfigurator(serviceConfigurator);
        out.onSave(location);
        EasyMock.verify(sipxServiceManager, serviceConfigurator);
    }

    public void testOnSaveNonLocation() {
        // no action expected
        SipxCallResolverDaoListener out = new SipxCallResolverDaoListener();
        out.onSave(new SipxProxyService());
    }
}
