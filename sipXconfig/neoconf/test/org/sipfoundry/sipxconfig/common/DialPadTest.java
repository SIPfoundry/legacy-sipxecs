/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.common;

import junit.framework.TestCase;

public class DialPadTest extends TestCase {
    public void testRange() throws Exception {
        DialPad[] range = DialPad.getRange(DialPad.NUM_3, DialPad.NUM_7);
        assertEquals(5, range.length);
        assertSame(DialPad.NUM_3, range[0]);
        assertSame(DialPad.NUM_4, range[1]);
        assertSame(DialPad.NUM_5, range[2]);
        assertSame(DialPad.NUM_6, range[3]);
        assertSame(DialPad.NUM_7, range[4]);
    }

    public void testRangeEmpty() throws Exception {
        DialPad[] range = DialPad.getRange(DialPad.NUM_7, DialPad.NUM_5);
        assertEquals(0, range.length);
    }

    public void testRangeSingle() throws Exception {
        DialPad[] range = DialPad.getRange(DialPad.NUM_8, DialPad.NUM_8);
        assertEquals(1, range.length);
        assertSame(DialPad.NUM_8, range[0]);
    }
}
