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

import java.util.Arrays;
import org.sipfoundry.sipxconfig.service.SipxAcdService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxService;

import junit.framework.TestCase;

public class RestartNeededStateTest extends TestCase {

    public void testMark() {
        SipxService proxy = new SipxProxyService();
        proxy.setBeanId(SipxProxyService.BEAN_ID);
        SipxAcdService acd = new SipxAcdService();
        acd.setBeanId(SipxAcdService.BEAN_ID);

        Location l1 = new Location();
        l1.setFqdn("l1.example.org");
        Location l2 = new Location();
        l2.setFqdn("l2.example.org");

        RestartNeededState state = new RestartNeededState();
        assertTrue(state.isEmpty());

        assertFalse(state.isMarked(l1, proxy));
        assertFalse(state.isMarked(l1, acd));
        assertFalse(state.isMarked(l2, proxy));
        assertFalse(state.isMarked(l2, acd));

        state.mark(l1, proxy);
        state.mark(l2, acd);

        assertTrue(state.isMarked(l1, proxy));
        assertFalse(state.isMarked(l1, acd));
        assertFalse(state.isMarked(l2, proxy));
        assertTrue(state.isMarked(l2, acd));

        assertFalse(state.isEmpty());

        state.mark(l1, acd);
        state.mark(l2, acd);

        assertTrue(state.isMarked(l1, proxy));
        assertTrue(state.isMarked(l1, acd));
        assertFalse(state.isMarked(l2, proxy));
        assertTrue(state.isMarked(l2, acd));

        state.unmark(l1, acd);
        state.unmark(l1, proxy);

        assertFalse(state.isMarked(l1, proxy));
        assertFalse(state.isMarked(l1, acd));
        assertFalse(state.isMarked(l2, proxy));
        assertTrue(state.isMarked(l2, acd));

        state.unmark(l1, acd);
        state.unmark(l2, acd);

        assertTrue(state.isEmpty());

        assertFalse(state.isMarked(l1, proxy));
        assertFalse(state.isMarked(l1, acd));
        assertFalse(state.isMarked(l2, proxy));
        assertFalse(state.isMarked(l2, acd));
    }

    public void testMarkMany() {
        SipxService proxy = new SipxProxyService();
        proxy.setBeanId(SipxProxyService.BEAN_ID);
        SipxAcdService acd = new SipxAcdService();
        acd.setBeanId(SipxAcdService.BEAN_ID);

        Location l1 = new Location();
        l1.setFqdn("l1.example.org");

        RestartNeededState state = new RestartNeededState();
        state.mark(l1, Arrays.asList(proxy, acd));

        assertFalse(state.isEmpty());
        assertTrue(state.isMarked(l1, proxy));
        assertTrue(state.isMarked(l1, acd));

        state.unmark(l1, Arrays.asList(proxy, acd));
        assertTrue(state.isEmpty());
        assertFalse(state.isMarked(l1, proxy));
        assertFalse(state.isMarked(l1, acd));
    }

}
