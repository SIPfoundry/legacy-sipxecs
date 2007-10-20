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

    public void testGetLanguageId() {
        Localization localization = new Localization();
        assertNull(localization.getLanguageId());
        localization.setLanguage("stdprompts_pl");
        assertEquals("pl", localization.getLanguageId());

    }

    public void testGetRegionId() {
        Localization localization = new Localization();
        assertNull(localization.getRegionId());
        localization.setRegion("region_na");
        assertEquals("na", localization.getRegionId());
    }
}
