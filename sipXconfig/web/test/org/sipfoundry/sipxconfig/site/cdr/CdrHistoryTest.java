/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr;

import java.util.Calendar;
import java.util.Date;

import junit.framework.TestCase;

public class CdrHistoryTest extends TestCase {

    public void testGetDefaultEndTime() {
        Calendar now = Calendar.getInstance();
        Date defaultEndTime = CdrHistory.getDefaultEndTime();
        Calendar endTime = Calendar.getInstance();
        endTime.setTime(defaultEndTime);
        assertTrue(endTime.after(now));
        assertEquals(0, endTime.get(Calendar.HOUR_OF_DAY));
        assertEquals(0, endTime.get(Calendar.MINUTE));
        assertEquals(0, endTime.get(Calendar.SECOND));
    }
}
