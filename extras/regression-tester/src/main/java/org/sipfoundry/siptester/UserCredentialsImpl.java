/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.siptester;

import gov.nist.javax.sip.clientauthutils.UserCredentialHash;
import gov.nist.javax.sip.clientauthutils.UserCredentials;

public class UserCredentialsImpl implements UserCredentials {
    private String userName;
    private String password;
    
    public UserCredentialsImpl(String userName, String password ) {
        this.userName = userName;
        this.password = password;
    }

   

    @Override
    public String getSipDomain() {
       return SipTester.getTesterConfig().getSipxProxyDomain();
    }

    @Override
    public String getUserName() {
        return userName;
    }

    public void setUserName(String userName) {
        this.userName = userName;
    }


    /**
     * @param password the password to set
     */
    public void setPassword(String password) {
        this.password = password;
    }



    /**
     * @return the password
     */
    public String getPassword() {
        return password;
    }

}
