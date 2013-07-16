/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.security;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UsernameNotFoundException;

public class LocationUserServiceTest extends TestCase {
    public void testLoadUserByUsername() {
        Location l = new Location();
        l.setPassword("12345");
        l.setFqdn("primary.example.com");

        LocationsManager lm = createMock(LocationsManager.class);
        lm.getLocationByFqdn("primary.example.com");
        expectLastCall().andReturn(l);

        replay(lm);

        LocationUserService lus = new LocationUserService();
        lus.setLocationsManager(lm);

        UserDetails details = lus.loadUserByUsername("primary.example.com");

        assertEquals("primary.example.com", details.getUsername());
        assertEquals("12345", details.getPassword());

        verify(lm);
    }

    public void testLoadUserByUsernameNoUser() {
        LocationsManager lm = createMock(LocationsManager.class);
        lm.getLocationByFqdn("primary.example.com");
        expectLastCall().andReturn(null);

        replay(lm);

        LocationUserService lus = new LocationUserService();
        lus.setLocationsManager(lm);

        try {
            lus.loadUserByUsername("primary.example.com");
            fail("Should throw authorization exception");
        } catch (UsernameNotFoundException e) {
            assertTrue(e.getMessage().contains("primary.example.com"));
        }
        verify(lm);
    }
}
