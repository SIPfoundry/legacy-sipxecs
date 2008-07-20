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

import gov.nist.javax.sip.clientauthutils.AccountManager;
import gov.nist.javax.sip.clientauthutils.UserCredentials;

import javax.sip.ClientTransaction;

import org.sipfoundry.sipxconfig.common.User;

class AccountManagerImpl implements AccountManager {

    private UserCredentials m_userCredentials;

    public AccountManagerImpl(User user, String realm) {
        m_userCredentials = new UserCredentialsImpl(user, realm);
    }

    public UserCredentials getCredentials(ClientTransaction clientTransaction, String realm) {
        TransactionApplicationData tad = (TransactionApplicationData) clientTransaction
                .getApplicationData();
        return tad.getUserCredentials();
    }
}
