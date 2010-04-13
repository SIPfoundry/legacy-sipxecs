/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Arrays;
import java.util.HashSet;

import junit.framework.TestCase;

public class FeatureFilterTest extends TestCase {

    public void testEvaluate() {
        DeviceDescriptor dd = new DeviceDescriptor() {
        };

        FeatureFilter filter = new FeatureFilter("intercom");
        assertFalse(filter.evaluate(dd));

        dd.setSupportedFeatures(new HashSet<String>(Arrays.asList("a", "b")));
        assertFalse(filter.evaluate(dd));

        dd.setSupportedFeatures(new HashSet<String>(Arrays.asList("a", "intercom", "b")));
        assertTrue(filter.evaluate(dd));
    }

    public void testSupportedFeaturesByVersion() {
        DeviceVersion version = new DeviceVersion("vendor", "1.0");

        FeatureFilter filter = new FeatureFilter("intercom");
        assertFalse(filter.evaluate(version));

        version.setSupportedFeatures(new HashSet<String>(Arrays.asList("a", "b")));
        assertFalse(filter.evaluate(version));

        version.setSupportedFeatures(new HashSet<String>(Arrays.asList("a", "intercom", "b")));
        assertTrue(filter.evaluate(version));
    }
}
