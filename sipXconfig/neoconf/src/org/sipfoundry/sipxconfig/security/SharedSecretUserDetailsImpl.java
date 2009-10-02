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
import org.sipfoundry.sipxconfig.common.Md5Encoder;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class SharedSecretUserDetailsImpl extends UserDetailsImpl {
    private final DomainManager m_domainManager;

    /**
     * UserDetails constructor
     *
     * Create an Acegi Security UserDetails object based on the sipXconfig User, the
     * userNameOrAlias that is the userName part of the user's credentials, and the authorities
     * granted to this user.
     */
    public SharedSecretUserDetailsImpl(DomainManager domainManager, User user, String userNameOrAlias,
            GrantedAuthority... authorities) {
        super(user, userNameOrAlias, authorities);
        m_domainManager = domainManager;
    }

    /** Return the MD5-encoded SHARED-SECRET as password */
    @Override
    public String getPassword() {
        String sharedSecret = m_domainManager.getSharedSecret();
        return Md5Encoder.digestPassword(getUsername(), m_domainManager.getAuthorizationRealm(), sharedSecret);
    }
}
