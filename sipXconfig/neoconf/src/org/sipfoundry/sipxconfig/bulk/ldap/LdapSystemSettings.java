/**
 *
 *
 * Copyright (c) 2010 / 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.Collection;
import java.util.Collections;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.EnumUserType;
import org.sipfoundry.sipxconfig.feature.Feature;

/**
 * System wide settings for ldap support within sipXecs
 */
public class LdapSystemSettings extends BeanWithId implements DeployConfigOnEdit {
    private AuthenticationOptions m_authenticationOptions = AuthenticationOptions.NO_LDAP;
    private boolean m_enableOpenfire;
    private boolean m_configured;

    public boolean isEnableOpenfireConfiguration() {
        return m_enableOpenfire;
    }

    public void setEnableOpenfireConfiguration(boolean enableOpenfire) {
        m_enableOpenfire = enableOpenfire;
    }

    public AuthenticationOptions getAuthenticationOptions() {
        return m_authenticationOptions;
    }

    public boolean isLdapEnabled() {
        return isLdapOnly() || ObjectUtils.equals(m_authenticationOptions, AuthenticationOptions.PIN_LDAP);
    }

    public boolean isLdapOnly() {
        return ObjectUtils.equals(m_authenticationOptions, AuthenticationOptions.LDAP);
    }

    public void setAuthenticationOptions(AuthenticationOptions authenticationOptions) {
        m_authenticationOptions = authenticationOptions;
    }

    public boolean isConfigured() {
        return m_configured;
    }

    public void setConfigured(boolean configured) {
        m_configured = configured;
    }

    /**
     * Authentication options enumeration
     */
    public static final class AuthenticationOptions extends Enum {
        public static final AuthenticationOptions NO_LDAP = new AuthenticationOptions("noLDAP");
        public static final AuthenticationOptions LDAP = new AuthenticationOptions("LDAP");
        public static final AuthenticationOptions PIN_LDAP = new AuthenticationOptions("pinLDAP");

        public AuthenticationOptions(String name) {
            super(name);
        }

        public static AuthenticationOptions getEnum(String type) {
            return (AuthenticationOptions) getEnum(AuthenticationOptions.class, type);
        }
    }

    /**
     * Used for Hibernate type translation
     */
    public static class UserType extends EnumUserType {
        public UserType() {
            super(AuthenticationOptions.class);
        }
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) LdapManager.FEATURE);
    }
}
