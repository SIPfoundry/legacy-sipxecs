/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.device;

import junit.framework.TestCase;

public class DeviceDefaultsTest extends TestCase {

    public void testGetMusicOnHoldUri() {
        DeviceDefaults defaults = new DeviceDefaults();
        defaults.setMohUser("~~mh~");
        assertEquals("sip:~~mh~@example.org", defaults.getMusicOnHoldUri("example.org"));
    }
}
