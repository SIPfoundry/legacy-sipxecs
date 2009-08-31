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

import java.util.Collections;

import junit.framework.TestCase;
import org.acegisecurity.GrantedAuthority;
import org.acegisecurity.userdetails.UserDetails;
import org.acegisecurity.userdetails.UsernameNotFoundException;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionName;

import static org.apache.commons.lang.ArrayUtils.contains;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class StandardUserDetailsServiceTest extends TestCase {
    private static final String USER_NAME = "Hjelje";

    public void testLoadUserByUsername() {
        User u = new User() {
            @Override
            public boolean hasPermission(PermissionName permission) {
                return true;
            }
        };
        u.setUserName(USER_NAME);
        u.setUniqueId();

        CoreContext coreContext = createMock(CoreContext.class);
        AcdContext acdContext = createMock(AcdContext.class);
        StandardUserDetailsService uds = new StandardUserDetailsService();
        uds.setCoreContext(coreContext);
        uds.setAcdContext(acdContext);

        coreContext.loadUserByUserNameOrAlias(USER_NAME);
        expectLastCall().andReturn(u);

        acdContext.getUsersWithAgents();
        expectLastCall().andReturn(Collections.emptyList());
        replay(coreContext, acdContext);

        // load the user details
        UserDetails details = uds.loadUserByUsername(USER_NAME);
        assertEquals(USER_NAME, details.getUsername());
        GrantedAuthority[] authorities = details.getAuthorities();

        assertTrue(contains(authorities, UserRole.Admin.toAuth()));
        assertTrue(contains(authorities, UserRole.User.toAuth()));
        assertFalse(contains(authorities, UserRole.AcdAgent.toAuth()));
        assertFalse(contains(authorities, UserRole.AcdSupervisor.toAuth()));
        assertTrue(contains(authorities, UserRole.AttendantAdmin.toAuth()));

        verify(coreContext, acdContext);
    }

    public void testLoadUserByUsernameDifferentRoles() {
        User u = new User() {
            @Override
            public boolean hasPermission(PermissionName permission) {
                return false;
            }
        };
        u.setUserName(USER_NAME);
        u.setUniqueId();

        CoreContext coreContext = createMock(CoreContext.class);
        AcdContext acdContext = createMock(AcdContext.class);
        StandardUserDetailsService uds = new StandardUserDetailsService();
        uds.setCoreContext(coreContext);
        uds.setAcdContext(acdContext);

        coreContext.loadUserByUserNameOrAlias(USER_NAME);
        expectLastCall().andReturn(u);

        acdContext.getUsersWithAgents();
        expectLastCall().andReturn(Collections.singletonList(u));
        replay(coreContext, acdContext);

        // load the user details
        UserDetails details = uds.loadUserByUsername(USER_NAME);
        assertEquals(USER_NAME, details.getUsername());
        GrantedAuthority[] authorities = details.getAuthorities();

        assertFalse(contains(authorities, UserRole.Admin.toAuth()));
        assertTrue(contains(authorities, UserRole.User.toAuth()));
        assertTrue(contains(authorities, UserRole.AcdAgent.toAuth()));
        assertFalse(contains(authorities, UserRole.AcdSupervisor.toAuth()));
        assertFalse(contains(authorities, UserRole.AttendantAdmin.toAuth()));

        verify(coreContext, acdContext);
    }

    public void testNoUser() {
        CoreContext coreContext = createMock(CoreContext.class);
        StandardUserDetailsService uds = new StandardUserDetailsService();
        uds.setCoreContext(coreContext);

        coreContext.loadUserByUserNameOrAlias(USER_NAME);
        expectLastCall().andReturn(null);

        replay(coreContext);

        try {
            uds.loadUserByUsername(USER_NAME);
            fail("Should throw authorization exception");
        } catch (UsernameNotFoundException e) {
            assertTrue(e.getMessage().contains(USER_NAME));
        }
        verify(coreContext);
    }
}
