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

import org.sipfoundry.sipxconfig.admin.commserver.Location;

import junit.framework.TestCase;

import org.acegisecurity.GrantedAuthority;
import org.acegisecurity.providers.dao.SaltSource;
import org.acegisecurity.userdetails.UserDetails;
import org.sipfoundry.sipxconfig.common.User;

public class SaltSourceImplTest extends TestCase {
    public void testGetSalt() {
        SaltSource ss = new SaltSourceImpl();
        assertNull(ss.getSalt(new LocationDetailsImpl(new Location())));

        User u = new User();
        u.setUserName("bongo");
        UserDetails user = new UserDetailsImpl(u, "userNameOrAlias", new GrantedAuthority[0]);
        assertEquals("bongo", ss.getSalt(user));
    }
}
