/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.common;

import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;

import org.sipfoundry.sipxconfig.site.common.DateTimeEditor;

import junit.framework.TestCase;

public class DateTimeEditorTest extends TestCase {

    public void testToDateTime() throws Exception {
        Date date = new Date();
        SimpleDateFormat format = new SimpleDateFormat("HH:mm");
        Date time = format.parse("14:35");

        Calendar expected = Calendar.getInstance();
        expected.setTime(date);

        Calendar actual = Calendar.getInstance();
        actual.setTime(DateTimeEditor.toDateTime(date, time, Locale.US));

        assertEquals(expected.get(Calendar.YEAR), actual.get(Calendar.YEAR));
        assertEquals(expected.get(Calendar.MONTH), actual.get(Calendar.MONTH));
        assertEquals(expected.get(Calendar.DAY_OF_MONTH), actual.get(Calendar.DAY_OF_MONTH));

        assertEquals(14, actual.get(Calendar.HOUR_OF_DAY));
        assertEquals(35, actual.get(Calendar.MINUTE));
    }

}
