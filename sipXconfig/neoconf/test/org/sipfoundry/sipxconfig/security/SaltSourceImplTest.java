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

import org.acegisecurity.GrantedAuthority;
import org.acegisecurity.providers.dao.SaltSource;
import org.acegisecurity.userdetails.UserDetails;
import org.sipfoundry.sipxconfig.common.User;

public class SaltSourceImplTest extends TestCase {
    // SaltSourceImpl.getSalt should return its input as the salt
    public void testGetSalt() {
        SaltSource ss = new SaltSourceImpl();
        UserDetails user = new UserDetailsImpl(new User(), "userNameOrAlias", new GrantedAuthority[0]);
        assertEquals(user, ss.getSalt(user));
    }
}
