/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Arrays;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.phone.TestPhone;

import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.same;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.createStrictMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.verify;

public class ProfileManagerImplTest extends TestCase {

    public void testNewPhone() {
        new TestPhone();
    }

    public void testRestartDevices() {
        Integer[] ids = {
            new Integer(1000), new Integer(2000)
        };

        RestartManager restartManager = createMock(RestartManager.class);
        restartManager.restart(ids[0], null);
        restartManager.restart(ids[1], null);

        replay(restartManager);

        ProfileManagerImpl pm = new ProfileManagerImpl();
        pm.setRestartManager(restartManager);

        pm.restartDevices(Arrays.asList(ids), null);

        verify(restartManager);
    }

    public void testGenerateProfilesAndRestart() {
        Integer jobId = new Integer(4);

        Integer[] ids = {
            new Integer(1000), new Integer(2000)
        };

        JobContext jobContext = createStrictMock(JobContext.class);
        jobContext.schedule("Projection for: 110000000000");
        expectLastCall().andReturn(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);
        jobContext.schedule("Projection for: 120000000000");
        expectLastCall().andReturn(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);

        MemoryProfileLocation location = new MemoryProfileLocation();
        DeviceDescriptor model = new DeviceDescriptor() {
        };
        model.setDefaultProfileLocation(location);

        Device phone = createStrictMock(Device.class);
        phone.getModel();
        expectLastCall().andReturn(model);
        phone.getNiceName();
        expectLastCall().andReturn("110000000000");
        phone.getProfileLocation();
        expectLastCall().andReturn(location);
        phone.generateProfiles(same(location));
        phone.getModel();
        expectLastCall().andReturn(model);
        phone.getNiceName();
        expectLastCall().andReturn("120000000000");
        phone.getProfileLocation();
        expectLastCall().andReturn(location);
        phone.generateProfiles(same(location));

        DeviceSource source = createMock(DeviceSource.class);
        expect(source.loadDevice(ids[0])).andReturn(phone);
        expect(source.loadDevice(ids[1])).andReturn(phone);

        RestartManager restartManager = createMock(RestartManager.class);
        restartManager.restart(ids[0], null);
        restartManager.restart(ids[1], null);

        replay(jobContext, phone, source, restartManager);

        ProfileManagerImpl pm = new ProfileManagerImpl();
        pm.setJobContext(jobContext);
        pm.setDeviceSource(source);
        pm.setRestartManager(restartManager);

        pm.generateProfiles(Arrays.asList(ids), true, null);

        verify(jobContext, phone, source, restartManager);
    }

    public void testGenerateProfileAndRestart() {
        Integer jobId = new Integer(4);
        Integer phoneId = new Integer(1000);

        JobContext jobContext = createStrictMock(JobContext.class);
        jobContext.schedule("Projection for: 110000000000");
        expectLastCall().andReturn(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);

        MemoryProfileLocation location = new MemoryProfileLocation();
        DeviceDescriptor model = new DeviceDescriptor() {
        };
        model.setDefaultProfileLocation(location);

        Device phone = createStrictMock(Device.class);
        phone.getModel();
        expectLastCall().andReturn(model);
        phone.getNiceName();
        expectLastCall().andReturn("110000000000");
        phone.getProfileLocation();
        expectLastCall().andReturn(location);
        phone.generateProfiles(same(location));

        DeviceSource source = createMock(DeviceSource.class);
        expect(source.loadDevice(phoneId)).andReturn(phone);

        RestartManager restartManager = createMock(RestartManager.class);
        restartManager.restart(phoneId, null);

        replay(jobContext, phone, source, restartManager);

        ProfileManagerImpl pm = new ProfileManagerImpl();
        pm.setJobContext(jobContext);
        pm.setDeviceSource(source);
        pm.setRestartManager(restartManager);

        pm.generateProfile(phoneId, true, null);

        verify(jobContext, phone, source, restartManager);
    }

    public void testGenerateProfile() {
        Integer jobId = new Integer(4);
        Integer phoneId = new Integer(1000);

        JobContext jobContext = createStrictMock(JobContext.class);
        jobContext.schedule("Projection for: 110000000000");
        expectLastCall().andReturn(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);

        MemoryProfileLocation location = new MemoryProfileLocation();
        DeviceDescriptor model = new DeviceDescriptor() {
        };
        model.setDefaultProfileLocation(location);

        Device phone = createMock(Device.class);
        phone.getModel();
        expectLastCall().andReturn(model);
        phone.getNiceName();
        expectLastCall().andReturn("110000000000");
        phone.getProfileLocation();
        expectLastCall().andReturn(location);
        phone.generateProfiles(same(location));

        DeviceSource source = createMock(DeviceSource.class);
        expect(source.loadDevice(phoneId)).andReturn(phone);

        RestartManager restartManager = createMock(RestartManager.class);

        replay(jobContext, phone, source, restartManager);

        ProfileManagerImpl pm = new ProfileManagerImpl();
        pm.setJobContext(jobContext);
        pm.setDeviceSource(source);
        pm.setRestartManager(restartManager);

        pm.generateProfile(phoneId, false, null);

        verify(jobContext, phone, source, restartManager);
    }

}
