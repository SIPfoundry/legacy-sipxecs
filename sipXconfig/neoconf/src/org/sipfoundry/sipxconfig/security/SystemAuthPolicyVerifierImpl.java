/**
 *
 *
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.security;

import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.security.authentication.AuthenticationServiceException;

public class SystemAuthPolicyVerifierImpl implements SystemAuthPolicyVerifier {

    private LdapManager m_ldapManager;

    @Override
    public void verifyPolicy(String username) throws AuthenticationServiceException {
        LdapSystemSettings settings = m_ldapManager.getSystemSettings();
        if (settings.isConfigured() && settings.isLdapOnly()) {
            throw new AuthenticationServiceException(
                "Only LDAP authentication is permitted");
        }
    }

    @Required
    public void setLdapManager(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
    }

    public LdapManager getLdapManager() {
        return m_ldapManager;
    }

    @Override
    public boolean isExternalXmppAuthOnly() {
        LdapSystemSettings ldapSettings = m_ldapManager.getSystemSettings();
        return ldapSettings.isEnableOpenfireConfiguration() && ldapSettings.isConfigured();
    }
}
