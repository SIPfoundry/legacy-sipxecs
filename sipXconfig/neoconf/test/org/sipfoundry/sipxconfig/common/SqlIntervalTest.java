/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import junit.framework.TestCase;

import org.postgresql.util.PGInterval;

public class SqlIntervalTest extends TestCase {

    public void testInterval2string() {
        PGInterval pgInterval = new PGInterval();
        pgInterval.setMinutes(10);
        SqlInterval sqlInterval = new SqlInterval(pgInterval);
        assertEquals(600000, sqlInterval.getMillisecs());
    }

    public void testCompareTo() {
        PGInterval pga = new PGInterval();
        pga.setDays(1);
        SqlInterval a = new SqlInterval(pga);

        PGInterval pgb = new PGInterval();
        pgb.setDays(2);
        SqlInterval b = new SqlInterval(pgb);

        assertEquals(-1, a.compareTo(b));
        assertEquals(1, b.compareTo(a));
        assertEquals(0, a.compareTo(a));
    }
}
