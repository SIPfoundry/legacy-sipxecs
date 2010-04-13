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

import org.acegisecurity.GrantedAuthority;
import org.acegisecurity.GrantedAuthorityImpl;
import org.acegisecurity.userdetails.UserDetails;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

public class LocationDetailsImpl implements UserDetails {
    private static final GrantedAuthority AUTH_LOCATION = new GrantedAuthorityImpl(Location.ROLE_LOCATION);

    private final String m_hostFqdn;
    private final String m_password;
    private final GrantedAuthority[] m_authorities = {
        AUTH_LOCATION
    };

    /**
     * LocationDetails constructor
     *
     * Create an Acegi Security LocationDetails object based on the Location, the userNameOrAlias
     * that is the location fqdn, and the authorities granted to this location.
     */
    public LocationDetailsImpl(Location location) {
        m_hostFqdn = location.getFqdn();
        m_password = location.getPassword();
    }

    public boolean isAccountNonExpired() {
        return true; // accounts don't expire
    }

    public boolean isAccountNonLocked() {
        return true; // accounts are never locked
    }

    public GrantedAuthority[] getAuthorities() {
        return m_authorities;
    }

    public boolean isCredentialsNonExpired() {
        return true; // credentials don't expire
    }

    public boolean isEnabled() {
        return true; // accounts are always enabled
    }

    /** Return the password */
    public String getPassword() {
        return m_password;
    }

    /**
     * Returns the host fully qualified domain name.
     */
    public String getUsername() {
        return m_hostFqdn;
    }
}
