/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc;

import junit.framework.TestCase;

public class SbcRoutesTest extends TestCase {

    public void testIsEmpty() {
        SbcRoutes routes = new SbcRoutes();
        assertTrue(routes.isEmpty());

        routes.addDomain();
        assertFalse(routes.isEmpty());

        routes.addSubnet();
        assertFalse(routes.isEmpty());


        routes.removeDomain(0);
        assertFalse(routes.isEmpty());

        routes.removeSubnet(0);
        assertTrue(routes.isEmpty());
    }
}
