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

import static org.sipfoundry.sipxconfig.admin.dialplan.CallDigits.findFirstNonEscapedSpecialChar;
import junit.framework.TestCase;

public class CallDigitsTest extends TestCase {

    public void testFindFirstNonEscapedSpecialChar() {
        assertEquals(0, findFirstNonEscapedSpecialChar("abcd", "ad", 'c'));
        assertEquals(1, findFirstNonEscapedSpecialChar("abcd", "db", 'c'));
        assertEquals(-1, findFirstNonEscapedSpecialChar("abcd", "zx", 'c'));
        assertEquals(-1, findFirstNonEscapedSpecialChar("abcd", "de", 'c'));
        assertEquals(4, findFirstNonEscapedSpecialChar("abccd", "de", 'c'));
    }

    public void testGetEscapedName() {
        assertEquals("digits-escaped", CallDigits.FIXED_DIGITS.getEscapedName());
        assertEquals("vdigits-escaped", CallDigits.VARIABLE_DIGITS.getEscapedName());
    }
}
