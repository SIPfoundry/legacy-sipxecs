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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

import java.util.Collection;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UsernameNotFoundException;

public class SharedSecretUserDetailsServiceTest extends TestCase {
    private static final String USER_NAME = "Hjelje";
    private CoreContext m_coreContext;
    private DomainManager m_domainManager;

    public void setUp() {
        m_coreContext = createMock(CoreContext.class);
    }

    public void testLoadUserByUsername() {
        User u= new User() {
            @Override
            public boolean hasPermission(PermissionName permission) {
                return false;
            }
        };
        u.setUserName(USER_NAME);
        u.setUniqueId();

        SharedSecretUserDetailsService sharedSecretuds = new SharedSecretUserDetailsService();
        sharedSecretuds.setCoreContext(m_coreContext);

        m_coreContext.loadUserByUserNameOrAlias(USER_NAME);
        expectLastCall().andReturn(u);

        replay(m_coreContext);

        // load the user details
        UserDetails details = sharedSecretuds.loadUserByUsername(USER_NAME);
        assertEquals(USER_NAME, details.getUsername());
        Collection<? extends GrantedAuthority> authorities = details.getAuthorities();

        assertFalse(authorities.contains(UserRole.Admin.toAuth()));
        assertTrue(authorities.contains(UserRole.User.toAuth()));
        assertFalse(authorities.contains(UserRole.AttendantAdmin.toAuth()));

        verify(m_coreContext);
    }

    public void testLoadUserByUsernameDifferentRoles() {
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

        SharedSecretUserDetailsService sharedSecretuds = new SharedSecretUserDetailsService();
        sharedSecretuds.setCoreContext(m_coreContext);
        sharedSecretuds.setDomainManager(m_domainManager);

        m_coreContext.loadUserByUserNameOrAlias(USER_NAME);
        expectLastCall().andReturn(u);

        replay(m_coreContext);

        // load the user details
        UserDetails details = sharedSecretuds.loadUserByUsername(USER_NAME);
        assertEquals(USER_NAME, details.getUsername());
        Collection<? extends GrantedAuthority> authorities = details.getAuthorities();

        assertTrue(authorities.contains(UserRole.Admin.toAuth()));
        assertTrue(authorities.contains(UserRole.User.toAuth()));
        assertTrue(authorities.contains(UserRole.AttendantAdmin.toAuth()));

        verify(m_coreContext);
    }

    public void testNoUser() {
        SharedSecretUserDetailsService sharedSecretuds = new SharedSecretUserDetailsService();
        sharedSecretuds.setCoreContext(m_coreContext);

        m_coreContext.loadUserByUserNameOrAlias(USER_NAME);
        expectLastCall().andReturn(null);

        m_coreContext.loadUserByConfiguredImId(USER_NAME);
        expectLastCall().andReturn(null);

        replay(m_coreContext);

        try {
            sharedSecretuds.loadUserByUsername(USER_NAME);
            fail("Should throw authorization exception");
        } catch (UsernameNotFoundException e) {
            assertTrue(e.getMessage().contains(USER_NAME));
        }
        verify(m_coreContext);
    }
}
