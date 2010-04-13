/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Collection;
import java.util.Collections;

import static java.util.Arrays.asList;

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.service.SipxService;

public class LocationStatusTest extends TestCase {

    public void testGetToBeReplicatedEmpty() {
        Collection<SipxService> started = Collections.emptySet();
        Collection<SipxService> stopped = Collections.emptySet();
        LocationStatus ls = new LocationStatus(started, stopped);
        assertTrue(ls.getToBeReplicated().isEmpty());
    }

    public void testGetToBeReplicated() {
        SipxService s1 = new DummyService("1");
        SipxService s2 = new DummyService("2");
        SipxService s3 = new DummyService("3");
        SipxService s4 = new DummyService("4");
        SipxService s5 = new DummyService("5");
        SipxService s6 = new DummyService("6");

        Collection<SipxService> started = asList(s1, s2);
        Collection<SipxService> stopped = asList(s4, s5);
        LocationStatus ls = new LocationStatus(started, stopped);

        Collection<SipxService> tbr = ls.getToBeReplicated();
        assertEquals(2, tbr.size());
        assertTrue(tbr.contains(s1));
        assertTrue(tbr.contains(s2));

        s1.setAffectedServices(asList(s3, s4));
        tbr = ls.getToBeReplicated();
        assertEquals(3, tbr.size());
        assertTrue(tbr.contains(s1));
        assertTrue(tbr.contains(s2));
        assertTrue(tbr.contains(s3));

        s4.setAffectedServices(asList(s1, s2, s5, s6));
        tbr = ls.getToBeReplicated();
        assertEquals(4, tbr.size());
        assertTrue(tbr.contains(s1));
        assertTrue(tbr.contains(s2));
        assertTrue(tbr.contains(s3));
        assertTrue(tbr.contains(s6));
    }

    private static class DummyService extends SipxService {
        public DummyService(String beanId) {
            setBeanId(beanId);
        }
    }
}
