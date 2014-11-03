/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.parkorbit;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ParkOrbitTest extends TestCase {
    public void testCreateAlias() {
        ParkOrbit orbit = new ParkOrbit();
        orbit.setExtension("1002");
        AliasMapping alias = orbit.generateAlias("from.com", "to.com");
        assertEquals("1002@from.com", alias.getIdentity());
        assertEquals("1002@to.com", alias.getContact());
    }

    public void testDefaultSettings() throws Exception {
        ParkOrbit orbit = new ParkOrbit();
        orbit.setModelFilesContext(TestHelper.getModelFilesContext());

        assertFalse(orbit.isParkTimeoutEnabled());
        assertEquals(86400, orbit.getParkTimeout());

        assertFalse(orbit.isMultipleCalls());

        assertFalse(orbit.isTransferAllowed());
        assertEquals("0", orbit.getTransferKey());
    }
}
