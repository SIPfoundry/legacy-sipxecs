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
import java.util.List;

import org.acegisecurity.Authentication;
import org.acegisecurity.GrantedAuthority;
import org.acegisecurity.providers.AbstractAuthenticationToken;
import org.acegisecurity.providers.UsernamePasswordAuthenticationToken;
import org.sipfoundry.sipxconfig.common.User;

import static org.sipfoundry.sipxconfig.security.UserRole.AcdSupervisor;
import static org.sipfoundry.sipxconfig.security.UserRole.Admin;
import static org.sipfoundry.sipxconfig.security.UserRole.User;

public class TestAuthenticationToken extends AbstractAuthenticationToken {
    private final User m_user;

    public TestAuthenticationToken(User user, boolean isAdmin, boolean isSupervisor) {
        super(createAuthorities(isAdmin, isSupervisor));
        m_user = user;
    }

    @Override
    public Object getCredentials() {
        return null;
    }

    @Override
    public Object getPrincipal() {
        return m_user;
    }

    /**
     * Always authenticate a test token. Create username and password authentication token that
     * matches user in test token. Create user details that is required by sipXconfig.
     *
     * @return authenticated token
     */
    public Authentication authenticateToken() {
        User user = (User) getPrincipal();
        GrantedAuthority[] authorities = getAuthorities();
        UserDetailsImpl detailsImpl = new UserDetailsImpl(user, user.getUserName(), authorities);

        UsernamePasswordAuthenticationToken result = new UsernamePasswordAuthenticationToken(user.getUserName(),
                null, authorities);
        result.setDetails(detailsImpl);
        return result;
    }

    private static GrantedAuthority[] createAuthorities(boolean isAdmin, boolean isSupervisor) {
        List<GrantedAuthority> gas = new ArrayList<GrantedAuthority>(3);
        gas.add(User.toAuth());

        if (isAdmin) {
            gas.add(Admin.toAuth());
        }
        if (isSupervisor) {
            gas.add(AcdSupervisor.toAuth());
        }
        return gas.toArray(new GrantedAuthority[gas.size()]);
    }
}
