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
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.RestartManager;
import org.sipfoundry.sipxconfig.job.JobContext;

public class PhoneProfileManagerImplTest extends TestCase {

    public void testNewPhone() {
        new TestPhone();
    }

    public void testGenerateProfilesAndRestart() {
        Integer jobId = new Integer(4);

        Integer[] ids = {
            new Integer(1000), new Integer(2000)
        };

        IMocksControl jobContextCtrl = EasyMock.createStrictControl();
        JobContext jobContext = jobContextCtrl.createMock(JobContext.class);
        jobContext.schedule("Projection for: 110000000000");
        jobContextCtrl.andReturn(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);
        jobContext.schedule("Projection for: 120000000000");
        jobContextCtrl.andReturn(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);
        jobContextCtrl.replay();

        MemoryProfileLocation location = new MemoryProfileLocation();
        PhoneModel model = new PhoneModel();
        model.setDefaultProfileLocation(location);

        IMocksControl phoneControl = org.easymock.classextension.EasyMock.createStrictControl();
        Phone phone = phoneControl.createMock(Phone.class);
        phone.getSerialNumber();
        phoneControl.andReturn("110000000000");
        phone.getModel();
        phoneControl.andReturn(model);
        phone.generateProfiles(EasyMock.same(location));
        phone.getSerialNumber();
        phoneControl.andReturn("120000000000");
        phone.getModel();
        phoneControl.andReturn(model);
        phone.generateProfiles(EasyMock.same(location));
        phoneControl.replay();

        IMocksControl phoneContextCtrl = EasyMock.createControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);
        phoneContext.loadPhone(ids[0]);
        phoneContextCtrl.andReturn(phone);
        phoneContext.loadPhone(ids[1]);
        phoneContextCtrl.andReturn(phone);
        phoneContextCtrl.replay();

        IMocksControl restartManagerCtrl = EasyMock.createControl();
        RestartManager restartManager = restartManagerCtrl.createMock(RestartManager.class);
        restartManager.restart(ids[0]);
        restartManager.restart(ids[1]);
        restartManagerCtrl.replay();

        PhoneProfileManagerImpl pm = new PhoneProfileManagerImpl();
        pm.setJobContext(jobContext);
        pm.setPhoneContext(phoneContext);
        pm.setRestartManager(restartManager);

        pm.generateProfiles(Arrays.asList(ids), true);

        jobContextCtrl.verify();
        phoneControl.verify();
        phoneContextCtrl.verify();
        restartManagerCtrl.verify();
    }

    public void testGenerateProfileAndRestart() {
        Integer jobId = new Integer(4);
        Integer phoneId = new Integer(1000);

        IMocksControl jobContextCtrl = EasyMock.createStrictControl();
        JobContext jobContext = jobContextCtrl.createMock(JobContext.class);
        jobContext.schedule("Projection for: 110000000000");
        jobContextCtrl.andReturn(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);
        jobContextCtrl.replay();

        MemoryProfileLocation location = new MemoryProfileLocation();
        PhoneModel model = new PhoneModel();
        model.setDefaultProfileLocation(location);

        IMocksControl phoneControl = org.easymock.classextension.EasyMock.createStrictControl();
        Phone phone = phoneControl.createMock(Phone.class);
        phone.getSerialNumber();
        phoneControl.andReturn("110000000000");
        phone.getModel();
        phoneControl.andReturn(model);
        phone.generateProfiles(EasyMock.same(location));
        phoneControl.replay();

        IMocksControl phoneContextCtrl = EasyMock.createControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);
        phoneContext.loadPhone(phoneId);
        phoneContextCtrl.andReturn(phone);
        phoneContextCtrl.replay();

        IMocksControl restartManagerCtrl = EasyMock.createControl();
        RestartManager restartManager = restartManagerCtrl.createMock(RestartManager.class);
        restartManager.restart(phoneId);
        restartManagerCtrl.replay();

        PhoneProfileManagerImpl pm = new PhoneProfileManagerImpl();
        pm.setJobContext(jobContext);
        pm.setPhoneContext(phoneContext);
        pm.setRestartManager(restartManager);

        pm.generateProfile(phoneId, true);

        jobContextCtrl.verify();
        phoneControl.verify();
        phoneContextCtrl.verify();
        restartManagerCtrl.verify();
    }

    public void testGenerateProfile() {
        Integer jobId = new Integer(4);
        Integer phoneId = new Integer(1000);

        IMocksControl jobContextCtrl = EasyMock.createStrictControl();
        JobContext jobContext = jobContextCtrl.createMock(JobContext.class);
        jobContext.schedule("Projection for: 110000000000");
        jobContextCtrl.andReturn(jobId);
        jobContext.start(jobId);
        jobContext.success(jobId);
        jobContextCtrl.replay();

        MemoryProfileLocation location = new MemoryProfileLocation();
        PhoneModel model = new PhoneModel();
        model.setDefaultProfileLocation(location);

        IMocksControl phoneControl = org.easymock.classextension.EasyMock.createStrictControl();
        Phone phone = phoneControl.createMock(Phone.class);
        phone.getSerialNumber();
        phoneControl.andReturn("110000000000");
        phone.getModel();
        phoneControl.andReturn(model);
        phone.generateProfiles(EasyMock.same(location));
        phoneControl.replay();

        IMocksControl phoneContextCtrl = EasyMock.createControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);
        phoneContext.loadPhone(phoneId);
        phoneContextCtrl.andReturn(phone);
        phoneContextCtrl.replay();

        IMocksControl restartManagerCtrl = EasyMock.createControl();
        RestartManager restartManager = restartManagerCtrl.createMock(RestartManager.class);
        restartManagerCtrl.replay();

        PhoneProfileManagerImpl pm = new PhoneProfileManagerImpl();
        pm.setJobContext(jobContext);
        pm.setPhoneContext(phoneContext);
        pm.setRestartManager(restartManager);

        pm.generateProfile(phoneId, false);

        jobContextCtrl.verify();
        phoneControl.verify();
        phoneContextCtrl.verify();
        restartManagerCtrl.verify();
    }

}
