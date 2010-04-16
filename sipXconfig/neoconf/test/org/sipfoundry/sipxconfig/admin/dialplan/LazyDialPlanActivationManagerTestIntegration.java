/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;
import static java.lang.Thread.sleep;

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.device.ProfileManager;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

// marked integration due to time sensitivity
public class LazyDialPlanActivationManagerTestIntegration extends TestCase {

    public void testLazy() throws Exception {
        SipxProcessContext processContext = createMock(SipxProcessContext.class);
        processContext.markDialPlanRelatedServicesForRestart(SipxProxyService.BEAN_ID, SipxRegistrarService.BEAN_ID);
        expectLastCall().times(7);

        DialPlanActivationManager target = createMock(DialPlanActivationManager.class);
        target.replicateDialPlan(true);
        expectLastCall().once();
        target.replicateDialPlan(false);
        expectLastCall().once();

        List<Integer> ids = new ArrayList<Integer>();

        SbcDeviceManager sbcDeviceManager = createMock(SbcDeviceManager.class);
        sbcDeviceManager.getAllSbcDeviceIds();
        expectLastCall().andReturn(ids);

        ProfileManager profileManager = createMock(ProfileManager.class);
        profileManager.restartDevices(ids, null);
        expectLastCall().once();

        replay(target, processContext, sbcDeviceManager, profileManager);

        LazyDialPlanActivationManager dpam = new LazyDialPlanActivationManager();
        dpam.setTarget(target);
        dpam.setSipxProcessContext(processContext);
        dpam.setSbcDeviceManager(sbcDeviceManager);
        dpam.setSbcProfileManager(profileManager);
        dpam.setSleepInterval(100);
        dpam.init();

        dpam.replicateDialPlan(false);
        dpam.replicateDialPlan(true);
        dpam.replicateDialPlan(false);
        sleep(300);

        dpam.replicateDialPlan(false);
        dpam.replicateDialPlan(false);
        dpam.replicateDialPlan(false);
        sleep(300);

        dpam.replicateDialPlan(true);
        dpam.onApplicationEvent(new DialPlanActivatedEvent(dpam, false));
        sleep(300);
        verify(target, processContext, sbcDeviceManager, profileManager);
    }

}
