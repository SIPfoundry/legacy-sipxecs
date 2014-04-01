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
import java.util.Collection;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.authority.GrantedAuthorityImpl;
import org.springframework.security.core.userdetails.UserDetails;

public class UserDetailsImplTest extends TestCase {
    public void testUserDetailsImpl() {
        final User user = new RegularUser();
        final String userName = "angelina";
        user.setUserName(userName);
        final String pintoken = "lara";
        user.setPintoken(pintoken);
        PermissionManager pManager = createMock(PermissionManager.class);
        pManager.getPermissionModel();
        expectLastCall().andReturn(TestHelper.loadSettings("commserver/user-settings.xml")).anyTimes();
        replay(pManager);
        user.setPermissionManager(pManager);
        Collection<GrantedAuthority> authorities = new ArrayList<GrantedAuthority>(1);
        GrantedAuthority party = new GrantedAuthorityImpl("party");
        authorities.add(party);
        UserDetails details = new UserDetailsImpl(user, userName, authorities);

        assertTrue(details.isAccountNonExpired());
        assertTrue(details.isAccountNonLocked());
        assertTrue(details.isCredentialsNonExpired());
        assertTrue(details.isEnabled());
        assertFalse(((UserDetailsImpl)details).isDbAuthOnly());
        Collection<? extends GrantedAuthority> actualAuthorities = details.getAuthorities();
        assertEquals(1, actualAuthorities.size());
        assertTrue(actualAuthorities.contains(party));
        assertEquals(userName, details.getUsername());
        assertEquals(pintoken, details.getPassword());
    }

    public static class RegularUser extends User {
        @Override
        public boolean isAdmin() {
            return false;
        }
    }
}
