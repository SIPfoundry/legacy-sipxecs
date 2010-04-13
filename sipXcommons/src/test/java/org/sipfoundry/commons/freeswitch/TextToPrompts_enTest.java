/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.freeswitch;

import org.sipfoundry.commons.freeswitch.TextToPrompts;
import org.sipfoundry.commons.freeswitch.TextToPrompts_en;
import org.sipfoundry.commons.freeswitch.TextToPrompts.Types;

import junit.framework.TestCase;

public class TextToPrompts_enTest extends TestCase {

    public void testCardinal() {
        TextToPrompts c = new TextToPrompts_en();
        c.setType(Types.cardinal);
        assertEquals("0.wav", c.render("0"));
        assertEquals("05.wav", c.render("5"));
        assertEquals("99.wav", c.render("99"));
        assertEquals("100.wav", c.render("100"));
        assertEquals("100.wav:01.wav", c.render("101"));
        assertEquals("200.wav", c.render("200"));
        assertEquals("200.wav:22.wav", c.render("222"));
        assertEquals("900.wav:99.wav", c.render("999"));
        assertEquals("1000.wav", c.render("1000"));

        assertEquals("1000.wav:01.wav", c.render("1001"));
        assertEquals("1000.wav:100.wav", c.render("1100"));
        assertEquals("1000.wav:100.wav:10.wav", c.render("1110"));
        assertEquals("1000.wav:900.wav:99.wav", c.render("1999"));
        assertEquals("2000.wav", c.render("2000"));
        assertEquals("2000.wav:01.wav", c.render("2001"));
        assertEquals("2019.wav", c.render("2019"));

        assertEquals("9000.wav:900.wav:99.wav", c.render("9999"));
        assertEquals("10.wav:thousand.wav", c.render("10000"));
        assertEquals("10.wav:thousand.wav:01.wav", c.render("10001"));
        assertEquals("10.wav:thousand.wav:100.wav:01.wav", c.render("10101"));

        assertEquals("900.wav:99.wav:thousand.wav:900.wav:99.wav", c.render("999999"));

        // assertEquals("minus.wav:42.wav", c.render("-42")) ; // cannot handle negatives!
        assertEquals("42.wav", c.render("-42"));

        assertEquals("", c.render("Woof!"));

        c.setPrefix("/fuzz/");
        assertEquals("/fuzz/0.wav", c.render("0"));
        assertEquals("/fuzz/10.wav:/fuzz/thousand.wav", c.render("10000"));

    }

    public void testOrdinal() {
        TextToPrompts c = new TextToPrompts_en();
        c.setType(Types.ordinal);
        assertEquals("", c.render("0"));
        assertEquals("", c.render("-42"));
        assertEquals("", c.render("Woof!"));
        assertEquals("1st.wav", c.render("1"));
        assertEquals("2nd.wav", c.render("2"));
        assertEquals("3rd.wav", c.render("3"));
        assertEquals("31st.wav", c.render("31"));
        assertEquals("30.wav:2nd.wav", c.render("32"));
        assertEquals("40.wav:2nd.wav", c.render("42"));
        assertEquals("40.wav:9th.wav", c.render("49"));
        assertEquals("50th.wav", c.render("50"));

        c.setPrefix("/fuzz/");
        assertEquals("", c.render("0"));
        assertEquals("/fuzz/40.wav:/fuzz/9th.wav", c.render("49"));
    }

    public void testDigits() {
        TextToPrompts c = new TextToPrompts_en();
        c.setType(Types.digits);
        assertEquals("", c.render("Woof!"));
        assertEquals("0.wav", c.render("0"));
        assertEquals("0.wav:0.wav", c.render("00"));
        assertEquals("0.wav:01.wav:02.wav", c.render("0A1b2-"));

        c.setPrefix("/fuzz/");
        assertEquals("/fuzz/0.wav:/fuzz/01.wav:/fuzz/02.wav", c.render("0A1b2-"));
    }

    public void testLetters() {
        TextToPrompts c = new TextToPrompts_en();
        c.setType(Types.letters);
        assertEquals("0.wav", c.render("0"));
        assertEquals("0.wav:0.wav", c.render("00"));
        assertEquals("0.wav:a.wav:01.wav:b.wav:02.wav", c.render("0A1B2"));
        assertEquals("w.wav:o.wav:o.wav:f.wav:at.wav:s.wav:dot.wav:o.wav:r.wav:g.wav", c
                .render("woof@s.org"));
        assertEquals("", c.render("_=$%"));

        c.setPrefix("D");
        assertEquals("D/0.wav", c.render("0"));
    }

    public void testDates() {
        TextToPrompts c = new TextToPrompts_en();
        c.setType(Types.date);
        c.setFormat("MDY");
        assertEquals("january.wav:26th.wav:19.wav:63.wav", c.render("1963-01-26 13:42:11"));
        assertEquals("december.wav:31st.wav:2008.wav", c.render("2008-12-31 00:00:00"));
        assertEquals("june.wav:10th.wav:2000.wav:21.wav", c.render("2021-06-10 00:00:00"));

        c.setFormat("YMD");
        assertEquals("19.wav:63.wav:january.wav:26th.wav", c.render("1963-01-26 13:42:11"));
        assertEquals("2008.wav:december.wav:31st.wav", c.render("2008-12-31 00:00:00"));
        assertEquals("2000.wav:21.wav:june.wav:10th.wav", c.render("2021-06-10 00:00:00"));

        c.setFormat("WMDYT");
        assertEquals("saturday.wav:january.wav:26th.wav:19.wav:63.wav:01.wav:42.wav:pm.wav",
                c.render("1963-01-26 13:42:11"));
        c.setFormat("WMD");
        assertEquals("friday.wav:july.wav:11th.wav", c.render("2008-07-11 16:27:05"));

        c.setFormat("m");
        assertEquals("0.wav:hours.wav", c.render("2008-07-11 00:00:00"));
        assertEquals("o.wav:o.wav:o_one.wav", c.render("2008-07-11 00:01:00"));
        assertEquals("o.wav:o.wav:59.wav", c.render("2008-07-11 00:59:00"));
        assertEquals("o.wav:100.wav:hours.wav", c.render("2008-07-11 01:00:00"));
        assertEquals("o.wav:100.wav:o_one.wav", c.render("2008-07-11 01:01:00"));
        assertEquals("o.wav:100.wav:10.wav", c.render("2008-07-11 01:10:00"));
        assertEquals("10.wav:hours.wav", c.render("2008-07-11 10:00:00"));
        assertEquals("12.wav:hours.wav", c.render("2008-07-11 12:00:00"));
        assertEquals("13.wav:23.wav", c.render("2008-07-11 13:23:00"));
        assertEquals("23.wav:59.wav", c.render("2008-07-11 23:59:59"));

        c.setFormat("T");
        assertEquals("12.wav:midnight.wav", c.render("2008-07-11 00:00:00"));
        assertEquals("12.wav:o_one.wav:am.wav", c.render("2008-07-11 00:01:00"));
        assertEquals("12.wav:59.wav:am.wav", c.render("2008-07-11 00:59:00"));
        assertEquals("01.wav:o_clock.wav:am.wav", c.render("2008-07-11 01:00:00"));
        assertEquals("01.wav:o_one.wav:am.wav", c.render("2008-07-11 01:01:00"));
        assertEquals("01.wav:10.wav:am.wav", c.render("2008-07-11 01:10:00"));
        assertEquals("10.wav:o_clock.wav:am.wav", c.render("2008-07-11 10:00:00"));
        assertEquals("12.wav:noon.wav", c.render("2008-07-11 12:00:00"));
        assertEquals("01.wav:23.wav:pm.wav", c.render("2008-07-11 13:23:00"));
        assertEquals("11.wav:59.wav:pm.wav", c.render("2008-07-11 23:59:59"));

        c.setFormat("MDY");
        c.setPrefix("D");
        assertEquals("D/january.wav:D/26th.wav:D/19.wav:D/63.wav", c
                .render("1963-01-26 13:42:11"));
    }

    public void testPause() {
        TextToPrompts c = new TextToPrompts_en();
        c.setType(Types.pause);
        c.setFormat("1234");
        assertEquals("silence_stream://0", c.render("0"));
        assertEquals("silence_stream://1234", c.render("-"));
    }
}
