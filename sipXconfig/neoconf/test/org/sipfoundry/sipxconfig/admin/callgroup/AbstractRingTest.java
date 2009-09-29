/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.callgroup;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.dialplan.ForkQueueValue;

public class AbstractRingTest extends TestCase {

    public void testCalculateContact() {
        AbstractRing ring = new RingMock("444");

        ForkQueueValue q = new ForkQueueValue(3);
        ring.setExpiration(45);
        ring.setType(AbstractRing.Type.IMMEDIATE);

        String contact = ring.calculateContact("sipfoundry.org", q, false, true, null);
        assertEquals("<sip:444@sipfoundry.org?expires=45>;q=1.0", contact);

        AbstractRing ring2 = new RingMock("333");
        ring2.setExpiration(25);
        ring2.setType(AbstractRing.Type.DELAYED);
        String contact2 = ring2.calculateContact("sipfoundry.org", q, true, true, null);
        assertEquals("<sip:333@sipfoundry.org;sipx-noroute=Voicemail?expires=25>;q=0.95",
                contact2);

        // with new q value - ring2 is delayed, q mustbe < 1.0
        ForkQueueValue q1 = new ForkQueueValue(3);
        contact2 = ring2.calculateContact("sipfoundry.org", q1, false, false, null);
        assertEquals("<sip:333@sipfoundry.org;sipx-userforward=false?expires=25>;q=0.95", contact2);
    }

    public void testCalculateContactWithPrefix() {
        AbstractRing ring = new RingMock("444");

        ForkQueueValue q = new ForkQueueValue(3);
        ring.setExpiration(45);
        ring.setType(AbstractRing.Type.IMMEDIATE);

        String contact = ring.calculateContact("sipfoundry.org", q, false, true, "~~vm~");
        assertEquals("<sip:~~vm~444@sipfoundry.org?expires=45>;q=1.0", contact);

        AbstractRing ring2 = new RingMock("333");
        ring2.setExpiration(25);
        ring2.setType(AbstractRing.Type.DELAYED);
        String contact2 = ring2.calculateContact("sipfoundry.org", q, true, true, "~~vm~");
        assertEquals("<sip:~~vm~333@sipfoundry.org;sipx-noroute=Voicemail?expires=25>;q=0.95",
                contact2);
    }

    private static final class RingMock extends AbstractRing {
        private final String m_userPart;

        public RingMock(String userPart) {
            m_userPart = userPart;
        }

        protected Object getUserPart() {
            return m_userPart;
        }
    }
}
