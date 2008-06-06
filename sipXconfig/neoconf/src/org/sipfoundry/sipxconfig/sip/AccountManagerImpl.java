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

public class AccountManagerImpl implements AccountManager {

    private UserCredentialsImpl m_userCredenitals;
    
    
    public AccountManagerImpl(UserCredentialsImpl userCredentials) {
        this.m_userCredenitals = userCredentials;
    }
  
    public UserCredentials getCredentials(SipURI sipUri, String realm) {     
        return m_userCredenitals;
    }

}
