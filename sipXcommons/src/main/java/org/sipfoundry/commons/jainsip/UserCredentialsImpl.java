/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.commons.jainsip;

import org.sipfoundry.commons.userdb.User;

import gov.nist.javax.sip.clientauthutils.UserCredentialHash;


public class UserCredentialsImpl implements UserCredentialHash {
   
    User user;
    String userName;
    private String sipDomain;

    public UserCredentialsImpl(User user) {
       String[] nameDomain = user.getIdentity().split("@");
       userName = nameDomain[0];     
       this.user = user;
       this.sipDomain = nameDomain[1];
    }

    public String getSipDomain() {
        return sipDomain;
    }

    public String getUserName() {
       return this.userName;
    }
    
    

    public String getHashUserDomainPassword() {
        return user.getPasstoken();
    }


   
   
}
