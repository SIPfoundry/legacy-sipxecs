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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.dialplan.ForkQueueValue;

/**
 * RingTest
 */
public class RingTest extends TestCase {

    public void testCalculateContact() {
        ForkQueueValue q = new ForkQueueValue(3);
        Ring ring = new Ring();
        ring.setNumber("555");
        ring.setExpiration(45);
        ring.setType(Ring.Type.IMMEDIATE);
        
        String contact = ring.calculateContact("sipfoundry.org", q, false);
        assertEquals("<sip:555@sipfoundry.org?expires=45>;q=1.0",contact);
    }

    public void testCalculateAorContact() {
        ForkQueueValue q = new ForkQueueValue(3);
        Ring ring = new Ring();
        ring.setNumber("joe@example.com");
        ring.setExpiration(45);
        ring.setType(Ring.Type.IMMEDIATE);
        
        String contact = ring.calculateContact("shouldnt-be-used.com", q, false);
        assertEquals("<sip:joe@example.com?expires=45>;q=1.0",contact);
    }
}
