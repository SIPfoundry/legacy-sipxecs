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

import javax.sip.address.SipURI;

import gov.nist.javax.sip.clientauthutils.AccountManager;
import gov.nist.javax.sip.clientauthutils.UserCredentials;

import org.sipfoundry.sipxconfig.common.User;

class AccountManagerImpl implements AccountManager {

    private UserCredentials m_userCredentials;

    public AccountManagerImpl(User user, String realm) {
        m_userCredentials = new UserCredentialsImpl(user, realm);
    }

    public UserCredentials getCredentials(SipURI sipUri, String realm) {
        return m_userCredentials;
    }
}
