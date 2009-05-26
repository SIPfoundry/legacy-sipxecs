/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.File;
import java.util.Date;

import org.sipfoundry.sipxconfig.admin.Snapshot.SnapshotResult;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.common.UserException;

import junit.framework.TestCase;

import static org.apache.commons.lang.StringUtils.join;

public class SnapshotTest extends TestCase {

    public void testGetCmdLine() {
        Snapshot snapshot = new Snapshot();
        Date startDate = new Date(1215779686450l);// Fri Jul 11 12:34:46 UTC 2008
        Date endDate = new Date(1215779810971l); // Fri Jul 11 12:36:50 UTC 2008

        snapshot.setFilterTime(false);
        String cmdLine = join(snapshot.getCmdLine(startDate, endDate), '|');
        assertEquals("--logs|current", cmdLine);

        snapshot.setFilterTime(true);
        cmdLine = join(snapshot.getCmdLine(startDate, endDate), '|');
        assertEquals("--logs|current|--log-start|2008-07-11 12:34:46|"
                + "--log-stop|2008-07-11 12:36:50", cmdLine);

        snapshot.setCredentials(true);
        snapshot.setWww(false);
        cmdLine = join(snapshot.getCmdLine(startDate, endDate), '|');
        assertEquals("--logs|current|--log-start|2008-07-11 12:34:46|"
                + "--log-stop|2008-07-11 12:36:50|--credentials|--no-www", cmdLine);

        snapshot.setLogs(false);
        snapshot.setCdr(true);
        snapshot.setProfiles(true);
        cmdLine = join(snapshot.getCmdLine(startDate, endDate), '|');
        assertEquals(
                "--logs|none|--credentials|--cdr|--profiles|--no-www",
                cmdLine);
    }

    public void testSnapshotResultFailure() {
        SnapshotResult result = new Snapshot.SnapshotResult(new UserException());
        assertFalse(result.isSuccess());
        assertNotNull(result.getUserException());
    }

    public void testSnapshotResultSuccess() {
        File file = new File("/snapshot/dir/snapshot.tgz");
        Location location = new Location();
        location.setFqdn("host.example.com");

        SnapshotResult result = new Snapshot.SnapshotResult(location, file);
        assertTrue(result.isSuccess());
        assertNull(result.getUserException());

        assertEquals("/snapshot/dir", result.getDir());
        assertEquals("host.example.com", result.getFqdn());
    }
}
