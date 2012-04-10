/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.cfgmgt;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;

import java.io.File;
import java.util.Arrays;
import java.util.Collection;
import java.util.Set;

import org.junit.Test;
import org.sipfoundry.sipxconfig.commserver.Location;

public class RegisteredLocationResolverTest {
    private File m_lastSeen = new File(getClass().getResource("lastseen.csv").getFile());
    
    @Test
    public void loadRegisteredIps() {
        RegisteredLocationResolver resolver = new RegisteredLocationResolver(null, null, m_lastSeen);
        Set<String> actual = resolver.loadRegisteredIps();
        assertEquals(2, actual.size());
        assertEquals("[192.168.1.112, 127.0.0.1]", actual.toString());
    }

    @Test
    public void getRegisteredLocations() {
        ConfigCommands configManager = createMock(ConfigCommands.class);
        configManager.lastSeen();
        expectLastCall().times(2);
        replay(configManager);
        RegisteredLocationResolver resolver = new RegisteredLocationResolver(configManager, null, m_lastSeen);
        Location l1 = new Location("one", "1.1.1.1");
        Location l2 = new Location("two", "2.2.2.2");
        Location l3 = new Location("three", "192.168.1.112");
        Collection<Location> actual = resolver.getRegisteredLocations(Arrays.asList(l1, l2));
        assertEquals(0, actual.size());        
        assertEquals("[192.168.1.112, 127.0.0.1]", resolver.getRegisteredIps().toString());
        
        actual = resolver.getRegisteredLocations(Arrays.asList(l1, l2, l3));
        assertEquals(1, actual.size());
        assertEquals("[192.168.1.112, 127.0.0.1]", resolver.getRegisteredIps().toString());
        
        verify(configManager);
    }
}
