/*
 *
 *
 * Copyright (C) 2010 eZuce, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

package org.sipfoundry.sipxconfig.admin.commserver;

import java.util.Arrays;
import java.util.Collection;

import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;

import junit.framework.TestCase;

public class ReloadNeededStateTest extends TestCase {

    private SipxFreeswitchService m_freeswitch;
    private Location m_l1;

    @Override
    protected void setUp() throws Exception {
        m_freeswitch = new SipxFreeswitchService();
        m_freeswitch.setBeanId(SipxFreeswitchService.BEAN_ID);

        m_l1 = new Location();
        m_l1.setFqdn("l1.example.org");
    }

    public void testMark() {
        ReloadNeededState state = new ReloadNeededState();
        assertTrue(state.isEmpty());

        assertFalse(state.isMarked(m_l1, m_freeswitch));

        state.mark(m_l1, m_freeswitch);

        assertTrue(state.isMarked(m_l1, m_freeswitch));
        assertFalse(state.isEmpty());

        state.unmark(m_l1, m_freeswitch);

        assertFalse(state.isMarked(m_l1, m_freeswitch));
        assertTrue(state.isEmpty());
        assertFalse(state.isMarked(m_l1, m_freeswitch));
    }

    public void testGetAffectedLocation() {
        ReloadNeededState state = new ReloadNeededState();
        Collection<ReloadNeededService> affected = state.getAffected();
        assertTrue(affected.isEmpty());

        state.mark(m_l1, m_freeswitch);
        affected = state.getAffected();
        assertEquals(1, affected.size());
    }

    public void testUnmark() {
        ReloadNeededState state = new ReloadNeededState();
        state.mark(m_l1, m_freeswitch);
        assertFalse(state.isEmpty());

        state.unmark(m_l1, m_freeswitch);
        assertTrue(state.isEmpty());
    }
}
