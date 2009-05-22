/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.forwarding;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.callgroup.AbstractRing;
import org.sipfoundry.sipxconfig.common.User;

/**
 * CallSequenceTest
 */
public class CallSequenceTest extends TestCase {
    private User m_user;

    @Override
    protected void setUp() throws Exception {
        m_user = new User();
        m_user.setUserName("abc");
    }

    public void testGenerateAliases() {
        final int N = 7;
        List rings = new ArrayList(N);
        for (int i = 0; i < N; i++) {
            Ring ring = new Ring("2" + i, i, AbstractRing.Type.DELAYED, true);
            rings.add(ring);
        }
        // add empty ring - should not change anything
        rings.add(new Ring());

        CallSequence sequence = new CallSequence();
        sequence.setUser(m_user);
        sequence.setRings(rings);

        List aliases = sequence.generateAliases("sipfoundry.org");
        assertEquals(N, aliases.size());
        for (Iterator i = aliases.iterator(); i.hasNext();) {
            AliasMapping a = (AliasMapping) i.next();
            assertEquals("abc@sipfoundry.org", a.getIdentity());
            String contact = a.getContact();
            assertTrue(contact
                    .matches("<sip:\\d+@sipfoundry.org;sipx-noroute=Voicemail\\?expires=\\d+>;q=[01]\\.\\d+"));
        }
    }

    public void testGenerateXabledAliases() {
        final int N = 8;
        List ringsDisabled = new ArrayList(N);
        List ringsMixed = new ArrayList(N);
        for (int i = 0; i < N; i++) {
			boolean enabled = (i % 2) == 0;
            ringsMixed.add(new Ring("2" + i, i, Ring.Type.DELAYED, enabled));
            ringsDisabled.add(new Ring("2" + i, i, Ring.Type.DELAYED, false));
        }

        CallSequence sequence = new CallSequence();
        sequence.setUser(m_user);

        sequence.setRings(ringsDisabled);
        List aliases = sequence.generateAliases("sipfoundry.org");
        assertEquals(0, aliases.size());

        sequence.setRings(ringsMixed);
        aliases = sequence.generateAliases("sipfoundry.org");
        assertEquals(N/2, aliases.size());

        for (Iterator i = aliases.iterator(); i.hasNext();) {
            AliasMapping a = (AliasMapping) i.next();
            assertEquals("abc@sipfoundry.org", a.getIdentity());
            String contact = a.getContact();
            assertTrue(contact
                    .matches("<sip:\\d+@sipfoundry.org;sipx-noroute=Voicemail\\?expires=\\d+>;q=[01]\\.\\d+"));
        }
    }

    public void testGenerateAliasesEmpty() {
        CallSequence sequence = new CallSequence();
        sequence.setUser(m_user);
        List list = sequence.generateAliases("sipfoundry.org");
        assertEquals(0, list.size());
    }

    public void testMove() {
        CallSequence callSequence = new CallSequence();

        List calls = new ArrayList();

        Ring ring0 = new Ring("000", 40, Ring.Type.IMMEDIATE, true);
        Ring ring1 = new Ring("111", 40, Ring.Type.IMMEDIATE, true);
        Ring ring2 = new Ring("222", 40, Ring.Type.IMMEDIATE, true);

        calls.add(ring0.setUniqueId());
        calls.add(ring1.setUniqueId());
        calls.add(ring2.setUniqueId());

        callSequence.setRings(calls);
        assertTrue(callSequence.moveRingDown(ring0));
        assertTrue(callSequence.moveRingDown(ring0));
        assertTrue(callSequence.moveRingUp(ring2));
        calls = callSequence.getRings();

        assertSame(ring2, calls.get(0));
        assertSame(ring1, calls.get(1));
        assertSame(ring0, calls.get(2));
    }
}
