/*
 *
 *
 * Copyright (C) 2008-2009 Nortel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.commons.jainsip;

import gov.nist.javax.sip.clientauthutils.SecureAccountManager;
import gov.nist.javax.sip.clientauthutils.UserCredentialHash;

import javax.sip.ClientTransaction;
import javax.sip.address.SipURI;
import javax.sip.header.FromHeader;
import javax.sip.message.Request;

import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;

public abstract class AbstractAccountManager implements SecureAccountManager {

    ValidUsersXML validUsers;
 
    public AbstractAccountManager() throws Exception {
        validUsers = ValidUsersXML.update(null, true);
      
    }

    public UserCredentialHash getCredentialHash(String userName) {
        try {
            validUsers = ValidUsersXML.update(null, true);
            User user = validUsers.getUser(userName);
            if (user == null) {
                return null;
            } else {
                return new UserCredentialsImpl(user);
            }
        } catch (Exception ex) {
            return null;
        }
    }

    public User getUser(String userName) {
        try {
            validUsers = ValidUsersXML.update(null, true);
            return validUsers.getUser(userName);
        } catch (Exception ex) {
            return null;
        }
    }

   
    public String getIdentity(String userName) {
        User user = validUsers.getUser(userName);
        if (user == null) {
            return null;
        } else {
            return user.getIdentity();
        }
    }

    @Override
    public UserCredentialHash getCredentialHash(ClientTransaction clientTransaction, String realm) {
        Request request = clientTransaction.getRequest();
        FromHeader from = (FromHeader) request.getHeader(FromHeader.NAME);
        String fromUser  = ((SipURI)from.getAddress().getURI()).getUser();
        return this.getCredentialHash(fromUser);
    }

}
