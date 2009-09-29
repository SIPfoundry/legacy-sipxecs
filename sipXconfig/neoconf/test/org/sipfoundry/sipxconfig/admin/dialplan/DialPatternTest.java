/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import junit.framework.TestCase;

import org.apache.commons.lang.StringUtils;

/**
 * DialPatternTest
 */
public class DialPatternTest extends TestCase {
    public void testCalculatePattern() {
        DialPattern pattern = new DialPattern();
        pattern.setDigits(3);
        pattern.setPrefix("19");
        assertEquals("19xxx", pattern.calculatePattern());

        pattern.setDigits(0);
        pattern.setPrefix("19");
        assertEquals("19", pattern.calculatePattern());

        pattern.setDigits(5);
        pattern.setPrefix("");
        assertEquals("xxxxx", pattern.calculatePattern());

        pattern.setDigits(5);
        pattern.setPrefix(null);
        assertEquals("xxxxx", pattern.calculatePattern());

        pattern.setDigits(-1);
        pattern.setPrefix("134");
        assertEquals("134.", pattern.calculatePattern());
    }

    public void testGetPatternsFromList() {
        String list = "sdf, 123, 444   , 34";
        String[] patterns = DialPattern.getPatternsFromList(list, StringUtils.EMPTY);
        assertEquals("sdf", patterns[0]);
        assertEquals("123", patterns[1]);
        assertEquals("444", patterns[2]);
        assertEquals("34", patterns[3]);

        patterns = DialPattern.getPatternsFromList(list, "bongo");
        assertEquals("sdfbongo", patterns[0]);
        assertEquals("123bongo", patterns[1]);
        assertEquals("444bongo", patterns[2]);
        assertEquals("34bongo", patterns[3]);

        patterns = DialPattern.getPatternsFromList(StringUtils.EMPTY, "bongo");
        assertEquals(0, patterns.length);
    }
}
