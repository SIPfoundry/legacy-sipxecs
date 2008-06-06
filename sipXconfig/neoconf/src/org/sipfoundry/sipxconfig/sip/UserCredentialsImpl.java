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

public class UserCredentialsImpl implements UserCredentials {

    private User m_user;

    private SipStackBean m_helper;

    public UserCredentialsImpl(User user, SipStackBean stackBean) {
        m_user = user;
        m_helper = stackBean;
    }

    public String getPassword() {
        return m_user.getSipPassword();
    }

    public String getSipDomain() {
        return m_helper.getHostName();
    }

    public String getUserName() {
        return m_user.getUserName();
    }

}
