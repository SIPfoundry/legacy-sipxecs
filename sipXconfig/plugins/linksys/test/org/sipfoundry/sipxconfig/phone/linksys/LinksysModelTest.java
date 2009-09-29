/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.phone.linksys;

import junit.framework.TestCase;

public class LinksysModelTest extends TestCase {

    public void testGetDefaultConfigName() {
        LinksysModel linksysModel = new LinksysModel();
        assertNull(linksysModel.getDefaultConfigName());
        linksysModel.setPsn("942");
        assertEquals("spa942.cfg", linksysModel.getDefaultConfigName());
    }
}
