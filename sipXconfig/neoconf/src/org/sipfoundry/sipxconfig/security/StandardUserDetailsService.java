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
import java.util.List;

import org.acegisecurity.Authentication;
import org.acegisecurity.GrantedAuthority;
import org.acegisecurity.context.SecurityContext;
import org.acegisecurity.context.SecurityContextHolder;
import org.acegisecurity.userdetails.UserDetails;
import org.acegisecurity.userdetails.UserDetailsService;
import org.acegisecurity.userdetails.UsernameNotFoundException;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.beans.factory.annotation.Required;

import static org.sipfoundry.sipxconfig.permission.PermissionName.RECORD_SYSTEM_PROMPTS;
import static org.sipfoundry.sipxconfig.security.UserRole.AcdAgent;
import static org.sipfoundry.sipxconfig.security.UserRole.AcdSupervisor;
import static org.sipfoundry.sipxconfig.security.UserRole.Admin;
import static org.sipfoundry.sipxconfig.security.UserRole.AttendantAdmin;
import static org.sipfoundry.sipxconfig.security.UserRole.User;

public class StandardUserDetailsService implements UserDetailsService {
    private CoreContext m_coreContext;

    private AcdContext m_acdContext;

    public UserDetails loadUserByUsername(String userNameOrAlias) {
        User user = m_coreContext.loadUserByUserNameOrAlias(userNameOrAlias);
        if (user == null) {
            throw new UsernameNotFoundException(userNameOrAlias);
        }

        List<GrantedAuthority> gas = new ArrayList<GrantedAuthority>(4);
        gas.add(User.toAuth());

        if (user.isAdmin()) {
            gas.add(Admin.toAuth());
        }
        if (user.isSupervisor()) {
            gas.add(AcdSupervisor.toAuth());
        }
        // FIXME: should be possible to implement without loading all agents in memory
        boolean isAgent = m_acdContext.getUsersWithAgents().contains(user);
        if (isAgent) {
            gas.add(AcdAgent.toAuth());
        }
        if (user.hasPermission(RECORD_SYSTEM_PROMPTS)) {
            gas.add(AttendantAdmin.toAuth());
        }

        return new UserDetailsImpl(user, userNameOrAlias, gas.toArray(new GrantedAuthority[gas.size()]));
    }

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setAcdContext(AcdContext acdContext) {
        m_acdContext = acdContext;
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
