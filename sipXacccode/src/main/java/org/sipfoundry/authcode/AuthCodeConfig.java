/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.authcode;

import java.util.HashMap;

import org.sipfoundry.sipxacccode.ApplicationConfiguraton;

/**
 * Holds the configuration data needed for Authorization Codes.
 * 
 */
public class AuthCodeConfig extends ApplicationConfiguraton {
        private String m_authcode ;    // The actual authorization code.
        private String m_authname;     // The authorization name for this authorization code.
        private String m_authpassword; // The authorization password for the authorization code.

        public AuthCodeConfig() {
            super();
        }

        public String getAuthCode() {
            return m_authcode;
        }

        public void setAuthCode(String authcode) {
            m_authcode = authcode;
        }

        public String getAuthName() {
            return m_authname;
        }

        public void setAuthName(String authname) {
            m_authname = authname;
        }

        public String getAuthPassword() {
            return m_authpassword;
        }

        public void setAuthPassword(String authpassword) {
            m_authpassword = authpassword;
        }
}
