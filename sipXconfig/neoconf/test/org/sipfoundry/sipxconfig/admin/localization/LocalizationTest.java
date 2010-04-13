/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.localization;

import junit.framework.TestCase;

public class LocalizationTest extends TestCase {

    protected void setUp() throws Exception {
        super.setUp();
    }

    public void testGetLanguage() {
        Localization localization = new Localization();
        assertNull(localization.getLanguage());
        localization.setLanguage("pl");
        assertEquals("pl", localization.getLanguage());

    }

    public void testGetRegionId() {
        Localization localization = new Localization();
        assertNull(localization.getRegionId());
        localization.setRegion("region_na");
        assertEquals("na", localization.getRegionId());
    }
}
