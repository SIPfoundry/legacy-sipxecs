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

/**
 * CallPatternTest
 */
public class CallPatternTest extends TestCase {

    public void testCalculatePattern() {
        CallPattern pattern = new CallPattern();
        pattern.setPrefix("91");
        pattern.setDigits(CallDigits.NO_DIGITS);
        assertEquals("91", pattern.calculatePattern());

        pattern.setPrefix("12");
        pattern.setDigits(CallDigits.VARIABLE_DIGITS);
        assertEquals("12{vdigits}", pattern.calculatePattern());

        pattern.setPrefix("");
        pattern.setDigits(CallDigits.FIXED_DIGITS);
        assertEquals("{digits}", pattern.calculatePattern());

        pattern.setPrefix(null);
        pattern.setDigits(CallDigits.NO_DIGITS);
        assertEquals("", pattern.calculatePattern());

        CallPattern pattern2 = new CallPattern("15", CallDigits.VARIABLE_DIGITS);
        assertEquals("15{vdigits}", pattern2.calculatePattern());
    }

    public void testTransformVariable() throws Exception {
        CallPattern pattern = new CallPattern("15", CallDigits.VARIABLE_DIGITS);
        DialPattern dp = new DialPattern("33", 4);
        DialPattern tdp = pattern.transform(dp);
        assertEquals("15", tdp.getPrefix());
        assertEquals(4, tdp.getDigits());
    }

    public void testTransformFixed() throws Exception {
        CallPattern pattern = new CallPattern("15", CallDigits.FIXED_DIGITS);
        DialPattern dp = new DialPattern("33", 4);
        DialPattern tdp = pattern.transform(dp);
        assertEquals("1533", tdp.getPrefix());
        assertEquals(4, tdp.getDigits());
    }

    public void testTransformNoDigits() throws Exception {
        CallPattern pattern = new CallPattern("15", CallDigits.NO_DIGITS);
        DialPattern dp = new DialPattern("33", 4);
        DialPattern tdp = pattern.transform(dp);
        assertEquals("15", tdp.getPrefix());
        assertEquals(0, tdp.getDigits());
    }

    public void testTransformRangeVariable() throws Exception {
        CallPattern pattern = new CallPattern("15", CallDigits.VARIABLE_DIGITS);
        DialPattern dp = new DialPattern("33[2-5]", 4);
        DialPattern tdp = pattern.transform(dp);
        assertEquals("15[2-5]", tdp.getPrefix());
        assertEquals(4, tdp.getDigits());
    }

    public void testTransformRangeInMiddleVariable() throws Exception {
        CallPattern pattern = new CallPattern("15", CallDigits.VARIABLE_DIGITS);
        DialPattern dp = new DialPattern("33[2-5]22", 4);
        DialPattern tdp = pattern.transform(dp);
        assertEquals("15[2-5]22", tdp.getPrefix());
        assertEquals(4, tdp.getDigits());

        // escaped special character
        dp = new DialPattern("33\\[2-5]22", 4);
        tdp = pattern.transform(dp);
        assertEquals("15", tdp.getPrefix());
        assertEquals(4, tdp.getDigits());

        // escaped escape character - . here is a special character
        dp = new DialPattern("33\\\\.22", 4);
        tdp = pattern.transform(dp);
        assertEquals("15.22", tdp.getPrefix());
        assertEquals(4, tdp.getDigits());
    }

    public void testTransformRangeFixed() throws Exception {
        CallPattern pattern = new CallPattern("15", CallDigits.FIXED_DIGITS);
        DialPattern dp = new DialPattern("33[2-5]", 4);
        DialPattern tdp = pattern.transform(dp);
        assertEquals("1533[2-5]", tdp.getPrefix());
        assertEquals(4, tdp.getDigits());
    }

    public void testTransformRangeNoDigits() throws Exception {
        CallPattern pattern = new CallPattern("15", CallDigits.NO_DIGITS);
        DialPattern dp = new DialPattern("33[2-5]", 4);
        DialPattern tdp = pattern.transform(dp);
        assertEquals("15", tdp.getPrefix());
        assertEquals(0, tdp.getDigits());
    }
}
