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

import junit.framework.TestCase;
import org.acegisecurity.GrantedAuthority;
import org.acegisecurity.GrantedAuthorityImpl;
import org.acegisecurity.userdetails.UserDetails;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.common.Md5Encoder;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class SharedSecretUserDetailsImplTest extends TestCase {

    private DomainManager m_domainManager;

    @Override
    public void setUp() {
        m_domainManager= TestUtil.getMockDomainManager();
        m_domainManager.getSharedSecret();
        EasyMock.expectLastCall().andReturn("secret");
        m_domainManager.getAuthorizationRealm();
        EasyMock.expectLastCall().andReturn("realm.sipfoundry.org");
        EasyMock.replay(m_domainManager);

    }
    public void testSharedSecretUserDetailsImpl() {
        final User user = new User();
        final String userName = "angelina";
        user.setUserName(userName);

        String hashedSecret = Md5Encoder.digestPassword(userName, "realm.sipfoundry.org", "secret");

        GrantedAuthority[] authorities = new GrantedAuthority[1];
        GrantedAuthority party = new GrantedAuthorityImpl("party");
        authorities[0] = party;
        UserDetails details = new SharedSecretUserDetailsImpl(m_domainManager, user, userName, authorities);

        assertTrue(details.isAccountNonExpired());
        assertTrue(details.isAccountNonLocked());
        assertTrue(details.isCredentialsNonExpired());
        assertTrue(details.isEnabled());
        GrantedAuthority[] actualAuthorities = details.getAuthorities();
        assertEquals(1, actualAuthorities.length);
        assertEquals(party, actualAuthorities[0]);
        assertEquals(userName, details.getUsername());
        assertEquals(hashedSecret, details.getPassword());
    }
}
