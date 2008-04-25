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

import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.createStrictMock;
import static org.easymock.classextension.EasyMock.replay;
import static org.easymock.classextension.EasyMock.verify;

public class RestartManagerImplTest extends TestCase {

    public void testGenerateProfiles() throws Exception {
        Integer jobId = new Integer(4);
        Integer phoneId = new Integer(1000);

        JobContext jobContext = createStrictMock(JobContext.class);
        jobContext.schedule("Restarting: 000000000000");
        expectLastCall().andReturn(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);

        Device phone = createStrictMock(Device.class);
        phone.getNiceName();
        expectLastCall().andReturn("000000000000");
        phone.restart();

        DeviceSource source = createMock(DeviceSource.class);
        expect(source.loadDevice(phoneId)).andReturn(phone);

        replay(jobContext, phone, source);

        RestartManagerImpl rm = new RestartManagerImpl();
        rm.setJobContext(jobContext);
        rm.setDeviceSource(source);

        rm.restart(phoneId);

        verify(jobContext, phone, source);
    }

    public void testRestartException() throws Exception {
        Integer jobId = new Integer(4);
        Integer phoneId = new Integer(1000);

        RestartException re = new RestartException("xxx");

        JobContext jobContext = createStrictMock(JobContext.class);
        jobContext.schedule("Restarting: 000000000000");
        expectLastCall().andReturn(jobId);
        jobContext.start(jobId);
        jobContext.failure(jobId, null, re);

        Device phone = createStrictMock(Device.class);
        phone.getNiceName();
        expectLastCall().andReturn("000000000000");
        phone.restart();
        expectLastCall().andThrow(re);

        DeviceSource source = createMock(DeviceSource.class);
        expect(source.loadDevice(phoneId)).andReturn(phone);

        replay(jobContext, phone, source);

        RestartManagerImpl rm = new RestartManagerImpl();
        rm.setJobContext(jobContext);
        rm.setDeviceSource(source);

        rm.restart(phoneId);

        verify(jobContext, phone, source);
    }

    public void testThrottle() {
        Integer jobId = new Integer(4);

        Integer[] ids = {
            new Integer(1000), new Integer(2000)
        };

        JobContext jobContext = createStrictMock(JobContext.class);
        for (int i = 0; i < 2; i++) {
            jobContext.schedule("Restarting: 110000000000");
            expectLastCall().andReturn(jobId);
            jobContext.start(jobId);
            jobContext.success(jobId);
            jobContext.schedule("Restarting: 120000000000");
            expectLastCall().andReturn(jobId);
            jobContext.start(jobId);
            jobContext.success(jobId);
        }

        Device phone = createStrictMock(Device.class);
        for (int i = 0; i < 2; i++) {
            phone.getNiceName();
            expectLastCall().andReturn("110000000000");
            phone.restart();
            phone.getNiceName();
            expectLastCall().andReturn("120000000000");
            phone.restart();
        }

        DeviceSource phoneSource = createMock(DeviceSource.class);
        for (int i = 0; i < 2; i++) {
            phoneSource.loadDevice(ids[0]);
            expectLastCall().andReturn(phone);
            phoneSource.loadDevice(ids[1]);
            expectLastCall().andReturn(phone);
        }

        replay(jobContext, phone, phoneSource);

        final int throttle = 50;

        RestartManagerImpl rm = new RestartManagerImpl();
        rm.setJobContext(jobContext);
        rm.setDeviceSource(phoneSource);

        rm.setThrottleInterval(0);

        long before = System.currentTimeMillis();
        rm.restart(Arrays.asList(ids));
        long duration = System.currentTimeMillis() - before;
        assertTrue(duration < 2 * throttle);

        rm.setThrottleInterval(throttle);

        before = System.currentTimeMillis();
        rm.restart(Arrays.asList(ids));
        duration = System.currentTimeMillis() - before;

        // failed 2 out of 10 times on windows until fudge added
        // machine was 3GHz, Dual core p4.
        final long timingFudge = 100;
        assertTrue(duration + timingFudge >= 2 * throttle);

        verify(jobContext, phone, phoneSource);
    }
}
