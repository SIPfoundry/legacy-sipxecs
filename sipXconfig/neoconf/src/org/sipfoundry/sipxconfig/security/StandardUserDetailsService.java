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

import java.util.List;

import org.acegisecurity.Authentication;
import org.acegisecurity.GrantedAuthority;
import org.acegisecurity.context.SecurityContext;
import org.acegisecurity.context.SecurityContextHolder;
import org.acegisecurity.userdetails.UserDetails;
import org.sipfoundry.sipxconfig.common.User;

public class StandardUserDetailsService extends AbstractUserDetailsService {
    @Override
    protected UserDetails createUserDetails(String userNameOrAlias, User user, List<GrantedAuthority> gas) {
        return new UserDetailsImpl(user, userNameOrAlias, gas.toArray(new GrantedAuthority[gas.size()]));
    }

    public static UserDetailsImpl getUserDetails() {
        SecurityContext context = SecurityContextHolder.getContext();
        if (context == null) {
            return null;
        }
        Authentication authentication = context.getAuthentication();
        if (authentication == null) {
            return null;
        }
        // depending on authentication token user details are kept as details or as a principal...
        Object principal = authentication.getPrincipal();
        if (principal instanceof UserDetailsImpl) {
            return (UserDetailsImpl) principal;
        }
        Object details = authentication.getDetails();
        if (details instanceof UserDetailsImpl) {
            return (UserDetailsImpl) details;
        }
        return null;
    }
}
