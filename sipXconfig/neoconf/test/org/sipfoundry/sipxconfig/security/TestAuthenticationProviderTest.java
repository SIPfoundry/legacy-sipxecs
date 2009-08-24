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
import org.acegisecurity.Authentication;
import org.acegisecurity.GrantedAuthority;
import org.acegisecurity.providers.UsernamePasswordAuthenticationToken;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;

import static org.apache.commons.lang.ArrayUtils.contains;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expect;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.sipfoundry.sipxconfig.security.TestAuthenticationProvider.DUMMY_ADMIN_USER_NAME;

public class TestAuthenticationProviderTest extends TestCase {
    public void testLoadUserByUsernameDebugVsNonDebug() {

        CoreContext coreContext = createMock(CoreContext.class);
        expect(coreContext.getDebug()).andReturn(false);
        expect(coreContext.getDebug()).andReturn(true);

        replay(coreContext);

        TestAuthenticationProvider tap = new TestAuthenticationProvider();
        tap.setCoreContext(coreContext);

        TestAuthenticationToken token = new TestAuthenticationToken(new User(), false, false);

        Authentication authentication = tap.authenticate(token);
        assertNull(authentication);

        authentication = tap.authenticate(token);
        assertNotNull(authentication);
        assertTrue(authentication.isAuthenticated());

        verify(coreContext);
    }

    public void testSupports() {
        TestAuthenticationProvider tap = new TestAuthenticationProvider();
        assertTrue(tap.supports(TestAuthenticationToken.class));
        assertTrue(tap.supports(UsernamePasswordAuthenticationToken.class));
        assertFalse(tap.supports(Authentication.class));
        assertFalse(tap.supports(String.class));
    }

    public void testTestAuthentication() {
        CoreContext coreContext = createMock(CoreContext.class);
        expect(coreContext.getDebug()).andReturn(true);

        replay(coreContext);

        TestAuthenticationProvider tap = new TestAuthenticationProvider();
        tap.setCoreContext(coreContext);

        User user = new User();
        user.setUniqueId();
        user.setName("bongo");

        TestAuthenticationToken token = new TestAuthenticationToken(user, false, true);
        Authentication authentication = tap.authenticate(token);

        assertTrue(authentication.isAuthenticated());
        GrantedAuthority[] authorities = authentication.getAuthorities();
        assertFalse(contains(authorities, UserRole.Admin.toAuth()));
        assertTrue(contains(authorities, UserRole.User.toAuth()));
        assertFalse(contains(authorities, UserRole.AcdAgent.toAuth()));
        assertTrue(contains(authorities, UserRole.AcdSupervisor.toAuth()));

        Object details = authentication.getDetails();
        assertTrue(details instanceof UserDetailsImpl);
        UserDetailsImpl udi = (UserDetailsImpl) details;

        assertEquals("bongo", udi.getCanonicalUserName());
        assertEquals(user.getId(), udi.getUserId());
    }

    public void testDummyUserAuthentication() {
        CoreContext coreContext = createMock(CoreContext.class);
        expect(coreContext.getDebug()).andReturn(true).anyTimes();

        replay(coreContext);

        TestAuthenticationProvider tap = new TestAuthenticationProvider();
        tap.setCoreContext(coreContext);

        Authentication token1 = new UsernamePasswordAuthenticationToken("bongo", "kuku");
        assertNull(tap.authenticate(token1));

        Authentication token2 = new UsernamePasswordAuthenticationToken(DUMMY_ADMIN_USER_NAME, "kuku");
        Authentication authentication = tap.authenticate(token2);
        assertNotNull(authentication);
        assertTrue(authentication.isAuthenticated());

        GrantedAuthority[] authorities = authentication.getAuthorities();
        assertTrue(contains(authorities, UserRole.Admin.toAuth()));
        assertTrue(contains(authorities, UserRole.User.toAuth()));
        assertFalse(contains(authorities, UserRole.AcdAgent.toAuth()));
        assertFalse(contains(authorities, UserRole.AcdSupervisor.toAuth()));

        Object details = authentication.getDetails();
        assertTrue(details instanceof UserDetailsImpl);
        UserDetailsImpl udi = (UserDetailsImpl) details;

        assertEquals(DUMMY_ADMIN_USER_NAME, udi.getCanonicalUserName());
    }
}
