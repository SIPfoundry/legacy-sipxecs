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

import java.util.Locale;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.TimeOfDay.TimeOfDayFormat;

public class TimeOfDayTest extends TestCase {
    private TimeOfDayFormat m_format;

    protected void setUp() throws Exception {
        m_format = new TimeOfDay.TimeOfDayFormat(Locale.US);
    }

    public void testFormatParseObject() throws Exception {
        TimeOfDay tod = (TimeOfDay) m_format.parseObject("3:13 pm");
        assertEquals(15, tod.getHrs());
        assertEquals(13, tod.getMin());

        tod = (TimeOfDay) m_format.parseObject("14:12");
        assertEquals(14, tod.getHrs());
        assertEquals(12, tod.getMin());

        tod = (TimeOfDay) m_format.parseObject("3:13 am");
        assertEquals(3, tod.getHrs());
        assertEquals(13, tod.getMin());
    }

    public void testFormatFormat() {
        TimeOfDay tod = new TimeOfDay(2, 14);
        assertEquals("2:14 AM", m_format.format(tod));

        tod = new TimeOfDay(13, 14);
        assertEquals("1:14 PM", m_format.format(tod));
    }
}
