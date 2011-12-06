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
import java.util.Collection;

import org.sipfoundry.sipxconfig.service.SipxAcdService;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxService;

import junit.framework.TestCase;

public class RestartNeededStateTest extends TestCase {

    private SipxService m_proxy;
    private SipxAcdService m_acd;
    private Location m_l1;
    private Location m_l2;

    @Override
    protected void setUp() throws Exception {
        m_proxy = new SipxProxyService();
        m_proxy.setBeanId(SipxProxyService.BEAN_ID);
        m_acd = new SipxAcdService();
        m_acd.setBeanId(SipxAcdService.BEAN_ID);

        m_l1 = new Location();
        m_l1.setFqdn("l1.example.org");

        m_l2 = new Location();
        m_l2.setFqdn("l2.example.org");
    }

    public void testMark() {
        RestartNeededState state = new RestartNeededState();
        assertTrue(state.isEmpty());

        assertFalse(state.isMarked(m_l1, m_proxy));
        assertFalse(state.isMarked(m_l1, m_acd));
        assertFalse(state.isMarked(m_l2, m_proxy));
        assertFalse(state.isMarked(m_l2, m_acd));

        state.mark(m_l1, m_proxy);
        state.mark(m_l2, m_acd);

        assertTrue(state.isMarked(m_l1, m_proxy));
        assertFalse(state.isMarked(m_l1, m_acd));
        assertFalse(state.isMarked(m_l2, m_proxy));
        assertTrue(state.isMarked(m_l2, m_acd));

        assertFalse(state.isEmpty());

        state.mark(m_l1, m_acd);
        state.mark(m_l2, m_acd);

        assertTrue(state.isMarked(m_l1, m_proxy));
        assertTrue(state.isMarked(m_l1, m_acd));
        assertFalse(state.isMarked(m_l2, m_proxy));
        assertTrue(state.isMarked(m_l2, m_acd));

        state.unmark(m_l1, m_acd);
        state.unmark(m_l1, m_proxy);

        assertFalse(state.isMarked(m_l1, m_proxy));
        assertFalse(state.isMarked(m_l1, m_acd));
        assertFalse(state.isMarked(m_l2, m_proxy));
        assertTrue(state.isMarked(m_l2, m_acd));

        state.unmark(m_l1, m_acd);
        state.unmark(m_l2, m_acd);

        assertTrue(state.isEmpty());

        assertFalse(state.isMarked(m_l1, m_proxy));
        assertFalse(state.isMarked(m_l1, m_acd));
        assertFalse(state.isMarked(m_l2, m_proxy));
        assertFalse(state.isMarked(m_l2, m_acd));
    }

    public void testMarkMany() {
        RestartNeededState state = new RestartNeededState();
        state.mark(m_l1, Arrays.asList(m_proxy, m_acd));

        assertFalse(state.isEmpty());
        assertTrue(state.isMarked(m_l1, m_proxy));
        assertTrue(state.isMarked(m_l1, m_acd));

        state.unmark(m_l1, Arrays.asList(m_proxy, m_acd));
        assertTrue(state.isEmpty());
        assertFalse(state.isMarked(m_l1, m_proxy));
        assertFalse(state.isMarked(m_l1, m_acd));
    }

    public void testGetAffectedLocation() {
        RestartNeededState state = new RestartNeededState();
        Collection<RestartNeededService> affected = state.getAffected();
        assertTrue(affected.isEmpty());

        state.mark(m_l1, Arrays.asList(m_proxy, m_acd));
        affected = state.getAffected();
        assertEquals(2, affected.size());

        state.mark(m_l2, Arrays.asList(m_proxy));

        affected = state.getAffected();
        assertEquals(3, affected.size());
    }

    public void testUnmark() {
        RestartNeededState state = new RestartNeededState();
        state.mark(m_l1, Arrays.asList(m_proxy));
        assertFalse(state.isEmpty());

        state.unmark(m_l1, m_proxy);
        assertTrue(state.isEmpty());
    }
}
