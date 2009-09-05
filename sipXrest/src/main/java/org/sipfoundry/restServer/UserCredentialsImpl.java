/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.restServer;

import org.sipfoundry.commons.userdb.User;

import gov.nist.javax.sip.clientauthutils.UserCredentialHash;


class UserCredentialsImpl implements UserCredentialHash {
   
    User user;
    String userName;

    public UserCredentialsImpl(User user) {
       userName = user.getIdentity().split("@")[0];     
       this.user = user;
    }

    public String getSipDomain() {
       return RestServer.getRestServerConfig().getSipxProxyDomain();
    }

    public String getUserName() {
       return this.userName;
    }
    
    

    public String getHashUserDomainPassword() {
        return user.getPasstoken();
    }


   
   
}
