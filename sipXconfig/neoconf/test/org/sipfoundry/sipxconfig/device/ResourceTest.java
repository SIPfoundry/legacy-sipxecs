/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.device;

import junit.framework.TestCase;

public class ResourceTest extends TestCase {

    private Resource m_resource;

    protected void setUp() throws Exception {
        m_resource = new Resource("bongo", "/usr/local/c/bongo.txt");
    }

    public void testGetMimeType() {
        assertEquals("text/plain", m_resource.getMimeType());
    }

    public void testGetDirName() {
        assertEquals("/usr/local/c", m_resource.getDirName());
    }

    public void testGetFileName() {
        assertEquals("bongo.txt", m_resource.getFileName());
    }

}
