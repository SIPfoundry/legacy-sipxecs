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

import org.acegisecurity.GrantedAuthority;
import org.acegisecurity.userdetails.UserDetails;
import org.sipfoundry.sipxconfig.common.User;

public class UserDetailsImpl implements UserDetails {
    private String m_canonicalUserName;
    private String m_userNameOrAlias;
    private String m_pintoken;  // MD5-encoded password
    private GrantedAuthority[] m_authorities;
    
    /**
     * UserDetails constructor
     * 
     * Create an Acegi Security UserDetails object based on the sipXconfig User, the
     * userNameOrAlias that is the userName part of the user's credentials, and the
     * authorities granted to this user.
     */
    public UserDetailsImpl(User user, String userNameOrAlias, GrantedAuthority[] authorities) {
        m_canonicalUserName = user.getUserName();
        m_userNameOrAlias = userNameOrAlias;
        m_pintoken = user.getPintoken();
        m_authorities = authorities;
    }
    
    //================================================================================
    // Implement UserDetails interface
    
    public boolean isAccountNonExpired() {
        return true;            // accounts don't expire
    }

    public boolean isAccountNonLocked() {
        return true;            // accounts are never locked
    }

    public GrantedAuthority[] getAuthorities() {
        return m_authorities;
    }

    public boolean isCredentialsNonExpired() {
        return true;            // credentials don't expire
    }

    public boolean isEnabled() {
        return true;            // accounts are always enabled
    }

    /** Return the MD5-encoded password, a.k.a. "pintoken" */
    public String getPassword() {
        return m_pintoken;
    }

    /**
     * Return the userName or alias that is the userName part of the user's credentials.
     */
    public String getUsername() {
        return m_userNameOrAlias;
    }
    //================================================================================

    public void setAuthorities(GrantedAuthority[] authorities) {
        m_authorities = authorities;
    }

    /**
     * Return the "canonical" userName.  You can log in with either the canonical userName
     * or an alias.  To a client of UserDetails, there is no difference, *but* the
     * canonical userName is used when MD5-encoding the password.
     */
    public String getCanonicalUserName() {
        return m_canonicalUserName;
    }
}
