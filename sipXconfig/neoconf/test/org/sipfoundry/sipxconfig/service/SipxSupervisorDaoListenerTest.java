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

import org.easymock.EasyMock;
import org.junit.Test;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class SipxSupervisorDaoListenerTest {

    @Test
    public void testOnSaveLocation() {
        SipxSupervisorDaoListener out = new SipxSupervisorDaoListener();
        
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
    
    @Test
    public void testOnSaveNonLocation() {
        // no action expected
        SipxSupervisorDaoListener out = new SipxSupervisorDaoListener();
        out.onSave(new SipxProxyService());
    }
}
