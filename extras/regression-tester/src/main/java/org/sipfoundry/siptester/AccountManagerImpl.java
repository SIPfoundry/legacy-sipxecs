/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.siptester;

import gov.nist.javax.sip.clientauthutils.AccountManager;
import gov.nist.javax.sip.clientauthutils.UserCredentialHash;
import gov.nist.javax.sip.clientauthutils.UserCredentials;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.HashMap;

import javax.sip.ClientTransaction;
import javax.sip.address.SipURI;
import javax.sip.header.FromHeader;
import javax.sip.message.Request;

import org.apache.log4j.Logger;
import org.sipfoundry.commons.jainsip.AbstractAccountManager;



public class AccountManagerImpl implements AccountManager {
    
    
    private static final Logger logger = Logger.getLogger(AccountManagerImpl.class);
    HashMap<String, UserCredentials> plainTextCredentials = new HashMap<String, UserCredentials>();
    
    
    public AccountManagerImpl() throws Exception {
        super();
    }

    /**
     * Add a plain text password account to this account manager. Clear text sip passwords
     * are made available for users with special IDs such as ~~id~callWatcher. These
     * are not part of the validuser.xml database and must be provided to the service
     * by other means ( configuration files ).
     * 
     * @param userName
     * @param realm
     * @param password
     */
    public void addAccount(String userName, String password) {
        UserCredentials credHash = new UserCredentialsImpl(userName, password);
        this.plainTextCredentials.put(userName, credHash);

    }

    public void addAccount(UserCredentials userCredentials) {
        this.plainTextCredentials.put(userCredentials.getUserName(), userCredentials);
    }
  
    @Override
    public UserCredentials getCredentials(ClientTransaction challengedTransaction, String realm) {
        Request request = challengedTransaction.getRequest();
        FromHeader from = (FromHeader) request.getHeader(FromHeader.NAME);
        String fromUser = ((SipURI) from.getAddress().getURI()).getUser();
        return this.plainTextCredentials.get(fromUser);
    }
}
