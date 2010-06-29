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

import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * System wide settings for ldap support within sipXecs
 */
public class LdapSystemSettings extends BeanWithId {
    private boolean m_enableWebAuthentication;
    private boolean m_enableOpenfire;

    public boolean isEnableOpenfireConfiguration() {
        return m_enableOpenfire;
    }

    public void setEnableOpenfireConfiguration(boolean enableOpenfire) {
        m_enableOpenfire = enableOpenfire;
    }

    public boolean isEnableWebAuthentication() {
        return m_enableWebAuthentication;
    }

    public void setEnableWebAuthentication(boolean useForWebAuthentication) {
        m_enableWebAuthentication = useForWebAuthentication;
    }
}
