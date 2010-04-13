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

import org.acegisecurity.GrantedAuthority;
import org.acegisecurity.GrantedAuthorityImpl;

public enum UserRole {
    User("ROLE_USER"), Admin("ROLE_ADMIN"), AcdAgent("ROLE_ACD_AGENT"), AcdSupervisor("ROLE_ACD_SUPERVISOR"),
        AttendantAdmin("ROLE_ATTENDANT_ADMIN");

    private String m_role;

    UserRole(String role) {
        m_role = role;
    }

    public GrantedAuthority toAuth() {
        return new GrantedAuthorityImpl(m_role);
    }

    public String toRole() {
        return m_role;
    }
}
