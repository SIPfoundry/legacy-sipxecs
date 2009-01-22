/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.admin.commserver.Location;

import junit.framework.TestCase;

public class SipxServiceBundleTest extends TestCase {

    public void testCanRunOn() {
        Location primary = new Location();
        primary.setPrimary(true);
        Location remote = new Location();

        SipxServiceBundle unrestricted = new SipxServiceBundle("unrestricted");
        SipxServiceBundle primaryOnly = new SipxServiceBundle("primaryOnly");
        primaryOnly.setOnlyPrimary(true);
        SipxServiceBundle remoteOnly = new SipxServiceBundle("remoteOnly");
        remoteOnly.setOnlyRemote(true);

        assertTrue(unrestricted.canRunOn(primary));
        assertTrue(unrestricted.canRunOn(remote));

        assertTrue(primaryOnly.canRunOn(primary));
        assertFalse(primaryOnly.canRunOn(remote));

        assertFalse(remoteOnly.canRunOn(primary));
        assertTrue(remoteOnly.canRunOn(remote));
    }
}
