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

import junit.framework.TestCase;
import static java.lang.Thread.sleep;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

// marked integration due to time sensitivity
public class LazyDialPlanActivationManagerTestIntegration extends TestCase {

    public void testLazy() throws Exception {
        DialPlanActivationManager target = createMock(DialPlanActivationManager.class);
        target.replicateDialPlan(true);
        expectLastCall().once();
        target.replicateDialPlan(false);
        expectLastCall().once();
        replay(target);

        LazyDialPlanActivationManager dpam = new LazyDialPlanActivationManager();
        dpam.setTarget(target);
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

        verify(target);
    }

}
