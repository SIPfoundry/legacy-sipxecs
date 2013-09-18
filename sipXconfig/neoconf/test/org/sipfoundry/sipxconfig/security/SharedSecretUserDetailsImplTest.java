/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.security;

import java.util.ArrayList;
import java.util.Collection;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.commons.security.Md5Encoder;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.authority.GrantedAuthorityImpl;
import org.springframework.security.core.userdetails.UserDetails;

public class SharedSecretUserDetailsImplTest extends TestCase {

    private DomainManager m_domainManager;

    @Override
    public void setUp() {
        m_domainManager= TestHelper.getMockDomainManager();
        m_domainManager.getSharedSecret();
        EasyMock.expectLastCall().andReturn("secret");
        m_domainManager.getAuthorizationRealm();
        EasyMock.expectLastCall().andReturn("realm.sipfoundry.org");
        EasyMock.replay(m_domainManager);

    }
    public void testSharedSecretUserDetailsImpl() {
        final User user = new AdminUser();
        final String userName = "angelina";
        user.setUserName(userName);

        String hashedSecret = Md5Encoder.getEncodedPassword("secret");

        Collection<GrantedAuthority> authorities = new ArrayList<GrantedAuthority>(1);
        GrantedAuthority party = new GrantedAuthorityImpl("party");
        authorities.add(party);
        UserDetails details = new SharedSecretUserDetailsImpl(m_domainManager, user, userName, authorities);

        assertTrue(details.isAccountNonExpired());
        assertTrue(details.isAccountNonLocked());
        assertTrue(details.isCredentialsNonExpired());
        assertTrue(details.isEnabled());
        Collection<? extends GrantedAuthority> actualAuthorities = details.getAuthorities();
        assertEquals(1, actualAuthorities.size());
        assertTrue(actualAuthorities.contains(party));
        assertEquals(userName, details.getUsername());
        assertEquals(hashedSecret, details.getPassword());
    }

    private static class AdminUser extends User {
        @Override
        public boolean isAdmin() {
            return true;
        }
    }
}
