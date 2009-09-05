/*
 *
 *
 * Copyright (C) 2008 Nortel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.restServer;

import gov.nist.javax.sip.clientauthutils.SecureAccountManager;
import gov.nist.javax.sip.clientauthutils.UserCredentialHash;

import javax.sip.ClientTransaction;

import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;


public class AccountManagerImpl implements SecureAccountManager {

    ValidUsersXML validUsers ;
    
    public AccountManagerImpl() throws Exception {
        validUsers = ValidUsersXML.update(true);          
    }
    
    public UserCredentialHash getCredentialHash(String userName) {
       User user = validUsers.getUser(userName);
       if ( user == null ) {
           return null;
       } else {
           return new UserCredentialsImpl(user);
       }
    }
    
    public User getUser(String userName) {
        return validUsers.getUser(userName);
    }
    
    public String getIdentity(String userName) {
        User user = validUsers.getUser(userName);
        if ( user == null ) {
            return null;
        } else {
            return user.getIdentity();
        }
    }
   

    public UserCredentialHash getCredentialHash(ClientTransaction clientTransaction, String realm) {
        Object applicationData = clientTransaction.getApplicationData();
//        if (applicationData instanceof TransactionApplicationData) {
//            TransactionApplicationData tad = (TransactionApplicationData) applicationData;
//            return tad.getUserCredentials();
//        }
        return null;
    }

    

   
}
