/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.backup;

import java.io.File;
import java.io.StringWriter;
import java.io.Writer;
import java.util.Date;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.backup.Snapshot.SnapshotResult;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;

public class SnapshotTest extends TestCase {

    public void testGetCmdLine() throws Exception {
        Snapshot snapshot = new Snapshot();
        snapshot.setDestDirectory("tmp");
        Date startDate = new Date(1215779686450l);// Fri Jul 11 12:34:46 UTC 2008
        Date endDate = new Date(1215779810971l); // Fri Jul 11 12:36:50 UTC 2008

        snapshot.setFilterTime(false);
        Writer writer = new StringWriter();
        snapshot.composeCmdLine(writer, startDate, endDate, "my.test.org");
        assertEquals("--logs current tmp/sipx-snapshot-my.test.org.tar.gz", writer.toString());

        snapshot.setFilterTime(true);
        writer = new StringWriter();
        snapshot.composeCmdLine(writer, startDate, endDate, "my.test.org");
        assertEquals(
                "--logs current --log-start '2008-07-11 12:34:46' --log-stop '2008-07-11 12:36:50' tmp/sipx-snapshot-my.test.org.tar.gz",
                writer.toString());

        snapshot.setCredentials(true);
        snapshot.setWww(false);
        writer = new StringWriter();
        snapshot.composeCmdLine(writer, startDate, endDate, "my.test.org");
        assertEquals(
                "--logs current --log-start '2008-07-11 12:34:46' --log-stop '2008-07-11 12:36:50' --credentials --no-www tmp/sipx-snapshot-my.test.org.tar.gz",
                writer.toString());

        snapshot.setLogs(false);
        snapshot.setCdr(true);
        snapshot.setProfiles(true);
        writer = new StringWriter();
        snapshot.composeCmdLine(writer, startDate, endDate, "my.test.org");
        assertEquals("--logs none --credentials --cdr --profiles --no-www tmp/sipx-snapshot-my.test.org.tar.gz",
                writer.toString());
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
