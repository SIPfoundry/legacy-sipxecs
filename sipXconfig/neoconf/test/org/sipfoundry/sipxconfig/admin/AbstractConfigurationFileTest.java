/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.IOException;
import java.io.Writer;
import org.sipfoundry.sipxconfig.admin.commserver.Location;

import junit.framework.TestCase;

public class AbstractConfigurationFileTest extends TestCase {

    public void testGetPath() {
        AbstractConfigurationFile file = new AbstractConfigurationFile() {
            public void write(Writer writer, Location location) throws IOException {
            }
        };

        try {
            file.getPath();
            fail("Cannot get path if name is not set.");
        } catch (IllegalStateException e) {
            // ok
        }

        file.setName("xxx");
        assertEquals("xxx", file.getPath());

        file.setDirectory("/etc/sipx");
        assertEquals("/etc/sipx/xxx", file.getPath());
    }
}
