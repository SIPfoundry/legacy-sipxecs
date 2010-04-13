/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.security;

import org.acegisecurity.providers.encoding.PasswordEncoder;
import org.sipfoundry.sipxconfig.login.LoginContext;
import org.springframework.beans.factory.annotation.Required;

public class PasswordEncoderImpl implements PasswordEncoder {
    private LoginContext m_loginContext;

    public boolean isPasswordValid(String encPass, String rawPass, Object salt) {
        if (rawPass == null) {
            return false;
        }
        String pass = encodePassword(rawPass, salt);
        return pass.equals(encPass);
    }

    public String encodePassword(String rawPass, Object salt) {
        if (salt instanceof String) {
            return m_loginContext.getEncodedPassword((String) salt, rawPass);
        }
        return rawPass;
    }

    @Required
    public void setLoginContext(LoginContext loginContext) {
        m_loginContext = loginContext;
    }
}
