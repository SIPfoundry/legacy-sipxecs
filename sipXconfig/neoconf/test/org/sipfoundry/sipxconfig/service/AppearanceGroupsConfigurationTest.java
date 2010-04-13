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

import java.util.Arrays;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

public class AppearanceGroupsConfigurationTest extends SipxServiceTestBase {
    public void testWrite() throws Exception {
        AppearanceGroupsConfiguration out = new AppearanceGroupsConfiguration();
        out.setTemplate("sipxsaa/appearance-groups.vm");

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.getSharedUsers();

        User firstSharedUser = new User();
        firstSharedUser.setUserName("sharedline");
        User secondSharedUser = new User();
        secondSharedUser.setUserName("321");

        expectLastCall().andReturn(Arrays.asList(new User[] {
            firstSharedUser, secondSharedUser
        }));
        replay(coreContext);

        out.setCoreContext(coreContext);

        SipxSaaService saaService = new SipxSaaService();
        saaService.setBeanId(SipxSaaService.BEAN_ID);
        saaService.setDomainManager(TestUtil.getMockDomainManager(true));
        SipxServiceManager sipxServiceManager = TestUtil.getMockSipxServiceManager(true, saaService);
        out.setSipxServiceManager(sipxServiceManager);

        assertCorrectFileGeneration(out, "expected-appearance-groups.xml");
    }
}
