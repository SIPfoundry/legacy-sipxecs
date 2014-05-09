package org.sipfoundry.sipxconfig.rest;

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.security.TestAuthenticationToken;
import org.sipfoundry.sipxconfig.security.UserDetailsImpl;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.GrantedAuthority;

public class TestAuthenticationTokenWithCredentials extends TestAuthenticationToken {
    private static final long serialVersionUID = 3486010975942546200L;

    public TestAuthenticationTokenWithCredentials(User user, boolean isAdmin, boolean isSupervisor) {
        super(user, isAdmin, isSupervisor);
    }

    @Override
    public Authentication authenticateToken() {
        User user = (User) getPrincipal();
        Collection<GrantedAuthority> authorities = getAuthorities();
        UserDetailsImpl detailsImpl = new UserDetailsImpl(user, user.getUserName(), authorities, false);

        UsernamePasswordAuthenticationToken result = new UsernamePasswordAuthenticationToken(user.getUserName(),
                user.getPintoken(), authorities);
        result.setDetails(detailsImpl);
        return result;
    }
}
