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

import javax.sip.ClientTransaction;

import gov.nist.javax.sip.clientauthutils.AccountManager;
import gov.nist.javax.sip.clientauthutils.UserCredentials;

class AccountManagerImpl implements AccountManager {

    public AccountManagerImpl() {
    }

    public UserCredentials getCredentials(ClientTransaction clientTransaction, String realm) {
        Object applicationData = clientTransaction.getApplicationData();
        if (applicationData instanceof TransactionApplicationData) {
            TransactionApplicationData tad = (TransactionApplicationData) applicationData;
            return tad.getUserCredentials();
        }
        return null;
    }
}
