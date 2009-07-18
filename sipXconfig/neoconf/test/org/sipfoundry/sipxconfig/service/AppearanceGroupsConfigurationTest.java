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

import org.easymock.classextension.EasyMock;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

public class AppearanceGroupsConfigurationTest extends SipxServiceTestBase {
    public void testWrite() throws Exception {
        AppearanceGroupsConfiguration out = new AppearanceGroupsConfiguration();
        out.setTemplate("sipxsaa/appearance-groups.vm");

        CoreContext coreContext = EasyMock.createMock(CoreContext.class);
        coreContext.getSharedUsers();

        User firstSharedUser = new User();
        firstSharedUser.setUserName("sharedline");
        User secondSharedUser = new User();
        secondSharedUser.setUserName("321");

        EasyMock.expectLastCall().andReturn(Arrays.asList(new User[] {
            firstSharedUser, secondSharedUser
        }));
        EasyMock.replay(coreContext);

        out.setCoreContext(coreContext);

        assertCorrectFileGeneration(out, "expected-appearance-groups.xml");
    }
}
