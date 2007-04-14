/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Arrays;
import java.util.Collection;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContextImpl.EventsToServices;

public class EventsToServicesTest extends TestCase {

    public void testAddServicesEmpty() {
        EventsToServices ets = new EventsToServices();
        Collection services = ets.getServices(Object.class);
        assertEquals(0, services.size());
    }

    public void testAddServicesOneClass() {
        EventsToServices ets = new EventsToServices();
        String[] onInteger = {
            "a", "b", "c"
        };
        ets.addServices(Arrays.asList(onInteger), Integer.class);

        Collection services = ets.getServices(Object.class);
        assertEquals(0, services.size());
    }

    public void testAddServicesManyClasses() {
        EventsToServices ets = new EventsToServices();
        String[] onInteger = {
            "a", "b", "c"
        };
        String[] onObject = {
            "a", "x", "y"
        };

        ets.addServices(Arrays.asList(onInteger), Integer.class);
        ets.addServices(Arrays.asList(onObject), Object.class);

        Collection services = ets.getServices(Object.class);
        assertEquals(3, services.size());
        assertFalse(services.contains("b"));
        
        services = ets.getServices(Integer.class);
        // 5 - 3 two remaining services should be here
        assertEquals(2, services.size());
        assertTrue(services.contains("b"));
    }
}
