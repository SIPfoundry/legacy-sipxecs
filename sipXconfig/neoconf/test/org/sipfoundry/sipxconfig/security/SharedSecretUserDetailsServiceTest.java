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
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.permission.PermissionName;

import static org.apache.commons.lang.ArrayUtils.contains;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SharedSecretUserDetailsServiceTest extends TestCase {
    private static final String USER_NAME = "Hjelje";
    private AcdContext m_acdContext;
    private CoreContext m_coreContext;
    private DomainManager m_domainManager;

    public void setUp() {
        m_coreContext = createMock(CoreContext.class);
        m_acdContext = createMock(AcdContext.class);
    }

    public void testLoadUserByUsername() {
        User u= new User() {
            @Override
            public boolean hasPermission(PermissionName permission) {
                return true;
            }
        };
        u.setUserName(USER_NAME);
        u.setUniqueId();

        SharedSecretUserDetailsService sharedSecretuds = new SharedSecretUserDetailsService();
        sharedSecretuds.setCoreContext(m_coreContext);
        sharedSecretuds.setAcdContext(m_acdContext);

        m_coreContext.loadUserByUserNameOrAlias(USER_NAME);
        expectLastCall().andReturn(u);

        m_acdContext.getUsersWithAgents();
        expectLastCall().andReturn(Collections.emptyList());
        replay(m_coreContext, m_acdContext);

        // load the user details
        UserDetails details = sharedSecretuds.loadUserByUsername(USER_NAME);
        assertEquals(USER_NAME, details.getUsername());
        GrantedAuthority[] authorities = details.getAuthorities();

        assertTrue(contains(authorities, UserRole.Admin.toAuth()));
        assertTrue(contains(authorities, UserRole.User.toAuth()));
        assertFalse(contains(authorities, UserRole.AcdAgent.toAuth()));
        assertFalse(contains(authorities, UserRole.AcdSupervisor.toAuth()));
        assertTrue(contains(authorities, UserRole.AttendantAdmin.toAuth()));

        verify(m_coreContext, m_acdContext);
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

        SharedSecretUserDetailsService sharedSecretuds = new SharedSecretUserDetailsService();
        sharedSecretuds.setCoreContext(m_coreContext);
        sharedSecretuds.setAcdContext(m_acdContext);
        sharedSecretuds.setDomainManager(m_domainManager);

        m_coreContext.loadUserByUserNameOrAlias(USER_NAME);
        expectLastCall().andReturn(u);

        m_acdContext.getUsersWithAgents();
        expectLastCall().andReturn(Collections.singletonList(u));
        replay(m_coreContext, m_acdContext);

        // load the user details
        UserDetails details = sharedSecretuds.loadUserByUsername(USER_NAME);
        assertEquals(USER_NAME, details.getUsername());
        GrantedAuthority[] authorities = details.getAuthorities();

        assertFalse(contains(authorities, UserRole.Admin.toAuth()));
        assertTrue(contains(authorities, UserRole.User.toAuth()));
        assertTrue(contains(authorities, UserRole.AcdAgent.toAuth()));
        assertFalse(contains(authorities, UserRole.AcdSupervisor.toAuth()));
        assertFalse(contains(authorities, UserRole.AttendantAdmin.toAuth()));

        verify(m_coreContext, m_acdContext);
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
