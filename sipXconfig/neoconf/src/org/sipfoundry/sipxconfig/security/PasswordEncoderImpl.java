/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.security;

import org.acegisecurity.providers.encoding.PasswordEncoder;
import org.sipfoundry.sipxconfig.login.LoginContext;

public class PasswordEncoderImpl implements PasswordEncoder {
    private LoginContext m_loginContext;

    public boolean isPasswordValid(String encPass, String rawPass, Object salt) {
        // dummy admin user is enabled only when running tests
        if (salt instanceof UserDetailsImpl
                && AuthenticationDaoImpl.allowDummyUser(getUserName(salt))) {
            return true;
        }

        String encodedPassword = encodePassword(rawPass, salt);
        return encodedPassword.equals(encPass);
    }

    public String encodePassword(String rawPass, Object salt) {
        if (salt instanceof LocationDetailsImpl) {
            return rawPass;
        } else if (salt instanceof UserDetailsImpl) {
            return m_loginContext.getEncodedPassword(getUserName(salt), rawPass);
        }
        return null;
    }

    public void setLoginContext(LoginContext loginContext) {
        m_loginContext = loginContext;
    }

    private String getUserName(Object salt) {
        UserDetailsImpl userDetails = (UserDetailsImpl) salt;
        return userDetails.getCanonicalUserName();
    }
}
