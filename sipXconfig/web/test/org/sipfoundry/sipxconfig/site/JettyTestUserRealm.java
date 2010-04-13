/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

import java.security.Principal;

import org.mortbay.http.HttpRequest;
import org.mortbay.http.UserRealm;

/**
 * Simple implementation of authentication realm for unit testing.
 * Currently any user is authenticated as "superadmin" with all the roles.
 * One can add other users to test if role-based authentication works correctly.
 */
class JettyTestUserRealm implements UserRealm {

    private static final class User implements Principal {
        private String[] m_roles;

        public User(String[] roles) {
            m_roles = roles;
        }

        public String getName() {
            return "admin";
        }

        public String[] getRoles() {
            return m_roles;
        }

        public boolean isUserInRole(String role) {
            for (int i = 0; i < m_roles.length; i++) {
                if (m_roles[i].equals(role)) {
                    return true;
                }
            }
            return false;
        }
    }

    /**
     * For unit testing, all roles are considered to be admin roles.
     * Roles listed here must match roles listed in web.xml.
     */
    private static final String[] ADMIN_ROLES = { "admin" };

    User m_admin = new User(ADMIN_ROLES);

    /** Return the realm name.  Must match the realm declared in web.xml. */
    public String getName() {
        return "jetty realm";
    }

    public Principal getPrincipal(String username) {
        return m_admin;
    }

    public Principal authenticate(String username, Object credentials, HttpRequest request) {
        return m_admin;
    }

    public boolean reauthenticate(Principal user) {
        return true;
    }

    /**
     * Any role is a good role.
     */
    public boolean isUserInRole(Principal user, String role) {
        return m_admin.isUserInRole(role);
    }

    public void disassociate(Principal user) {
        // do nothing
    }

    public Principal pushRole(Principal user, String role) {
        throw new UnsupportedOperationException();
    }

    public Principal popRole(Principal user) {
        throw new UnsupportedOperationException();
    }

    public void logout(Principal user) {
        // do nothing
    }

}
