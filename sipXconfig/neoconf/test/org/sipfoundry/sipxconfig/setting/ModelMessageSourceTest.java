/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.io.File;

import junit.framework.TestCase;

public class ModelMessageSourceTest extends TestCase {
    public void testGetBundle() throws Exception {
        File file = new File("manufacturer/phone.xml");
        ModelMessageSource source = new ModelMessageSource(file);
        // ideally assertEquals(manufacturer.phone, source.basenames[0])
        assertTrue(source.toString().contains("basenames=[manufacturer.phone]"));
    }
}
