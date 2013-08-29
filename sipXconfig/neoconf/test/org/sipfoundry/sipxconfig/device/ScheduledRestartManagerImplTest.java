/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.device;

import static org.easymock.EasyMock.expect;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.createStrictMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.verify;

import java.util.Date;
import java.util.concurrent.ScheduledExecutorService;

import junit.framework.TestCase;

import org.junit.Assert;
import org.sipfoundry.sipxconfig.job.JobContextImpl;
import org.sipfoundry.sipxconfig.test.MemoryProfileLocation;

public class ScheduledRestartManagerImplTest extends TestCase {

    public void testRestartSupported() {
        ScheduledRestartManagerImpl restartManager = new ScheduledRestartManagerImpl();
        Integer deviceId = new Integer(1000);
        MemoryProfileLocation location = new MemoryProfileLocation();
        DeviceDescriptor model = new DeviceDescriptor() {};
        model.setRestartSupported(true);
        model.setDefaultProfileLocation(location);
        JobContextImpl jobContext = new JobContextImpl();
        jobContext.init();        
        Device device = createStrictMock(Device.class);
        DeviceSource source = createMock(DeviceSource.class);
        ScheduledExecutorService scheduler = createMock(ScheduledExecutorService.class);

        expect(device.getModel()).andReturn(model);
        expect(source.loadDevice(deviceId)).andReturn(device);

        replay(scheduler, device, source);

        restartManager.setJobContext(jobContext);
        restartManager.setExecutorService(scheduler);
        restartManager.setDeviceSource(source);
        //TEST restart is supported
        //restart call should fail because scheduler.schedule method
        //that launches restart action is not mapped for ScheduledExecutorService
        try {
            restartManager.restart(deviceId, new Date());
            Assert.fail("Did not fail on restart");
        } catch (AssertionError ex) {
            Assert.assertNotNull(ex.getMessage());
        }

        // XX-10757 - easymockclassextensions, JDK 1.7 and method class in contructors 
        // don't work well together.  Upgrade from 2.2 to easymock 3.2 to see if it helps
        if (!System.getProperty("java.version").startsWith("1.7")) {
            verify(device, source);
        }
    }

    public void testRestartNotSupported() {
        ScheduledRestartManagerImpl restartManager = new ScheduledRestartManagerImpl();
        Integer deviceId = new Integer(1000);
        MemoryProfileLocation location = new MemoryProfileLocation();
        DeviceDescriptor model = new DeviceDescriptor() {};
        model.setDefaultProfileLocation(location);
        JobContextImpl jobContext = new JobContextImpl();
        jobContext.init();
        Device device = createStrictMock(Device.class);
        DeviceSource source = createMock(DeviceSource.class);
        ScheduledExecutorService scheduler = createMock(ScheduledExecutorService.class);

        expect(device.getModel()).andReturn(model);
        expect(source.loadDevice(deviceId)).andReturn(device);

        replay(scheduler, device, source);

        restartManager.setJobContext(jobContext);
        restartManager.setExecutorService(scheduler);
        restartManager.setDeviceSource(source);

        // XX-10757 - easymockclassextensions, JDK 1.7 and method class in contructors 
        // don't work well together.  Upgrade from 2.2 to easymock 3.2 to see if it helps
        if (!System.getProperty("java.version").startsWith("1.7")) {
            //TEST restart is not supported
            //restart call should not fail because no restart action is scheduled
            //(schedule method is not mapped for ScheduledExecutorService)
            restartManager.restart(deviceId, new Date());

            verify(device, source);
        }
    }
}
