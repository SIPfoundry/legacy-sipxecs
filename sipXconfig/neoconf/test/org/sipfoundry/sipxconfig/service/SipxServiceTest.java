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

import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;

import junit.framework.TestCase;

public class SipxServiceTest extends TestCase {

    public void testIsAutoEnabled() {
        SipxServiceBundle autoEnabledBundle = new SipxServiceBundle("autoEnabled");
        autoEnabledBundle.setAutoEnable(true);

        SipxServiceBundle standardBundle = new SipxServiceBundle("standard");

        SipxService service = new SipxService() {
        };

        assertFalse(service.isAutoEnabled());

        service.setBundles(new HashSet(Arrays.asList(autoEnabledBundle, standardBundle)));
        assertTrue(service.isAutoEnabled());

        service.setBundles(Collections.singleton(autoEnabledBundle));
        assertTrue(service.isAutoEnabled());

        service.setBundles(Collections.singleton(standardBundle));
        assertFalse(service.isAutoEnabled());
    }
}
