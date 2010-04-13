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

import java.util.List;

import org.acegisecurity.GrantedAuthority;
import org.acegisecurity.userdetails.UserDetails;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.springframework.beans.factory.annotation.Required;

/**
 * All services have access to SHARED_SECRET in domain_config sipXconfig accept BASIC and DIGEST
 * authentication with SHARED_SECRET as a password for all users
 */
public class SharedSecretUserDetailsService extends AbstractUserDetailsService {

    private DomainManager m_domainManager;

    @Override
    protected UserDetails createUserDetails(String userNameOrAlias, User user, List<GrantedAuthority> gas) {
        return new SharedSecretUserDetailsImpl(m_domainManager, user, userNameOrAlias, gas
                .toArray(new GrantedAuthority[gas.size()]));
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }
}
