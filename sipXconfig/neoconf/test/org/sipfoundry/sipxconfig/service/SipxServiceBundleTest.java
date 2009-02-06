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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.service.SipxServiceBundle.TooFewBundles;
import org.sipfoundry.sipxconfig.service.SipxServiceBundle.TooManyBundles;

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

    public void testAutoEnableOn() {
        Location primary = new Location();
        primary.setPrimary(true);
        Location remote = new Location();

        SipxServiceBundle unrestricted = new SipxServiceBundle("unrestricted");
        SipxServiceBundle primaryOnly = new SipxServiceBundle("primaryOnly");
        primaryOnly.setOnlyPrimary(true);
        SipxServiceBundle remoteOnly = new SipxServiceBundle("remoteOnly");
        remoteOnly.setOnlyRemote(true);

        assertFalse(unrestricted.autoEnableOn(primary));
        assertFalse(unrestricted.autoEnableOn(remote));
        unrestricted.setAutoEnable(true);
        assertTrue(unrestricted.autoEnableOn(primary));
        assertFalse(unrestricted.autoEnableOn(remote));

        assertFalse(primaryOnly.autoEnableOn(primary));
        assertFalse(primaryOnly.autoEnableOn(remote));
        primaryOnly.setAutoEnable(true);
        assertTrue(primaryOnly.autoEnableOn(primary));
        assertFalse(primaryOnly.autoEnableOn(remote));

        assertFalse(remoteOnly.autoEnableOn(primary));
        assertFalse(remoteOnly.autoEnableOn(remote));
        remoteOnly.setAutoEnable(true);
        assertFalse(remoteOnly.autoEnableOn(primary));
        assertTrue(remoteOnly.autoEnableOn(remote));
    }

    public void testVerifyCount() {
        SipxServiceBundle bundle = new SipxServiceBundle("");
        try {
            bundle.verifyCount(0);
            bundle.verifyCount(1);
            bundle.verifyCount(4);
            bundle.verifyCount(5);
        } catch (UserException e) {
            fail("Cardinality should be OK here");
        }

        bundle.setMax(4);
        try {
            bundle.verifyCount(0);
            bundle.verifyCount(1);
            bundle.verifyCount(4);
        } catch (UserException e) {
            fail("Cardinality should be OK here");
        }
        try {
            bundle.verifyCount(5);
            fail("Should throw TooManyBundles");
        } catch (TooManyBundles e) {
            // expected too many bundles exception
        }

        bundle.setMin(1);
        try {
            bundle.verifyCount(0);
            fail("Should throw TooFewBundles");
        } catch (TooFewBundles e) {
            // expected too few bundles exception
        }
        try {
            bundle.verifyCount(1);
            bundle.verifyCount(4);
        } catch (UserException e) {
            fail("Cardinality should be OK here");
        }
        try {
            bundle.verifyCount(5);
            fail("Should throw TooManyBundles");
        } catch (TooManyBundles e) {
            // expected too many bundles exception
        }
    }
}
