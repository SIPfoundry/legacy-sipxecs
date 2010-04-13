/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.sip;

import gov.nist.javax.sip.clientauthutils.UserCredentials;

import org.sipfoundry.sipxconfig.common.User;

class UserCredentialsImpl implements UserCredentials {
    private final String m_password;
    private final String m_realm;
    private final String m_userName;

    public UserCredentialsImpl(User user, String realm) {
        m_userName = user.getUserName();
        m_password = user.getSipPassword();
        m_realm = realm;
    }

    public String getPassword() {
        return m_password;
    }

    public String getSipDomain() {
        return m_realm;
    }

    public String getUserName() {
        return m_userName;
    }
}
