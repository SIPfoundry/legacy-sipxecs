/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.address;

import static org.junit.Assert.assertEquals;

import org.junit.Test;


public class AddressTest {
    
    @Test
    public void noProtocol() {
        assertEquals("sip:1:2", new Address(AddressType.sip("test"), "1", 2).toString());
        assertEquals("1:2", new Address(AddressType.sip("test"), "1", 2).stripProtocol());
        assertEquals("1", new Address(new AddressType("test"), "1").stripProtocol());
        assertEquals("one.two:3", new Address(new AddressType("test"), "one.two", 3).stripProtocol());
        assertEquals("test.example.org:2", new Address(AddressType.sip("test"), "test.example.org", 2).stripProtocol());
        assertEquals("test.example.org:2/x/y", new Address(new AddressType("test", "http://%s:%d/x/y"), "test.example.org", 2).stripProtocol());
    }

}
