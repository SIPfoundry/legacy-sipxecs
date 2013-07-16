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

import java.util.Collection;
import java.util.Collections;

import junit.framework.TestCase;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UsernameNotFoundException;

import static org.apache.commons.lang.ArrayUtils.contains;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class StandardUserDetailsServiceTest extends TestCase {
    private static final String USER_NAME = "Hjelje";
    private static final String USER_IM_ID = "staal";

    public void testLoadUserByUsername() {
        User u = new User() {
            @Override
            public boolean hasPermission(PermissionName permission) {
                if (permission.equals(PermissionName.SUPERADMIN) || permission.equals(PermissionName.RECORD_SYSTEM_PROMPTS)) {
                    return true;
                }
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

        replay(coreContext);

        // load the user details
        UserDetails details = uds.loadUserByUsername(USER_NAME);
        assertEquals(USER_NAME, details.getUsername());
        Collection<? extends GrantedAuthority> authorities = details.getAuthorities();

        assertTrue(authorities.contains(UserRole.Admin.toAuth()));
        assertTrue(authorities.contains(UserRole.User.toAuth()));
        assertFalse(authorities.contains(UserRole.AcdAgent.toAuth()));
        assertFalse(authorities.contains(UserRole.AcdSupervisor.toAuth()));
        assertTrue(authorities.contains(UserRole.AttendantAdmin.toAuth()));

        verify(coreContext);
    }

    public void testLoadUserByConfiguredImId() {
        CoreContext coreContext = createMock(CoreContext.class);
        UserDetails details = null;
        try {
            details = getUserDetailsTestService(coreContext, false).loadUserByUsername(USER_IM_ID);
            fail();
        } catch (UsernameNotFoundException e) {
            assertTrue(e.getMessage().contains(USER_IM_ID));
        }

        verify(coreContext);

        CoreContext coreContext1 = createMock(CoreContext.class);
        details = getUserDetailsTestService(coreContext1, true).loadUserByUsername(USER_IM_ID);
        assertEquals(USER_IM_ID, details.getUsername());

        verify(coreContext);
    }

    private StandardUserDetailsService getUserDetailsTestService(CoreContext ctx, boolean imEnabled) {
        User u = new User() {
            @Override
            public boolean hasPermission(PermissionName permission) {
                return true;
            }
        };
        u.setUserName(USER_NAME);
        u.setUniqueId();
        UserProfile abe = new UserProfile();
        abe.setImId(USER_IM_ID);
        u.setUserProfile(abe);
        u.setSettings(TestHelper.loadSettings("commserver/user-settings.xml"));
        u.setSettingTypedValue("im/im-account", imEnabled);

        AcdContext acdContext = createMock(AcdContext.class);
        StandardUserDetailsService uds = new StandardUserDetailsService();
        uds.setCoreContext(ctx);
        uds.setAcdContext(acdContext);

        ctx.loadUserByUserNameOrAlias(USER_IM_ID);
        expectLastCall().andReturn(null);

        ctx.loadUserByConfiguredImId(USER_IM_ID);
        expectLastCall().andReturn(u);

        acdContext.getUsersWithAgents();
        expectLastCall().andReturn(Collections.emptyList());

        replay(ctx, acdContext);
        return uds;
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

        replay(coreContext);

        // load the user details
        UserDetails details = uds.loadUserByUsername(USER_NAME);
        assertEquals(USER_NAME, details.getUsername());
        Collection<? extends GrantedAuthority> authorities = details.getAuthorities();

        assertFalse(authorities.contains(UserRole.Admin.toAuth()));
        assertTrue(authorities.contains(UserRole.User.toAuth()));
        assertFalse(authorities.contains(UserRole.AcdSupervisor.toAuth()));
        assertFalse(authorities.contains(UserRole.AttendantAdmin.toAuth()));

        verify(coreContext);
    }

    public void testNoUser() {
        CoreContext coreContext = createMock(CoreContext.class);
        StandardUserDetailsService uds = new StandardUserDetailsService();
        uds.setCoreContext(coreContext);

        coreContext.loadUserByUserNameOrAlias(USER_NAME);
        expectLastCall().andReturn(null);

        coreContext.loadUserByConfiguredImId(USER_NAME);
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
