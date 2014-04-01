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

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;
import static org.sipfoundry.sipxconfig.security.UserRole.Admin;
import static org.sipfoundry.sipxconfig.security.UserRole.User;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.security.authentication.AbstractAuthenticationToken;
import org.springframework.security.authentication.AuthenticationProvider;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.GrantedAuthority;

public class TestAuthenticationProvider implements AuthenticationProvider {
    static final String DUMMY_ADMIN_USER_NAME = "dummyAdminUserNameForTestingOnly";

    private static final Collection<GrantedAuthority> AUTH_USER_AND_ADMIN_COLLECTION =
            new ArrayList<GrantedAuthority>(Arrays.asList(new GrantedAuthority[]{User.toAuth(), Admin.toAuth()}));

    private CoreContext m_coreContext;

    @Override
    public Authentication authenticate(Authentication authentication) {
        if (!m_coreContext.getDebug()) {
            // this authentication only works in DEBUG mode
            return null;
        }

        if (authentication instanceof TestAuthenticationToken) {
            return ((TestAuthenticationToken) authentication).authenticateToken();
        }
        if (authentication instanceof UsernamePasswordAuthenticationToken) {
            return authenticateDummyUser((UsernamePasswordAuthenticationToken) authentication);
        }
        return null;
    }

    /**
     * Only authenticated the token is username matches dummy test username.
     *
     * @param token input token
     * @return authenticated token
     */
    private Authentication authenticateDummyUser(UsernamePasswordAuthenticationToken token) {
        if (!DUMMY_ADMIN_USER_NAME.equals(token.getName())) {
            return null;
        }

        User testUser = new RegularUser();
        testUser.setUserName(DUMMY_ADMIN_USER_NAME);
        testUser.setPintoken("");

        PermissionManager pManager = createMock(PermissionManager.class);
        pManager.getPermissionModel();
        expectLastCall().andReturn(TestHelper.loadSettings("commserver/user-settings.xml")).anyTimes();
        replay(pManager);
        testUser.setPermissionManager(pManager);

        UserDetailsImpl userDetails =
                new UserDetailsImpl(testUser, DUMMY_ADMIN_USER_NAME, AUTH_USER_AND_ADMIN_COLLECTION);
        AbstractAuthenticationToken result = new UsernamePasswordAuthenticationToken(token.getPrincipal(), token
                .getCredentials(), AUTH_USER_AND_ADMIN_COLLECTION);
        result.setDetails(userDetails);
        return result;
    }

    @Override
    public boolean supports(Class authentication) {
        return (UsernamePasswordAuthenticationToken.class.isAssignableFrom(authentication))
                || (TestAuthenticationToken.class.isAssignableFrom(authentication));
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    private static class RegularUser extends User {
        @Override
        public boolean isAdmin() {
            return false;
        }
    }
}
