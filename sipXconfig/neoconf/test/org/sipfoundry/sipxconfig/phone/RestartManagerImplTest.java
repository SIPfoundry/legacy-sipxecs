/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.util.Arrays;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.job.JobContext;

public class RestartManagerImplTest extends TestCase {
    
    public void testGenerateProfiles() throws Exception {
        Integer jobId = new Integer(4);
        Integer phoneId = new Integer(1000);

        IMocksControl jobContextCtrl = EasyMock.createStrictControl();
        JobContext jobContext = jobContextCtrl.createMock(JobContext.class);
        jobContext.schedule("Restarting phone 000000000000");
        jobContextCtrl.andReturn(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);
        jobContextCtrl.replay();

        IMocksControl phoneControl = org.easymock.classextension.EasyMock.createStrictControl();
        Phone phone = phoneControl.createMock(Phone.class);
        phone.getSerialNumber();
        phoneControl.andReturn("000000000000");
        phone.restart();
        phoneControl.replay();

        IMocksControl phoneContextCtrl = EasyMock.createControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);
        phoneContext.loadPhone(phoneId);
        phoneContextCtrl.andReturn(phone);
        phoneContextCtrl.replay();

        RestartManagerImpl rm = new RestartManagerImpl();
        rm.setJobContext(jobContext);
        rm.setPhoneContext(phoneContext);

        rm.restart(phoneId);

        jobContextCtrl.verify();
        phoneControl.verify();
        phoneContextCtrl.verify();
    }

    public void testRestartException() throws Exception {
        Integer jobId = new Integer(4);
        Integer phoneId = new Integer(1000);

        RestartException re = new RestartException("xxx");

        IMocksControl jobContextCtrl = EasyMock.createStrictControl();
        JobContext jobContext = jobContextCtrl.createMock(JobContext.class);
        jobContext.schedule("Restarting phone 000000000000");
        jobContextCtrl.andReturn(jobId);
        jobContext.start(jobId);
        jobContext.failure(jobId, null, re);
        jobContextCtrl.replay();

        IMocksControl phoneControl = org.easymock.classextension.EasyMock.createStrictControl();
        Phone phone = phoneControl.createMock(Phone.class);
        phone.getSerialNumber();
        phoneControl.andReturn("000000000000");
        phone.restart();
        phoneControl.andThrow(re);
        phoneControl.replay();

        IMocksControl phoneContextCtrl = EasyMock.createControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);
        phoneContext.loadPhone(phoneId);
        phoneContextCtrl.andReturn(phone);
        phoneContextCtrl.replay();

        RestartManagerImpl rm = new RestartManagerImpl();
        rm.setJobContext(jobContext);
        rm.setPhoneContext(phoneContext);

        rm.restart(phoneId);

        phoneControl.verify();
        jobContextCtrl.verify();
    }

    public void testThrottle() {
        Integer jobId = new Integer(4);

        Integer[] ids = {
            new Integer(1000), new Integer(2000)
        };

        IMocksControl jobContextCtrl = EasyMock.createStrictControl();
        JobContext jobContext = jobContextCtrl.createMock(JobContext.class);
        for (int i = 0; i < 2; i++) {
            jobContext.schedule("Restarting phone 110000000000");
            jobContextCtrl.andReturn(jobId);
            jobContext.start(jobId);
            jobContext.success(jobId);
            jobContext.schedule("Restarting phone 120000000000");
            jobContextCtrl.andReturn(jobId);
            jobContext.start(jobId);
            jobContext.success(jobId);
        }
        jobContextCtrl.replay();

        IMocksControl phoneControl = org.easymock.classextension.EasyMock.createStrictControl();
        Phone phone = phoneControl.createMock(Phone.class);
        for (int i = 0; i < 2; i++) {
            phone.getSerialNumber();
            phoneControl.andReturn("110000000000");
            phone.restart();
            phone.getSerialNumber();
            phoneControl.andReturn("120000000000");
            phone.restart();
        }
        phoneControl.replay();

        IMocksControl phoneContextCtrl = EasyMock.createControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);
        for (int i = 0; i < 2; i++) {
            phoneContext.loadPhone(ids[0]);
            phoneContextCtrl.andReturn(phone);
            phoneContext.loadPhone(ids[1]);
            phoneContextCtrl.andReturn(phone);
        }
        phoneContextCtrl.replay();

        final int throttle = 50;

        RestartManagerImpl rm = new RestartManagerImpl();
        rm.setJobContext(jobContext);
        rm.setPhoneContext(phoneContext);

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

        jobContextCtrl.verify();
        phoneControl.verify();
        phoneContextCtrl.verify();
    }
}
