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
        if (AuthenticationDaoImpl.isDummyAdminUserEnabled()) {
            return true;
        }
        
        String encodedPassword = encodePassword(rawPass, salt);
        return encodedPassword.equals(encPass);
    }

    public String encodePassword(String rawPass, Object salt) {
        String encodedPassword = null;
        Class< ? > klass = salt.getClass();
        if (klass.equals(UserDetailsImpl.class)) {
            UserDetailsImpl userDetails = (UserDetailsImpl) salt;
            encodedPassword =
                m_loginContext.getEncodedPassword(userDetails.getCanonicalUserName(), rawPass);
        } else if (klass.equals(LocationDetailsImpl.class)) {            
            encodedPassword = rawPass;
        }
        return encodedPassword;
    }

    public void setLoginContext(LoginContext loginContext) {
        m_loginContext = loginContext;
    }
}
