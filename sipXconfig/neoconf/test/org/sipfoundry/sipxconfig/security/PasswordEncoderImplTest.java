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

import junit.framework.TestCase;

import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.login.LoginContext;

public class PasswordEncoderImplTest extends TestCase {
    private static final String USER_NAME = "angelina";
    private static final String USER_ALIAS = "lara";
    private static final String RAW_PASSWORD = "croft";
    // dummy result based on userName and password
    private static final String ENCODED_PASSWORD = USER_NAME + RAW_PASSWORD;
    private IMocksControl m_loginControl;
    private LoginContext m_loginContext;
    private PasswordEncoderImpl m_passwordEncoder;
    private UserDetailsImpl m_userDetails;

    protected void setUp() throws Exception {
        m_loginControl = org.easymock.classextension.EasyMock.createStrictControl();
        m_loginContext = m_loginControl.createMock(LoginContext.class);
        m_passwordEncoder = new PasswordEncoderImpl();
        m_passwordEncoder.setLoginContext(m_loginContext);
        
        final User user = new User();
        user.setUserName(USER_NAME);
        m_userDetails = new UserDetailsImpl(user, USER_ALIAS, null);        
    }    
    
    public void testisPasswordValid() {
        setupSimulatedCallsToGetEncodedPassword(2);
        assertTrue(m_passwordEncoder.isPasswordValid(ENCODED_PASSWORD, RAW_PASSWORD, m_userDetails));
        assertFalse(m_passwordEncoder.isPasswordValid("bad encoded password", RAW_PASSWORD, m_userDetails));
        m_loginControl.verify();
    }
    
    public void testEncodePassword() {
        setupSimulatedCallsToGetEncodedPassword(1);
        assertEquals(ENCODED_PASSWORD, m_passwordEncoder.encodePassword(RAW_PASSWORD, m_userDetails));
        m_loginControl.verify();
    }
    
    private void setupSimulatedCallsToGetEncodedPassword(int count) {
        for (int i = 0; i < count; i++) {
            m_loginContext.getEncodedPassword(USER_NAME, RAW_PASSWORD);
            m_loginControl.andReturn(ENCODED_PASSWORD);
        }
        m_loginControl.replay();
    }
}
