/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.security;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;

import java.util.ArrayList;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.security.authentication.dao.SaltSource;
import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.userdetails.UserDetails;

public class SaltSourceImplTest extends TestCase {
    public void testGetSalt() {
        SaltSource ss = new SaltSourceImpl();
        assertNull(ss.getSalt(new LocationDetailsImpl(new Location())));

        User u = new UserDetailsImplTest.RegularUser();
        u.setUserName("bongo");
        PermissionManager pManager = createMock(PermissionManager.class);
        pManager.getPermissionModel();
        expectLastCall().andReturn(TestHelper.loadSettings("commserver/user-settings.xml")).anyTimes();
        replay(pManager);
        u.setPermissionManager(pManager);
        UserDetails user = new UserDetailsImpl(u, "userNameOrAlias", new ArrayList<GrantedAuthority>());
        assertEquals("bongo", ss.getSalt(user));
    }

}
