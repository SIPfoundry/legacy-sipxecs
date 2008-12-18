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
import org.sipfoundry.sipxconfig.service.SipxService;

public class EventsToServicesTest extends TestCase {

    public void testAddServicesEmpty() {
        EventsToServices ets = new EventsToServices();
        Collection services = ets.getServices(Object.class);
        assertEquals(0, services.size());
    }

    public void testAddServicesOneClass() {
        EventsToServices ets = new EventsToServices<String>();
        ets.addServices(Arrays.asList("a", "b", "c"), Integer.class);

        Collection<String> services = ets.getServices(Object.class);
        assertEquals(0, services.size());
    }

    public void testAddServicesForProcess() {
        EventsToServices<SipxService> ets = new EventsToServices<SipxService>();
        ets.addServices(Arrays.asList(new DummyService("a"), new DummyService("b"),
                new DummyService("c")), Integer.class);
        ets.addServices(Arrays.asList(new DummyService("a"), new DummyService("b")),
                Integer.class);

        Collection<SipxService> services = ets.getServices(Integer.class);
        assertEquals(3, services.size());

        // next time it should be empty
        services = ets.getServices(Integer.class);
        assertEquals(0, services.size());
    }

    public void testAddServicesManyClasses() {
        EventsToServices ets = new EventsToServices<String>();
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

        services = ets.getServices(Object.class);
        assertEquals(0, services.size());

        services = ets.getServices(Integer.class);
        // 5 - 3 two remaining services should be here
        assertEquals(2, services.size());
        assertTrue(services.contains("b"));
    }

    static class DummyService extends SipxService {
        private final String m_beanId;

        public DummyService(String beanId) {
            m_beanId = beanId;
        }

        @Override
        public String getBeanId() {
            return m_beanId;
        }

    }
}
