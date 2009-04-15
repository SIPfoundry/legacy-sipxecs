/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.setting.type;

import java.util.regex.Pattern;

import junit.framework.TestCase;

public class SipUriSettingTest extends TestCase {
    public void testPattern() {
        SipUriSetting setting = new SipUriSetting();
        Pattern pattern = Pattern.compile(setting.getPattern());
        assertTrue(pattern.matcher("abcd").matches());
        assertTrue(pattern.matcher("abcd%12@example.com").matches());
        assertTrue(pattern.matcher("a,b!c.d@example.org").matches());
        assertFalse(pattern.matcher("user@sss@domain.com").matches());
    }

    public void testUserPartOnlyPattern() {
        SipUriSetting setting = new SipUriSetting();
        setting.setUserPartOnly(true);
        Pattern pattern = Pattern.compile(setting.getPattern());
        assertTrue(pattern.matcher("abcd").matches());
        assertTrue(pattern.matcher("abcd%12").matches());
        assertFalse(pattern.matcher("a,b!c.d@example.org").matches());
        assertFalse(pattern.matcher("user@sss@domain.com").matches());
    }
}
