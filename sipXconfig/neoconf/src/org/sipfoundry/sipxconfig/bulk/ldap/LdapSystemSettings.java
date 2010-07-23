/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.enums.Enum;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.EnumUserType;

/**
 * System wide settings for ldap support within sipXecs
 */
public class LdapSystemSettings extends BeanWithId {
    private AuthenticationOptions m_authenticationOptions = AuthenticationOptions.NO_LDAP;
    private boolean m_enableOpenfire;

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
}
