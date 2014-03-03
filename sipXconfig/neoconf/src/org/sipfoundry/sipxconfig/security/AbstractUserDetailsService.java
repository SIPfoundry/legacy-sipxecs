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

import static org.sipfoundry.sipxconfig.permission.PermissionName.RECORD_SYSTEM_PROMPTS;
import static org.sipfoundry.sipxconfig.security.UserRole.Admin;
import static org.sipfoundry.sipxconfig.security.UserRole.AttendantAdmin;
import static org.sipfoundry.sipxconfig.security.UserRole.User;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.core.userdetails.UsernameNotFoundException;

public abstract class AbstractUserDetailsService implements UserDetailsService {

    private CoreContext m_coreContext;
    private AdminContext m_adminContext;
    private AdditionalAuthoritiesLoader m_authLoader;

    public final UserDetails loadUserByUsername(String userNameOrAliasOrImIdOrAuthAccnameOrEmail) {
        User user = m_coreContext.loadUserByUserNameOrAlias(userNameOrAliasOrImIdOrAuthAccnameOrEmail);
        if (user == null) {
            // 2nd attempt - try to login as an imID
            user = getUserForImId(userNameOrAliasOrImIdOrAuthAccnameOrEmail);
        }

        if (user == null && m_adminContext.isAuthAccName()) {
            // 3rd attempt - try to login as an authorization account name
            List<User> users = getUsersForAuthAccountName(userNameOrAliasOrImIdOrAuthAccnameOrEmail);
            user = checkDuplicateUser(users);
        }

        if (user == null && m_adminContext.isAuthEmailAddress()) {
            // 4th attempt - try to login as an email
            List<User> users = getUsersForEmail(userNameOrAliasOrImIdOrAuthAccnameOrEmail);
            user = checkDuplicateUser(users);
        }

        if (user == null) {
            throw new UsernameNotFoundException(userNameOrAliasOrImIdOrAuthAccnameOrEmail);
        }

        return createUserDetails(userNameOrAliasOrImIdOrAuthAccnameOrEmail, user);
    }

    private User checkDuplicateUser(List<User> users) {
        if (users == null) {
            return null;
        }
        if (users.size() != 1) {
            if (users.isEmpty()) {
                return null;
            } else {
                throw new DuplicateUserException("There are at least two users with same auth name or email", users);
            }
        } else {
            return users.get(0);
        }
    }

    public UserDetails createUserDetails(String userNameOrAliasOrImIdOrAuthAccnameOrEmail, User user) {
        List<GrantedAuthority> gas = new ArrayList<GrantedAuthority>(4);
        gas.add(User.toAuth());

        if (user.isAdmin()) {
            gas.add(Admin.toAuth());
        }

        if (user.hasPermission(RECORD_SYSTEM_PROMPTS)) {
            gas.add(AttendantAdmin.toAuth());
        }
        if (m_authLoader != null) {
            m_authLoader.addUserAuthorities(user, gas);
        }

        return createUserDetails(userNameOrAliasOrImIdOrAuthAccnameOrEmail, user, gas);
    }

    private User getUserForImId(String imId) {
        User user = m_coreContext.loadUserByConfiguredImId(imId);
        if (user != null) {
            ImAccount imAccount = new ImAccount(user);
            if (!imAccount.isEnabled()) {
                return null;
            }
        }

        return user;
    }

    private List<User> getUsersForAuthAccountName(String authAccountName) {
        return m_coreContext.loadUsersByAuthAccountName(authAccountName);
    }

    private List<User> getUsersForEmail(String email) {
        return m_coreContext.loadUsersByEmail(email);
    }

    protected abstract UserDetails createUserDetails(String userNameOrAlias, User user, List<GrantedAuthority> gas);

    @Required
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Required
    public void setAdminContext(AdminContext adminContext) {
        m_adminContext = adminContext;
    }

    public void setAdditionalAuthoritiesLoader(AdditionalAuthoritiesLoader loader) {
        m_authLoader = loader;
    }
}
