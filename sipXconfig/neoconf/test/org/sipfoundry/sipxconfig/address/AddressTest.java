/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.address;

import static org.junit.Assert.assertEquals;

import org.junit.Test;


public class AddressTest {
    
    @Test
    public void noProtocol() {
        assertEquals("sip:1:2", new Address(AddressType.sipTcp("test"), "1", 2).toString());
        assertEquals("1:2", new Address(AddressType.sipTcp("test"), "1", 2).stripProtocol());
        assertEquals("1", new Address(new AddressType("test"), "1").stripProtocol());
        assertEquals("one.two:3", new Address(new AddressType("test"), "one.two", 3).stripProtocol());
        assertEquals("test.example.org:2", new Address(AddressType.sipTcp("test"), "test.example.org", 2).stripProtocol());
        assertEquals("test.example.org:2/x/y", new Address(new AddressType("test", "http://%s:%d/x/y"), "test.example.org", 2).stripProtocol());
    }

}
