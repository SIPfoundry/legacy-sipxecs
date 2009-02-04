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

import java.util.Date;

import junit.framework.TestCase;

import static org.apache.commons.lang.StringUtils.join;

public class SnapshotTest extends TestCase {

    public void testGetCmdLine() {
        Snapshot snapshot = new Snapshot();
        Date startDate = new Date(1215779686450l);// Fri Jul 11 12:34:46 UTC 2008
        Date endDate = new Date(1215779810971l); // Fri Jul 11 12:36:50 UTC 2008

        String cmdLine = join(snapshot.getCmdLine("", startDate, endDate), '|');
        assertEquals("/sipx-snapshot|--logs|current|sipx-configuration.tar.gz", cmdLine);

        snapshot.setFilterTime(true);
        cmdLine = join(snapshot.getCmdLine("", startDate, endDate), '|');
        assertEquals("/sipx-snapshot|--logs|current|--log-start|2008-07-11 12:34:46|"
                + "--log-stop|2008-07-11 12:36:50|sipx-configuration.tar.gz", cmdLine);

        snapshot.setCredentials(true);
        snapshot.setWww(false);
        cmdLine = join(snapshot.getCmdLine("", startDate, endDate), '|');
        assertEquals("/sipx-snapshot|--logs|current|--log-start|2008-07-11 12:34:46|"
                + "--log-stop|2008-07-11 12:36:50|--credentials|--no-www|sipx-configuration.tar.gz", cmdLine);

        snapshot.setLogs(false);
        snapshot.setCdr(true);
        snapshot.setProfiles(true);
        cmdLine = join(snapshot.getCmdLine("xyz", startDate, endDate), '|');
        assertEquals(
                "xyz/sipx-snapshot|--logs|none|--credentials|--cdr|--profiles|--no-www|sipx-configuration.tar.gz",
                cmdLine);
    }
}
