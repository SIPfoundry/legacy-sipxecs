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

import org.acegisecurity.userdetails.UserDetails;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionName;

public class AuthenticationDaoImplTest extends TestCase {
    private static final String USER_NAME = "Hjelje";
    private static User s_user = null;
    
    private IMocksControl m_coreControl;
    private CoreContext m_coreContext;
    private AuthenticationDaoImpl m_authenticationDaoImpl;

    protected void setUp() throws Exception {
        m_coreControl = EasyMock.createStrictControl();
        m_coreContext = m_coreControl.createMock(CoreContext.class);
        m_authenticationDaoImpl = new AuthenticationDaoImpl();
        m_authenticationDaoImpl.setCoreContext(m_coreContext);
        s_user = new User() {
            public boolean hasPermission(PermissionName permission) {
                return true;
            }
        };
        s_user.setUserName(USER_NAME);
    }
    
    public void testLoadUserByUsername() {
        // prep the mock core context
        m_coreContext.loadUserByUserNameOrAlias(USER_NAME);
        m_coreControl.andReturn(s_user);
        m_coreControl.replay();
        
        // load the user details
        UserDetails details = m_authenticationDaoImpl.loadUserByUsername(USER_NAME);
        assertEquals(USER_NAME, details.getUsername());
        assertEquals(AuthenticationDaoImpl.AUTH_USER_AND_ADMIN_ARRAY, details.getAuthorities());
        m_coreControl.verify();
    }
}
