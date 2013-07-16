/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.security;

import java.util.Collection;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.userdetails.UserDetails;

public class UserDetailsImpl implements UserDetails {
    private final String m_canonicalUserName;
    private final Integer m_userId;
    private final String m_userNameOrAlias;
    private final String m_pintoken;  // MD5-encoded password
    private final Collection<GrantedAuthority> m_authorities;
    private final String m_userDomain;
    private final boolean m_enabled;

    /**
     * UserDetails constructor
     *
     * Create an Spring Security UserDetails object based on the sipXconfig User, the
     * userNameOrAlias that is the userName part of the user's credentials, and the
     * authorities granted to this user.
     */
    public UserDetailsImpl(User user, String userNameOrAlias, Collection<GrantedAuthority> authorities) {
        m_canonicalUserName = user.getUserName();
        m_userId = user.getId();
        m_userNameOrAlias = userNameOrAlias;
        m_pintoken = user.getPintoken();
        m_authorities = authorities;
        m_userDomain = user.getUserDomain();
        m_enabled = user.isEnabled();
    }

    @Override
    public boolean isAccountNonExpired() {
        return m_enabled;
    }

    @Override
    public boolean isAccountNonLocked() {
        return m_enabled;
    }

    @Override
    public Collection<GrantedAuthority> getAuthorities() {
        return m_authorities;
    }

    @Override
    public boolean isCredentialsNonExpired() {
        return m_enabled;
    }

    @Override
    public boolean isEnabled() {
        return m_enabled;
    }

    @Override
    public String getPassword() {
        return m_pintoken;
    }

    /**
     * Return the userName or alias that is the userName part of the user's credentials.
     */
    @Override
    public String getUsername() {
        return m_userNameOrAlias;
    }

    public Integer getUserId() {
        return m_userId;
    }

    /**
     * Return the "canonical" userName.  You can log in with either the canonical userName
     * or an alias.  To a client of UserDetails, there is no difference, *but* the
     * canonical userName is used when MD5-encoding the password.
     */
    public String getCanonicalUserName() {
        return m_canonicalUserName;
    }

    public String getUserDomain() {
        return m_userDomain;
    }
}
