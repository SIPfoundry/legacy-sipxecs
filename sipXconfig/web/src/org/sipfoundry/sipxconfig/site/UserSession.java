/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site;

import org.acegisecurity.GrantedAuthority;
import org.acegisecurity.context.SecurityContextHolder;
import org.acegisecurity.userdetails.UserDetails;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.security.StandardUserDetailsService;
import org.sipfoundry.sipxconfig.security.UserDetailsImpl;
import org.sipfoundry.sipxconfig.security.UserRole;

/**
 * Tapestry Visit object - session parameters for sipXconfig
 */
public class UserSession {
    public static final String SESSION_NAME = "userSession";

    /**
     * true if we want to display title bar and navigation false for testing and when embedding
     * pages in profilegen
     */
    private boolean m_navigationVisible = true;

    /**
     * user that is currently logged in
     */
    public boolean isNavigationVisible() {
        return m_navigationVisible;
    }

    public void setNavigationVisible(boolean navigationVisible) {
        m_navigationVisible = navigationVisible;
    }

    public boolean isAdmin() {
        return checkRole(UserRole.Admin);
    }

    public boolean isSupervisor() {
        return checkRole(UserRole.AcdSupervisor);
    }

    public boolean isAgent() {
        return checkRole(UserRole.AcdAgent);
    }

    public Integer getUserId() {
        UserDetailsImpl userDetails = getUserDetails();
        if (userDetails == null) {
            return null;
        }
        return userDetails.getUserId();
    }

    /**
     * Loads a user from the specified core context
     *
     * @param coreContext
     * @return The User object associated with this session
     */
    public User getUser(CoreContext coreContext) {
        return coreContext.loadUser(getUserId());
    }

    public boolean isLoggedIn() {
        return getUserDetails() != null;
    }

    /**
     * For testing only... can inject user details from other place
     *
     * @return
     */
    protected UserDetailsImpl getUserDetails() {
        return StandardUserDetailsService.getUserDetails();
    }

    public void logout() {
        SecurityContextHolder.clearContext();
    }

    /**
     * Checks if currently logged user has a requested role.
     *
     * @param role name of the role
     * @return true id the role has been granted
     */
    private boolean checkRole(UserRole role) {
        UserDetails userDetails = getUserDetails();
        if (userDetails == null) {
            return false;
        }
        for (GrantedAuthority authority : userDetails.getAuthorities()) {
            if (authority.equals(role.toAuth())) {
                return true;
            }
        }
        return false;
    }
}
