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

import junit.framework.TestCase;

import org.easymock.classextension.EasyMock;
import org.easymock.classextension.IMocksControl;
import org.sipfoundry.sipxconfig.device.ModelSource;

public class AddServiceTaskTest extends TestCase {

    public void testAddService() {
        ServiceDescriptor turkeyDesctriptor = new ServiceDescriptor("bird", "turkey");
        IMocksControl serviceSourceControl = EasyMock.createControl();
        ModelSource<ServiceDescriptor> serviceSource = serviceSourceControl.createMock(ModelSource.class);
        serviceSource.getModel("turkey");
        serviceSourceControl.andReturn(turkeyDesctriptor).anyTimes();
        serviceSourceControl.replay();

        ConfiguredService turkey = new UnmanagedService();
        turkey.setDescriptor(turkeyDesctriptor);

        IMocksControl serviceManagerControl = EasyMock.createStrictControl();
        ServiceManager serviceManager = serviceManagerControl.createMock(ServiceManager.class);


        // already a service
        serviceManager.getServiceByName("Thanksgiving");
        serviceManagerControl.andReturn(turkey);
        serviceManagerControl.replay();
        AddServiceTask task = new AddServiceTask();
        task.setServiceManager(serviceManager);
        task.setServicesSource(serviceSource);
        try {
            task.addService("Thanksgiving", "chicken", "1.1.1.1");
            fail();
        } catch (IllegalArgumentException expected) {
        }
        serviceManagerControl.verify();


        // update service
        serviceManagerControl.reset();
        serviceManager.getServiceByName("Thanksgiving");
        serviceManagerControl.andReturn(turkey);
        serviceManager.saveService(turkey);
        serviceManagerControl.replay();
        turkey.setName("Thanksgiving");
        task.addService("Thanksgiving", "turkey", "1.1.1.1");
        serviceManagerControl.verify();

        // add a service
        serviceManagerControl.reset();
        serviceManager.getServiceByName("Thanksgiving");
        serviceManagerControl.andReturn(null);
        serviceManager.newService(turkeyDesctriptor);
        serviceManagerControl.andReturn(turkey);
        serviceManager.saveService(turkey);
        serviceManagerControl.replay();
        turkey.setName("Thanksgiving");
        task.addService("Thanksgiving", "turkey", "1.1.1.1");
        serviceManagerControl.verify();
    }

    public void testAssertArgument() {
        AddServiceTask.assertArgument(new String[] {"x"}, 0, "x");
        try {
            AddServiceTask.assertArgument(new String[0], 0, "x");
            fail();
        } catch (IllegalArgumentException expected) {
            assertTrue(true);
        }
    }
}
