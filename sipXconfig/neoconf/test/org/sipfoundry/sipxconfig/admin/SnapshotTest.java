/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.File;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;

public class SnapshotTest extends TestCase {

    public void testGetCmdLine() {
        Snapshot snapshot = new Snapshot();

        String cmdLine = StringUtils.join(snapshot.getCmdLine(""), ' ');
        assertEquals(File.separator + "sipx-snapshot --logs current sipx-configuration.tar.gz", cmdLine);

        snapshot.setCredentials(true);
        snapshot.setWww(false);
        cmdLine = StringUtils.join(snapshot.getCmdLine(""), ' ');
        assertEquals(File.separator + "sipx-snapshot --logs current --credentials --no-www sipx-configuration.tar.gz", cmdLine);

        snapshot.setLogs(false);
        cmdLine = StringUtils.join(snapshot.getCmdLine("xyz"), ' ');
        assertEquals("xyz" + File.separator + "sipx-snapshot --logs none --credentials --no-www sipx-configuration.tar.gz",
                cmdLine);
    }
}
