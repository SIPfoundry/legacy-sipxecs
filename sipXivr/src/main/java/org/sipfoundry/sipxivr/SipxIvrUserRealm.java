/*
 * Copyright (c) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

package org.sipfoundry.sipxivr;

import java.security.Principal;

import org.apache.commons.codec.digest.DigestUtils;
import org.apache.log4j.Logger;
import org.mortbay.http.HttpRequest;
import org.mortbay.http.UserRealm;
import org.mortbay.util.Credential;
import org.mortbay.util.Password;
import org.sipfoundry.commons.userdb.User;
import org.sipfoundry.commons.userdb.ValidUsersXML;

public class SipxIvrUserRealm implements UserRealm {

    static final Logger LOG = Logger.getLogger("org.sipfoundry.sipxivr");

    private String m_name;
    private String m_sharedSecret;

    public SipxIvrUserRealm(String realmName, String sharedSecret) {
        m_name = realmName;
        m_sharedSecret = sharedSecret;
    }

    @Override
    public Principal authenticate(String username, Object credentials, HttpRequest request) {
        Principal principal = null;
        try {
            ValidUsersXML validUsers = ValidUsersXML.update(LOG, true);
            User user = validUsers.getUser(username);
            if (user != null) {
                principal = checkCredentials(user.getUserName(), user.getPintoken(), credentials);
                if (principal == null) {
                    // 2nd try with shared secret
                    String hashedSharedSecret = digestPassword(user.getUserName(), m_sharedSecret);
                    principal = checkCredentials(user.getUserName(), hashedSharedSecret, credentials);
                }
            }
        } catch (Exception ex) {
            LOG.warn("Could not authenticate user " + username);
        }
        return principal;
    }

    private Principal checkCredentials(String userName, String pintoken, Object credentials) {
        Credential password = new Password(pintoken);
        if (password.check(credentials)) {
            return new SipxIvrPrincipal(userName, "IvrRole");
        }
        return null;
    }

    private String digestPassword(String user, String password) {
        String full = user + ':' + m_name + ':' + password;
        String digest = DigestUtils.md5Hex(full);
        return digest;
    }

    @Override
    public void disassociate(Principal arg0) {
    }

    @Override
    public String getName() {
        return m_name;
    }

    @Override
    public Principal getPrincipal(String arg0) {
        return null;
    }

    @Override
    public boolean isUserInRole(Principal user, String role) {
        return ((SipxIvrPrincipal) user).getRole().equals(role);
    }

    @Override
    public void logout(Principal user) {
    }

    @Override
    public Principal popRole(Principal user) {
        return null;
    }

    @Override
    public Principal pushRole(Principal user, String role) {
        return null;
    }

    @Override
    public boolean reauthenticate(Principal user) {
        return ((SipxIvrPrincipal) user).isAuthenticated();
    }

    private class SipxIvrPrincipal implements Principal {
        private String m_userName;
        private String m_role;

        SipxIvrPrincipal(String name, String role) {
            m_userName = name;
            m_role = role;
        }

        public String getName() {
            return m_userName;
        }

        public boolean isAuthenticated() {
            return true;
        }

        public String getRole() {
            return m_role;
        }
    }

}
