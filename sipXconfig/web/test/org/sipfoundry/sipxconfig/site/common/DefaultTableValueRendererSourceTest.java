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

import java.util.Calendar;
import java.util.Locale;
import java.util.TimeZone;

import junit.framework.TestCase;

import org.apache.tapestry.valid.RenderString;
import org.postgresql.util.PGInterval;
import org.sipfoundry.sipxconfig.common.SqlInterval;

public class DefaultTableValueRendererSourceTest extends TestCase {

    public void testGetDefaultRender() {
        DefaultTableValueRendererSource renderer = new DefaultTableValueRendererSource();
        assertNull(renderer.getRender("honey creeper", null));
    }

    public void testSqlIntervalRenderer() {
        DefaultTableValueRendererSource renderer = new DefaultTableValueRendererSource();
        PGInterval pg = new PGInterval();
        pg.setMinutes(1);
        RenderString render = (RenderString) renderer.getRender(new SqlInterval(pg),
                Locale.ENGLISH);
        // assumes Millis is not localized
        assertEquals("1 minute, 0 seconds", render.getString());
    }

    public void testDateRenderer() {
        Calendar calendar = Calendar.getInstance();
        calendar.set(Calendar.YEAR, 1980);
        calendar.set(Calendar.MONTH, 6);
        calendar.set(Calendar.DAY_OF_MONTH, 15);
        calendar.set(Calendar.HOUR_OF_DAY, 17);
        calendar.setTimeZone(TimeZone.getTimeZone("UTC"));

        DefaultTableValueRendererSource renderer = new DefaultTableValueRendererSource();
        RenderString render = (RenderString) renderer.getRender(calendar.getTime(),
                Locale.ENGLISH);

        // please note that rendered has to return the time in current time zone formatted
        // according to Locale.ENGLISH - in EST it would be 7/15/80 1:00 PM
        assertTrue(render.getString().matches("7/1[456]/80 \\d{1,2}:\\d{2} [AP]M"));
    }
}
