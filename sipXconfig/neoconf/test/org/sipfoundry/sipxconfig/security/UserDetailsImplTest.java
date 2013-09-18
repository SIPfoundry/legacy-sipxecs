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

import java.util.ArrayList;
import java.util.Collection;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.User;
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
        Collection<GrantedAuthority> authorities = new ArrayList<GrantedAuthority>(1);
        GrantedAuthority party = new GrantedAuthorityImpl("party");
        authorities.add(party);
        UserDetails details = new UserDetailsImpl(user, userName, authorities);

        assertTrue(details.isAccountNonExpired());
        assertTrue(details.isAccountNonLocked());
        assertTrue(details.isCredentialsNonExpired());
        assertTrue(details.isEnabled());
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
